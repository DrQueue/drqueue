/*
 *  typemap.c
 *
 *  $Id: typemap.c,v 1.2 2000/06/18 17:15:27 fukusima Exp $
 *
 *  Copyright (C) 1999,2000  Masaki Fukushima
 *
 *  Class/Type and Module Mapping.
 */

#include "ruby-python.h"
#include "env.h"

/* store PyObject* as Integer in Hash */
#define PYOBJ2NUM(po) (INT2FIX((int)po))
#define NUM2PYOBJ(n) ((PyObject *)FIX2INT(n))

#define PYOBJ_AS_STR(po, pstr) (\
    pstr = PyObject_Str(po), PyString_AS_STRING(pstr))

PyObject *pmBuiltin, *pmMain, *pmSys;
PyObject *pdModules, *pdMainDict, *pdSysDict;
static VALUE hModule_py2rb, hModule_rb2py; /* Hash for module mapping */
static VALUE hClass_py2rb, hClass_rb2py; /* Hash for class/type mapping */
static int id_caller, id_set_backtrace;

static void pytm_refresh();

PyObject *
pytm_pymodule_noex(module)
    VALUE module;
{
    VALUE v = rb_hash_aref(hModule_rb2py, module);
    if (v == Qnil)
	return NULL;
    return NUM2PYOBJ(v);
}

PyObject *
pytm_pymodule(module)
    VALUE module;
{
    VALUE v;

    v = rb_hash_aref(hModule_rb2py, module);
    if (v == Qnil)
	rb_bug("pytm_pymodule: no python module mapped to a ruby module %s",
	       RSTRING(rb_inspect(module))->ptr);
    return NUM2PYOBJ(v);
}

static VALUE
pytm_rbmodule_noex(pmodule)
    PyObject *pmodule;
{
    return rb_hash_aref(hModule_py2rb, PYOBJ2NUM(pmodule));
}

VALUE
pytm_rbmodule(pmodule)
    PyObject *pmodule;
{
    VALUE module;

    module = pytm_rbmodule_noex(pmodule);
    if (module == Qnil) {
	pytm_refresh();
	module = pytm_rbmodule_noex(pmodule);
	if (module == Qnil) {
	    PyObject *pstr;
	    rb_bug("pytm_rbmodule: no ruby module mapped to a python module"
		   " (%s)", PYOBJ_AS_STR(pmodule, pstr));
	    Py_DECREF(pstr);
	}
    }
    return module;
}

void
pytm_add_module(rmod, pmod)
    VALUE rmod;
    PyObject *pmod;
{
    VALUE v;

//    if (pytm_rbmodule_noex(pmod) != Qnil)
//	rb_bug("pytm_add_module: try to add already exist module");

    Py_INCREF(pmod);
    v = PYOBJ2NUM(pmod);
    rb_hash_aset(hModule_py2rb, v, rmod);
    rb_hash_aset(hModule_rb2py, rmod, v);
}

PyObject *
pytm_pyclass_noex(class)
    VALUE class;
{
    VALUE v;

    v = rb_hash_aref(hClass_rb2py, class);
    if (v == Qnil)
	return NULL;
    return NUM2PYOBJ(v);
}

PyObject *
pytm_pyclass(class)
    VALUE class;
{
    VALUE v;

    v = rb_hash_aref(hClass_rb2py, class);
    if (v == Qnil)
	rb_bug("pytm_pyclass: no python type/class mapped to a ruby class");
    return NUM2PYOBJ(v);
}

static void
define_methods_for_pytype(class, ptype)
    VALUE class;
    PyTypeObject *ptype;
{
    rb_undef_method(rb_singleton_class(class), "new");
    if (ptype->tp_as_sequence)
	rb_include_module(class, mPySequence);
    if (ptype->tp_as_mapping)
	rb_include_module(class, mPyMapping);
}

static VALUE
pytm_rbclass_noex(pclass)
    PyObject *pclass;
{
    return rb_hash_aref(hClass_py2rb, PYOBJ2NUM(pclass));
}

VALUE
pytm_rbclass(pclass)
    PyObject *pclass;
{
    VALUE class;

    class = pytm_rbclass_noex(pclass);
    if (class == Qnil) {
	pytm_refresh();
	class = pytm_rbclass_noex(pclass);
	if (class == Qnil) {
	    PyTypeObject *ptype = (PyTypeObject *)PyObject_Type(pclass);

	    if (ptype == &PyType_Type) {
		/* this is orphaned type */
		PyObject *pname = PyObject_GetAttrString(pclass, "__name__");
		int id = rb_intern(PyString_AsString(pname));

		class = rb_define_class_id(id, cPyObject);
		pytm_add_class(class, pclass);
		define_methods_for_pytype(class, pclass);
		Py_DECREF(pname);
	    } else if (ptype == &PyClass_Type) {
		PyObject *pstr;
		rb_bug("pytm_rbclass: no ruby class mapped to a python "
		       "type/class (%s)", PYOBJ_AS_STR(pclass, pstr));
		Py_DECREF(pstr);
	    }
	    Py_DECREF(ptype);
	}
    }
    return class;
}

void
pytm_add_class(rclass, pclass)
    VALUE rclass;
    PyObject *pclass;
{
    VALUE v;

    if (pytm_rbclass_noex(pclass) != Qnil)
	rb_bug("pytm_add_class: try to add already exist class");

    Py_INCREF(pclass);
    v = PYOBJ2NUM(pclass);
    rb_hash_aset(hClass_py2rb, v, rclass);
    rb_hash_aset(hClass_rb2py, rclass, v);
}


VALUE
pytm_rbclass_of_pyobj(po, ptype)
    PyObject *po;
    PyTypeObject *ptype;
{
    PyObject *pclass;
    VALUE class, key;

    if (ptype == &PyInstance_Type)
	pclass = (PyObject *)((PyInstanceObject *)po)->in_class;
    else
	pclass = (PyObject *)ptype;

    return pytm_rbclass(pclass);
}

static VALUE
pymodule_method_missing(argc, argv, self)
    int argc;
    VALUE *argv, self;
{
    if (argc < 1)
	rb_bug("no method id");
    return pyobj_send(argc-1, argv+1, pytm_pymodule(self), NUM2INT(argv[0]));
}

static VALUE
pymodule_forward(argc, argv, self)
    int argc;
    VALUE *argv, self;
{
    VALUE anc, mod;
    PyObject *po;
    int id = ruby_frame->last_func;

    anc = rb_mod_ancestors(self);
    while (RARRAY(anc)->len > 0) {
	mod = rb_ary_shift(anc);
	if (TYPE(mod) != T_MODULE)
	    continue;
	po = pytm_pymodule_noex(mod);
	if (po != NULL && py_respond_to(po, id)) {
	    return pyobj_send(argc, argv, po, id);
	}
    }
    rb_bug("pymodule_forward: no python module matched");
}

static VALUE
pymodule_forward_method(argc, argv, self)
    int argc;
    VALUE *argv, self;
{
    return pymodule_forward(argc, argv, CLASS_OF(self));
}

#define IS_UNBOUND_METHOD(po) (PyMethod_Check(po) && PyMethod_Self(po) == NULL)

static int
has_method(pclass, name)
    PyObject *pclass;
    char *name;
{
    PyObject *pattr;
    int ret = 0;

    pattr = PyObject_GetAttrString(pclass, name);
    if (pattr == NULL) {
	PyErr_Clear();
	return 0;
    }

    if (IS_UNBOUND_METHOD(pattr))
	ret = 1;

    Py_DECREF(pattr);
    return ret;
}

static VALUE
pyclass_new(argc, argv, class)
    int argc;
    VALUE *argv, class;
{
    return pyobj_send(argc, argv, pytm_pyclass(class), rb_intern("call"));
}

static void
define_methods_for_pyclass(class, pclass)
    VALUE class;
    PyObject *pclass;
{
    PyObject *pdict, *pkey, *pvalue;
    int i;
    char *name;

    rb_define_singleton_method(class, "new", pyclass_new, -1);
    if (has_method(pclass, "__getitem__")) {
	if (has_method(pclass, "__getslice__"))
	    rb_include_module(class, mPySequence);
	else
	    rb_include_module(class, mPyMapping);
    }

    pdict = PyObject_GetAttrString(pclass, "__dict__");
    if (pdict == NULL) py_raise();
    i = 0;
    while (PyDict_Next(pdict, &i, &pkey, &pvalue)) {
	pvalue = PyObject_GetAttr(pclass, pkey);
	if (IS_UNBOUND_METHOD(pvalue)) {
	    name = PyString_AsString(pkey);
	    if (name == NULL) py_raise();
	    rb_define_method(class, name, pyobj_forward, -1);
	}
	Py_DECREF(pvalue);
    }
    Py_DECREF(pdict);
}

/* returned string must be freed */
static char *
build_const_name(orig)
    char *orig;
{
    char *new;

    if (!isalpha(orig[0]))
	return NULL;

    new = (char *)xmalloc(strlen(orig) + 1);
    strcpy(new, orig);
    new[0] = toupper(new[0]);

    return new;
}

static VALUE
define_module_under(outer, orig_name)
    VALUE outer;
    char *orig_name;
{
    char *name;
    VALUE module;
    ID id;

    name = build_const_name(orig_name);
    if (name != NULL)
	id = rb_intern(name);
    else
	id = rb_intern(orig_name);

    module = rb_define_module_id(id);
    rb_set_class_path(module, outer, rb_id2name(id));
    if (name != NULL) {
	rb_const_set(outer, id, module);
	free(name);
    }

    return module;
}

static VALUE
define_class_under(outer, orig_name, super)
    VALUE outer;
    char *orig_name;
    VALUE super;
{
    char *name;
    VALUE class;
    ID id;

    name = build_const_name(orig_name);
    if (name != NULL)
	id = rb_intern(name);
    else
	id = rb_intern(orig_name);

    class = rb_define_class_id(id, super);
    rb_set_class_path(class, outer, rb_id2name(id));
    if (name != NULL) {
	rb_const_set(outer, id, class);
	free(name);
    }

    return class;
}

static void
define_module_function_forwarder(rmod, name)
    VALUE rmod;
    char *name;
{
    rb_define_private_method(rmod, name, pymodule_forward_method, -1);
    rb_define_singleton_method(rmod, name, pymodule_forward, -1);
}

/* import attributes other than class/type/module */
static void
import_attrs_from_module(rmod, pmod)
    VALUE rmod;
    PyObject *pmod;
{
    PyObject *pdic, *pkey, *pvalue;
    int i;

    pdic = PyModule_GetDict(pmod);
    i = 0;
    while (PyDict_Next(pdic, &i, &pkey, &pvalue)) {
	char *name = PyString_AsString(pkey);

	if (isupper(name[0]) &&
	    (PyType_Check(pvalue) ||
	     PyClass_Check(pvalue) ||
	     PyModule_Check(pvalue))) {
	    continue;
	}

	/* do not override */
	if (rb_respond_to(rmod, rb_intern(name))) {
	    continue;
	}

	/* constant number? */
	if (isupper(name[0]) && PyNumber_Check(pvalue)) {
	    rb_define_const(rmod, name, py2rb(pvalue));
	} else {
	    define_module_function_forwarder(rmod, name);
	}
    }
}

static void
define_class_for_type(outer, name, ptype)
    VALUE outer;
    char *name;
    PyObject *ptype;
{
    VALUE class;

    class = pytm_rbclass_noex(ptype);
    if (class == Qnil) {
	class = define_class_under(outer, name, cPyObject);
	pytm_add_class(class, ptype);
	define_methods_for_pytype(class, ptype);
    } else {
	/* allow duplicated */
	char *cname = build_const_name(name);
	if (cname != NULL) {
	    rb_define_const(outer, cname, class);
	    free(cname);
	}
    }
}

static void
import_module(fullname, pmod, pclist)
    char *fullname;
    PyObject *pmod, *pclist;
{
    PyObject *pdic, *pkey, *pvalue, *pfname;
    VALUE rmod, outer;
    char *name;
    int i, allow_type;

    /* determine outer module */
    name = fullname + strlen(fullname);
    while (name > fullname && *name != '.') name--;
    if (*name == '.') {
	char *outer_name;
	int len;
	PyObject *pouter;

	name++;
	len = name - fullname;
	outer_name = ALLOCA_N(char, len);
	strncpy(outer_name, fullname, len - 1);
	outer_name[len - 1] = '\0';
	pouter = PyDict_GetItemString(pdModules, outer_name);
	outer = pytm_rbmodule(pouter);
    } else {
	outer = mPy;
    }

    /* define in ruby level */
    rmod = define_module_under(outer, name);
    rb_define_singleton_method(rmod, "method_missing",
			       pymodule_method_missing, -1);
    pytm_add_module(rmod, pmod);
    import_attrs_from_module(rmod, pmod);

    /* allow type only under C module or "types" module */
    allow_type = 1;
    pfname = PyObject_GetAttrString(pmod, "__file__");
    if (pfname == NULL)
	PyErr_Clear();
    if (pfname != NULL && strcmp(fullname, "types") != 0) {
	char *fname = PyString_AsString(pfname);
	int len = strlen(fname);
	if (len > 4 && strcmp(fname + len - 4, ".pyc") == 0) {
	    /* this is a normal module written in python */
	    allow_type = 0;
	}
    }
    Py_XDECREF(pfname);

    pdic = PyModule_GetDict(pmod);
    i = 0;
    while (PyDict_Next(pdic, &i, &pkey, &pvalue)) {
	PyTypeObject *ptype = (PyTypeObject *)PyObject_Type(pvalue);

	if (ptype == &PyType_Type) {
	    define_class_for_type(rmod, PyString_AsString(pkey), pvalue);
	} else if (ptype == &PyClass_Type) {
	    if (PySequence_In(pclist, pvalue) == 0)
		PyList_Append(pclist, pvalue);
	}
	Py_DECREF(ptype);
    }
}

static VALUE
find_super(pclass)
    PyClassObject *pclass;
{
    PyObject *pbases;

    if ((PyObject *)pclass == PyExc_Exception)
	return rb_eException;
    if ((PyObject *)pclass == PyExc_StandardError)
	return rb_eStandardError;

    pbases = pclass->cl_bases;
    if (pbases == NULL || PyTuple_Size(pbases) == 0)
	/* no base class */
	return cPyObject;
    else {
	/* return right-most class among the bases as super class */
	PyObject *psuper = PySequence_GetItem(pbases, -1);
	VALUE super = pytm_rbclass_noex(psuper);
	Py_DECREF(psuper);
	return super;
    }
}

static VALUE
define_class(pclass)
    PyObject *pclass;
{
    VALUE class, module, super;
    PyObject *pmod, *pmname;
    char *mname, *cname, *new_cname;

    class = pytm_rbclass_noex(pclass);
    if (class != Qnil)
	return class; /* already exist */

    super = find_super(pclass);
    if (super == Qnil)
	return Qnil;

    pmname = PyObject_GetAttrString(pclass, "__module__");
    mname = PyString_AsString(pmname);
    pmod = PyDict_GetItemString(pdModules, mname);
    Py_XDECREF(pmname);
    if (pmod == NULL) {
	rb_warning("define_class: cannot find outer module");
	module = mPy;
    } else {
	module = pytm_rbmodule(pmod);
    }
    cname = PyString_AsString(((PyClassObject*)pclass)->cl_name);
    class = define_class_under(module, cname, super);
    pytm_add_class(class, pclass);
    define_methods_for_pyclass(class, pclass);
    return class;
}

static void
pytm_refresh(void)
{
    PyObject *pmod, *pname, *pcname, *pclist, *mnames;
    VALUE mod;
    int i, prev_len;

    pclist = PyList_New(0);

    /* for each newly imported module */
    mnames = PyDict_Keys(pdModules);
    if (mnames == NULL || PyList_Sort(mnames) == -1)
	rb_bug("");
    for (i = 0; i < PyList_GET_SIZE(mnames); i++) {
	char *mname;

	pname = PyList_GetItem(mnames, i);
	pmod = PyDict_GetItem(pdModules, pname);
	if (!PyModule_Check(pmod) || pytm_rbmodule_noex(pmod) != Qnil)
	    /* this module is already defined in ruby level */
	    continue;

	mname = PyString_AsString(pname);
	PyDict_SetItemString(pdMainDict, mname, pmod);
	import_module(mname, pmod, pclist);
    }
    Py_DECREF(mnames);

    /*
     * Loop while list is not empty, because a class cannot
     * be defined until its super class is defined, 
     */
    prev_len = PyList_GET_SIZE(pclist) + 1;
    while (PyList_GET_SIZE(pclist) > 0) {
	/* If list is not shortening, loop forever */
	if (prev_len <= PyList_GET_SIZE(pclist))
	    rb_bug("pytm_import_pymodule: cannot find super class");
	prev_len = PyList_GET_SIZE(pclist);

	for (i = PyList_GET_SIZE(pclist) - 1; i >= 0; i--) {
	    PyObject *pclass;
	    VALUE class;

	    pclass = PyList_GET_ITEM(pclist, i);
	    class = define_class(pclass);
	    if (class != Qnil)
		/* this class is defined */
		PySequence_DelItem(pclist, i);
	}
    }
    Py_DECREF(pclist);
}

VALUE
pytm_import_pymodule(module, name)
    VALUE module;
    char *name;
{
    PyObject *pmod;
    int isnew = 0;

    pmod = PyDict_GetItemString(pdModules, name);
    if (pmod != NULL && pytm_rbmodule_noex(pmod) != Qnil)
	return Qnil; /* already imported in both python and ruby */

    if (pmod == NULL) {
	/* not imported in python level */
	pmod = PyImport_ImportModuleEx(name, pdMainDict, pdMainDict, NULL);
	if (pmod == NULL)
	    py_raise();
    }
    if (pytm_rbmodule_noex(pmod) == Qnil)
	isnew = 1;
    pytm_refresh();
    if (isnew)
	return pytm_rbmodule(pmod);
    else
	return Qnil;
}

static VALUE
pytrace2rbstr(ptrace)
    PyObject *ptrace;
{
    PyObject *plineno, *pframe, *pcode, *pfile, *pfunc;
    int lineno;
    VALUE str;

    plineno = PyObject_GetAttrString(ptrace, "tb_lineno");
    pframe  = PyObject_GetAttrString(ptrace, "tb_frame");
    pcode   = PyObject_GetAttrString(pframe, "f_code");
    pfile   = PyObject_GetAttrString(pcode, "co_filename");
    pfunc   = PyObject_GetAttrString(pcode, "co_name");
    str = rb_str_new2(PyString_AsString(pfile));
    rb_str_cat(str, ":", 1);
    rb_str_concat(str, rb_obj_as_string(INT2NUM(PyInt_AsLong(plineno))));
    rb_str_cat(str, ":in '", 5);
    rb_str_cat(str, PyString_AsString(pfunc), PyString_Size(pfunc));
    rb_str_cat(str, "'", 1);
    Py_DECREF(plineno);
    Py_DECREF(pframe);
    Py_DECREF(pcode);
    Py_DECREF(pfile);
    Py_DECREF(pfunc);
    return str;
}

static VALUE
pytm_rbbacktrace(ptrace)
    PyObject *ptrace;
{
    VALUE trace = rb_ary_new();

    Py_INCREF(ptrace);
    while (ptrace != Py_None) {
	PyObject *pnext;

	rb_ary_unshift(trace, pytrace2rbstr(ptrace));
	pnext = PyObject_GetAttrString(ptrace, "tb_next");
	Py_DECREF(ptrace);
	ptrace = pnext;
    }
    Py_DECREF(ptrace);
    return trace;
}

VALUE
pytm_rbexception(ptype, pvalue, ptrace)
    PyObject *ptype, *pvalue, *ptrace; /* pvalue and ptrace may NULL */
{
    PyObject *pvalue_s;
    char *emsg;
    char buf[BUFSIZ]; /* this size is derived from ruby/error.c */
    VALUE etype, exc, trace, trace_py;

    emsg      = PYOBJ_AS_STR(pvalue, pvalue_s);
    if (PyString_Check(ptype)) {
	etype = rb_eRuntimeError;
	snprintf(buf, BUFSIZ, "%s (%s)", emsg, PyString_AsString(ptype));
    } else {
	etype = pytm_rbclass(ptype);
	snprintf(buf, BUFSIZ, "%s", emsg);
    }
    Py_XDECREF(pvalue_s);

    exc = rb_exc_new2(etype, buf);
    if (ptrace != NULL) {
	trace_py = pytm_rbbacktrace(ptrace);
	trace = rb_funcall(rb_mKernel, id_caller, 1, INT2FIX(0));
	rb_ary_concat(trace_py, trace);
	rb_funcall(exc, id_set_backtrace, 1, trace_py);
    }
    return exc;
}

static VALUE
const_copy_iter(cname, mod)
    VALUE cname, mod;
{
    rb_define_const(mPy, RSTRING(cname)->ptr,
		    rb_const_get_at(mod, rb_intern(RSTRING(cname)->ptr)));
    return Qnil;
}

static PyObject *
rbary2pylist(ary)
    VALUE ary;
{
    PyObject *plist;
    int i;

    plist = PyList_New(0);
    if (plist == NULL) py_raise();
    for (i = 0; i < RARRAY(ary)->len; i++) {
	PyObject *a = rb2py(RARRAY(ary)->ptr[i]);
	if (PyList_Append(plist, a) != 0) py_raise();
	Py_XDECREF(a);
    }
    return plist;
}

void
pytm_init(void)
{
    VALUE mPyExceptions, ex_consts;

    id_caller = rb_intern("caller");
    id_set_backtrace = rb_intern("set_backtrace");

    hModule_py2rb = rb_hash_new();
    hModule_rb2py = rb_hash_new();
    hClass_py2rb = rb_hash_new();
    hClass_rb2py = rb_hash_new();
    rb_global_variable(&hModule_py2rb);
    rb_global_variable(&hModule_rb2py);
    rb_global_variable(&hClass_py2rb);
    rb_global_variable(&hClass_rb2py);

    pdModules = PyImport_GetModuleDict();
    pmBuiltin = PyDict_GetItemString(pdModules, "__builtin__");
    pmMain    = PyDict_GetItemString(pdModules, "__main__");
    pmSys     = PyDict_GetItemString(pdModules, "sys");
    pdMainDict = PyModule_GetDict(pmMain);
    pdSysDict  = PyModule_GetDict(pmSys);
    Py_INCREF(pmBuiltin);
    Py_INCREF(pmMain);
    Py_INCREF(pdModules);
    Py_INCREF(pdMainDict);
    Py_INCREF(pdSysDict);

    pytm_add_module(mPyBuiltin, pmBuiltin);
    import_attrs_from_module(mPyBuiltin, pmBuiltin);
    pytm_add_module(mPy, pmMain);
    import_attrs_from_module(mPy, pmMain);
    import_attrs_from_module(mPy, pmBuiltin);
    rb_define_singleton_method(mPy, "method_missing",
			       pymodule_method_missing, -1);
    /*pytm_import_pymodule(mPy, "sys");*/
    pytm_import_pymodule(mPy, "types");
    pytm_import_pymodule(mPy, "exceptions");
    /* define exceptins also under module Py */
    mPyExceptions = rb_const_get_at(mPy, rb_intern("Exceptions"));
    ex_consts = rb_mod_constants(mPyExceptions);
    rb_iterate(rb_each, ex_consts, const_copy_iter, mPyExceptions);

    /* rubymodule */
    initruby();
    pytm_add_class(rb_cObject, (PyObject *)&RbObject_Type);
    pytm_import_pymodule(mPy, "ruby");

    /* set sys.argv */
    if (!PyMapping_HasKeyString(pdSysDict, "argv")) {
	PyObject *av;
	VALUE argv;

	argv = rb_eval_string("[$0] + $*");
	av = rbary2pylist(argv);
	PyMapping_SetItemString(pdSysDict, "argv", av);
	Py_XDECREF(av);
    }
}
