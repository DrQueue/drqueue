/*
 *  python.c
 *
 *  $Id: python.c,v 1.3 2000/09/10 17:09:58 fukusima Exp $
 *
 *  Copyright (C) 1999,2000  Masaki Fukushima
 *
 * 'python' extension library for Ruby.
 */

#include "ruby-python.h"
#include "rubysig.h"
#include "env.h"
#include "st.h"

#define CURRENT_ID (ruby_frame->last_func)
#define PY_SECURE rb_secure(4)

VALUE mPy, mPyBuiltin, cPyObject;
VALUE mPySequence, mPyMapping;
VALUE oAS_KEYWORD;

static PyObject *poZero;
static PyObject *poOne;

#define GetPyObject(obj, po) \
    Data_Get_Struct(obj, PyObject, po)

/* for debug */
void
py_p(po)
    PyObject *po;
{
    PyObject_Print(po, stdout, 0);
    putchar('\n');
}

static void
free_pyobj(po)
    PyObject *po;
{
    Py_DECREF(po);
}

static VALUE
pyobj_new_with_class(po, class)
    PyObject *po;
    VALUE class;
{
    VALUE obj;

    obj = Data_Wrap_Struct(class, 0, free_pyobj, po);
    Py_INCREF(po);
    return obj;
}

static VALUE
pyobj_new(po, ptype)
    PyObject *po;
    PyTypeObject *ptype;
{
    VALUE obj, class;

    class = pytm_rbclass_of_pyobj(po, ptype);
    obj = Data_Wrap_Struct(class, 0, free_pyobj, po);
    Py_INCREF(po);
    return obj;
}

#define PYOBJ_AS_STR(po, pstr) (\
    pstr = PyObject_Str(po), PyString_AS_STRING(pstr))

#define PYINST_CLASS(po) ((PyObject *)((PyInstanceObject *)po)->in_class);

void
py_raise()
{
    PyObject *ptype, *pvalue, *ptrace;
    VALUE exc;

    PyErr_Fetch(&ptype, &pvalue, &ptrace);
    if (ptype == NULL)
	rb_bug("py_raise: no exception fetched");

    exc = pytm_rbexception(ptype, pvalue, ptrace);
    Py_DECREF(ptype);
    Py_XDECREF(pvalue);
    Py_XDECREF(ptrace);
    rb_exc_raise(exc);
}

static VALUE
py2rb_body(po, predicate, decref)
    PyObject *po;
    int predicate;
    int decref;
{
    PyTypeObject *ptype;
    VALUE obj;

    if (po == NULL) {
	if (PyErr_Occurred())
	    py_raise();
	else
	    rb_bug("Python API returns NULL, but no exception occurred.");
    }

    if (predicate) {
	if (PyObject_IsTrue(po))
	    obj = Qtrue;
	else
	    obj = Qfalse;
    } else {
	ptype = (PyTypeObject *)PyObject_Type(po);
	if (po == Py_None) {
	    obj = Qnil;
	} else if (ptype == &PyInt_Type) {
	    obj = INT2NUM(PyInt_AS_LONG(po));
	} else if (ptype == &PyLong_Type) {
	    long l = PyLong_AsLong(po);
	    if (l == -1 && PyErr_Occurred()) {
		/* Too big for C long.  Convert via string */
		PyObject *ps;

		PyErr_Clear();
		ps = PyObject_Repr(po);
#ifdef HAVE_RB_CSTR2INUM
		obj = rb_cstr2inum(PyString_AS_STRING(ps), 10);
#else
		obj = rb_str2inum(PyString_AS_STRING(ps), 10);
#endif
		Py_DECREF(ps);
	    } else {
		obj = INT2NUM(l);
	    }
	} else if (ptype == &PyFloat_Type) {
	    obj = rb_float_new(PyFloat_AS_DOUBLE(po));
	} else if (ptype == &PyString_Type) {
	    obj = rb_str_new(PyString_AS_STRING(po), PyString_GET_SIZE(po));
	} else if (ptype == &PyModule_Type) {
	    obj = pytm_rbmodule(po);
	} else if (ptype == &PyType_Type || ptype == &PyClass_Type) {
	    obj = pytm_rbclass(po);
	} else if (ptype == &PyInstance_Type) {
	    PyObject *pclass = PYINST_CLASS(po);
	    if (PyClass_IsSubclass(pclass, PyExc_Exception)) {
		obj = pytm_rbexception(pclass, po, NULL);
	    } else {
		obj = pyobj_new(po, ptype);
	    }
	} else if (ptype == &RbObject_Type) {
	    obj = RbObject_GetValue(po);
	} else {
	    obj = pyobj_new(po, ptype);
	}
	Py_DECREF(ptype);
    }
    if (decref)
	Py_DECREF(po);
    return obj;
}

VALUE
py2rb(po)
    PyObject *po;
{
    return py2rb_body(po, 0, 0);
}

static VALUE
py2rb_decref(po)
    PyObject *po;
{
    return py2rb_body(po, 0, 1);
}

static VALUE
py2rb_mayp(po, predicate)
    PyObject *po;
    int predicate;
{
    return py2rb_body(po, predicate, 0);
}

static VALUE
py2rb_decref_mayp(po, predicate)
    PyObject *po;
    int predicate;
{
    return py2rb_body(po, predicate, 1);
}

static VALUE
py2rb_decref_p(po)
    PyObject *po;
{
    return py2rb_body(po, 1, 1);
}

/*
 * Convert ruby object to python object.
 * Caller of this function receives ownership of the reference.
 */
PyObject *
rb2py(obj)
    VALUE obj;
{
    PyObject *po = NULL;

    switch (TYPE(obj)) {
    case T_NIL:
	po = Py_None; Py_INCREF(po);
	break;
    case T_TRUE:
	po = poOne; Py_INCREF(po);
	break;
    case T_FALSE:
	po = poZero; Py_INCREF(po);
	break;
    case T_FIXNUM:
	po = PyInt_FromLong(FIX2INT(obj));
	break;
    case T_BIGNUM:
	/* Convert via string */
	po = PyLong_FromString(RSTRING(rb_big2str(obj, 10))->ptr, NULL, 10);
	break;
    case T_FLOAT:
	po = PyFloat_FromDouble(NUM2DBL(obj));
	break;
    case T_STRING:
	po = PyString_FromStringAndSize(RSTRING(obj)->ptr, RSTRING(obj)->len);
	break;
    case T_CLASS:
	po = pytm_pyclass_noex(obj);
	if (po != NULL) {
	    Py_INCREF(po);
	} else {
	    po = RbObject_New(obj);
	}
	break;
    case T_MODULE:
	po = pytm_pymodule_noex(obj);
	if (po != NULL) {
	    Py_INCREF(po);
	} else {
	    po = RbObject_New(obj);
	}
	break;
    case T_DATA:
	if (rb_obj_is_kind_of(obj, cPyObject)) {
	    GetPyObject(obj, po);
	    Py_INCREF(po);
	    break;
	}
	/* fall through */
    default:
	po = RbObject_New(obj);
	break;

    }

    if (po == NULL) {
	if (PyErr_Occurred())
	    py_raise();
	else
	    rb_bug("rb2py: failed to convert ruby object to python object");
    }
    return po;
}

PyObject *
rbarg2pytuple(argc, argv)
    int argc;
    VALUE *argv;
{
    PyObject *tuple, *po;
    int i;

    tuple = PyTuple_New(argc);
    for (i = 0; i < argc; i++) {
	po = rb2py(argv[i]);
	PyTuple_SET_ITEM(tuple, i, po);
	/*
	 * Because PyTuple_SET_ITEM does not Py_INCREF,
	 * we transfer reference to tuple.
	 */
    }
    return tuple;
}

/*
 * Functions in module Py
 */

static VALUE
py_import(module, vname)
    VALUE module, vname;
{
    PY_SECURE;
    Check_SafeStr(vname);
    return pytm_import_pymodule(module, RSTRING(vname)->ptr);
}

static VALUE
py_eval_body(module, start, expr, pred)
    VALUE module;
    int start;
    VALUE expr;
    int pred;
{
    PyObject *pret, *pglobals, *plocals;

    PY_SECURE;
    Check_SafeStr(expr);
    pglobals = pdMainDict;
    plocals = pdMainDict;
    pret = PyRun_String(RSTRING(expr)->ptr, start,
			pglobals, plocals);
    return py2rb_decref_mayp(pret, pred);
}

static VALUE
py_eval(module, expr)
    VALUE module, expr;
{
    return py_eval_body(module, Py_eval_input, expr, 0);
}

static VALUE
py_eval_p(module, expr)
    VALUE module, expr;
{
    return py_eval_body(module, Py_eval_input, expr, 1);
}

static VALUE
py_exec(module, expr)
    VALUE module, expr;
{
    py_eval_body(module, Py_file_input, expr, 0);
    return Qnil;
}

static VALUE
py_send_to(argc, argv, module)
    int argc;
    VALUE *argv, module;
{
    PyObject *recv;
    VALUE ret;

    if (argc == 0)
	rb_raise(rb_eArgError, "no receiver given");
    else if (argc == 1)
	rb_raise(rb_eArgError, "no method name given");

    recv = rb2py(argv[0]);
    ret = pyobj_send(argc-2, argv+2, recv, rb_to_id(argv[1]));
    Py_DECREF(recv);
    return ret;
}

static VALUE
py_tuple(argc, argv, module)
    int argc;
    VALUE *argv, module;
{
    return py2rb_decref(rbarg2pytuple(argc, argv));
}

/*
 * Py::Object (super class of all python object)
 */

static VALUE
pyobj_equal(self, other)
    VALUE self, other;
{
    PyObject *po, *pother;
    int result;

    if (self == other)
	return Qtrue;

    GetPyObject(self, po);
    if (TYPE(other) != T_DATA || !rb_obj_is_kind_of(other, cPyObject))
	return Qfalse;

    GetPyObject(other, pother);
    if (PyObject_Cmp(po, pother, &result) == -1)
	py_raise();

    return (result == 0 ? Qtrue : Qfalse);
}

static VALUE
pyobj_cmp(self, other)
    VALUE self, other;
{
    PyObject *po, *pother;
    int result;

    if (self == other)
	return INT2FIX(0);

    GetPyObject(self, po);
    if (TYPE(other) == T_DATA && rb_obj_is_kind_of(other, cPyObject)) {
	GetPyObject(other, pother);
	Py_INCREF(pother);
    } else {
	pother = rb2py(other);
    }

    if (PyObject_Cmp(po, pother, &result) == -1) {
	Py_DECREF(pother);
	py_raise();
    } else {
	Py_DECREF(pother);
	return INT2FIX(result);
    }
}

static VALUE
pyobj_hash(self)
    VALUE self;
{
    PyObject *po;

    GetPyObject(self, po);
    return INT2NUM(PyObject_Hash(po));
}

static VALUE
pyobj_to_s(self)
    VALUE self;
{
    PyObject *po;

    GetPyObject(self, po);
    return py2rb_decref(PyObject_Str(po));
}

static VALUE
pyobj_inspect(self)
    VALUE self;
{
    PyObject *po;

    GetPyObject(self, po);
    return py2rb_decref(PyObject_Repr(po));
}

/* call a callable object */
PyObject *
pycall_call(argc, argv, pcallable)
    int argc;
    VALUE *argv;
    PyObject *pcallable;
{
    PyObject *pargs, *pret;

    pargs = rbarg2pytuple(argc, argv);
    TRAP_BEG;
    pret = PyObject_CallObject(pcallable, pargs);
    TRAP_END;
    Py_DECREF(pargs);
    return pret;
}

static int
rbhash2pykw_i(key, value, dict)
    VALUE key, value;
    PyObject *dict;
{
    PyObject *pkey, *pval;
    char *name;
    int ret;

    name = rb_id2name(rb_to_id(key));
    if (name == NULL) {
	return ST_STOP;
    }
    pkey = PyString_FromString(name);
    pval = rb2py(value);
    ret = PyDict_SetItem(dict, pkey, pval);
    Py_DECREF(pkey);
    Py_DECREF(pval);
    if (ret == -1)
	py_raise();
    return ST_CONTINUE;
}

static PyObject *
rbhash2pykw(hash)
    VALUE hash;
{
    PyObject *dict;

    dict = PyDict_New();
    st_foreach(RHASH(hash)->tbl, rbhash2pykw_i, dict);
    if (PyDict_Size(dict) != RHASH(hash)->tbl->num_entries) {
	Py_DECREF(dict);
	return NULL;
    }
    return dict;
}

static PyObject *
pycall_call_kw(argc, argv, kw, pcallable)
    int argc;
    VALUE *argv, kw;
    PyObject *pcallable;
{
    PyObject *pargs, *pkw, *pret;

    if (kw == Qnil)
	return pycall_call(argc, argv, pcallable);

    pargs = rbarg2pytuple(argc, argv);
    pkw = rbhash2pykw(kw);
    if (pkw == NULL) {
	Py_DECREF(pargs);
	PyErr_SetString(PyExc_TypeError, "invalid keyword");
	return NULL;
    }
    TRAP_BEG;
    pret = PyEval_CallObjectWithKeywords(pcallable, pargs, pkw);
    TRAP_END;
    Py_DECREF(pargs);
    Py_DECREF(pkw);
    return pret;
}

int
py_respond_to(po, id)
    PyObject *po;
    ID id;
{
    PyObject *pmeth;
    char *method;

    method = rb_id2name(id);
    /* check predicate */
    if (method[strlen(method)-1] == '?') {
	char *old;
	int len;
	old = method;
	len = strlen(method);
	method = ALLOCA_N(char, len);
	strncpy(method, old, len);
	method[len-1] = '\0';
    }

    pmeth = PyObject_GetAttrString(po, method);
    if (pmeth == NULL && po == pmMain) {
	/* search builtin */
	PyErr_Clear();
	pmeth = PyObject_GetAttrString(pmBuiltin, method);
    }
    if (pmeth == NULL) {
	PyErr_Clear();
	return 0;
    } else {
	Py_DECREF(pmeth);
	return 1;
    }
}

static VALUE
pyobj_respond_to(argc, argv, self)
    int argc;
    VALUE *argv;
    VALUE self;
{
    PyObject *po;
    VALUE mid, priv;
    ID id;

    GetPyObject(self, po);
    rb_scan_args(argc, argv, "11", &mid, &priv);
    id = rb_to_id(mid);
    if (py_respond_to(po, id)) {
	return Qtrue;
    }
    return rb_funcall2(self, id, argc, argv);
}

VALUE
pyobj_send(argc, argv, po, id)
    int argc;
    VALUE *argv;
    PyObject *po;
    ID id;
{
    PyObject *pret;
    VALUE keyword = Qnil;
    char *method;
    int predicate = 0, orig_argc = argc;

    PY_SECURE;
    method = rb_id2name(id);
    if (method == NULL) rb_raise(rb_eArgError, "unknown symbol");
    /* check predicate */
    if (method[strlen(method)-1] == '?') {
	char *old;
	int len;
	old = method;
	len = strlen(method);
	method = ALLOCA_N(char, len);
	strncpy(method, old, len);
	method[len-1] = '\0';
	predicate = 1;
    }

    /* check keyword arguments */
    if (argc >= 2 && TYPE(argv[argc-1]) == T_HASH &&
	argv[argc-2] == oAS_KEYWORD) {
	keyword = argv[argc-1];
	argc -= 2;
    }

    /* if block is given, then pass it as the last argument */
    if (rb_iterator_p()) {
	VALUE proc, *new_argv;
	int i;

	proc = rb_f_lambda();
	new_argv = ALLOCA_N(VALUE, argc+1);
	for (i = 0; i < argc; i++) new_argv[i] = argv[i];
	new_argv[i] = proc;
	argc++;
	argv = new_argv;
    }

    if (PyCallable_Check(po) && strcmp(method, "call") == 0) {
	/* calling a callable object */
	pret = pycall_call_kw(argc, argv, keyword, po);
    } else {
	PyObject *pmethod = PyObject_GetAttrString(po, method);
	if (pmethod == NULL && po == pmMain) {
	    /* search builtin */
	    PyErr_Clear();
	    pmethod = PyObject_GetAttrString(pmBuiltin, method);
	}

	if (pmethod == NULL) {
	    /* no such attribute */
	    py_raise();
	}

	if (PyCallable_Check(pmethod) && !PyClass_Check(pmethod) &&
	    (!PyMethod_Check(pmethod) || PyMethod_Self(pmethod) == po)) {
	    /* Function or bound method.  Call it. */
	    pret = pycall_call_kw(argc, argv, keyword, pmethod);
	} else {
	    /* Non-method attribute */
	    if (argc != 0 || keyword != Qnil) {
		Py_DECREF(pmethod);
		rb_raise(rb_eArgError, "Wrong # of arguments(%d for 0)",
			 orig_argc);
	    }
	    Py_INCREF(pmethod);
	    pret = pmethod;
	}
	Py_DECREF(pmethod);
    }
    return py2rb_decref_mayp(pret, predicate);
}

static VALUE
pyobj_method_missing(argc, argv, self)
    int argc;
    VALUE *argv, self;
{
    PyObject *po;

    GetPyObject(self, po);
    if (argc < 1)
	rb_bug("no method id");
    return pyobj_send(argc-1, argv+1, po, NUM2INT(argv[0]));
}

/* simply forward all messages to python object */
VALUE
pyobj_forward(argc, argv, self)
    int argc;
    VALUE *argv, self;
{
    PyObject *po;
    ID id = CURRENT_ID;
    GetPyObject(self, po);
    return pyobj_send(argc, argv, po, id);
}

static VALUE
pyobj_length(self)
    VALUE self;
{
    PyObject *po;
    int len;

    GetPyObject(self, po);
    len = PyObject_Length(po);
    if (len == -1)
	py_raise();
    return INT2NUM(len);
}

/*
 * Py::Sequence Mix-in
 *
 * Some routines are borrowed from ruby/array.c for the purpose
 * of compatibility.
 */
static VALUE
pyseq_entry(po, offset)
    PyObject *po;
    int offset;
{
    int len = PyObject_Length(po);

    if (len == -1) py_raise();
    if (len == 0) return Qnil;

    if (offset < 0) {
	offset = len + offset;
    }
    if (offset < 0 || len <= offset) {
	return Qnil;
    }

    return py2rb_decref(PySequence_GetItem(po, offset));
}

static VALUE
pyseq_subseq(po, beg, len)
    PyObject *po;
    int beg, len;
{
    VALUE ary2;
    int seq_len = PyObject_Length(po);

    if (seq_len == -1) py_raise();
    if (len < 0) return Qnil;
    if (beg > seq_len) return Qnil;
    if (beg < 0) return Qnil;
    if (beg + len > seq_len) {
	len = seq_len - beg;
    }
    if (len < 0) {
	len = 0;
    }
    return py2rb_decref(PySequence_GetSlice(po, beg, beg+len));
}

static VALUE
pyseq_aref(argc, argv, self)
    int argc;
    VALUE *argv, self;
{
    PyObject *po;
    VALUE arg1, arg2;
    long beg, len, seq_len;

    GetPyObject(self, po);
    seq_len = PyObject_Length(po);
    if (seq_len == -1)
	py_raise();
    if (rb_scan_args(argc, argv, "11", &arg1, &arg2) == 2) {
	beg = NUM2INT(arg1);
	len = NUM2INT(arg2);
	if (beg < 0) {
	    beg = seq_len + beg;
	}
	return pyseq_subseq(po, beg, len);
    }

    /* special case - speeding up */
    if (FIXNUM_P(arg1)) {
	return pyseq_entry(po, FIX2INT(arg1));
    }
    else if (TYPE(arg1) == T_BIGNUM) {
	rb_raise(rb_eIndexError, "index too big");
    }
    else {
	/* check if idx is Range */
	switch (rb_range_beg_len(arg1, &beg, &len, seq_len, 0)) {
	case Qfalse:
	    break;
	case Qnil:
	    return Qnil;
	default:
	    return pyseq_subseq(po, beg, len);
	}
    }
    return pyseq_entry(po, NUM2INT(arg1));
}

static VALUE
pyseq_aset(self, index, val)
    VALUE self, index, val;
{
    PyObject *po, *pval;
    int ret;

    GetPyObject(self, po);
    Check_Type(index, T_FIXNUM); /* FIXME: shuold allow Range */
    pval = rb2py(val);
    ret = PySequence_SetItem(po, FIX2INT(index), pval);
    Py_DECREF(pval);
    if (ret == -1)
	py_raise();
    return val;
}

static VALUE
pyseq_each(self)
    VALUE self;
{
    PyObject *po, *pelm;
    int len, i;

    GetPyObject(self, po);
    len = PyObject_Length(po);
    if (len == -1) rb_bug("failed to get length");
    for (i = 0; i < len; i++) {
	pelm = PySequence_GetItem(po, i);
	if (pelm == NULL) py_raise();
	rb_yield(py2rb_decref(pelm));
    }
    return Qnil;
}

static VALUE
pyseq_include(self, val)
    VALUE self, val;
{
    PyObject *po, *pval;
    int result;

    GetPyObject(self, po);
    pval = rb2py(val);
    result = PySequence_In(po, pval);
    Py_DECREF(pval);

    switch (result) {
    case -1: py_raise(); break;
    case  0: return Qfalse;
    case  1: return Qtrue;
    default: rb_bug("unknown return value");
    }
}

/*
 * Py::Mapping Mix-in
 */
static VALUE
pymap_aref(self, key)
    VALUE self, key;
{
    PyObject *po, *pkey, *pret;

    GetPyObject(self, po);
    pkey = rb2py(key);
    pret = PyObject_GetItem(po, pkey);
    Py_DECREF(pkey);
    if (pret == NULL && PyErr_ExceptionMatches(PyExc_KeyError)) {
	PyErr_Clear();
	return Qnil;
    }
    return py2rb_decref(pret);
}

static VALUE
pymap_aset(self, key, val)
    VALUE self, key, val;
{
    PyObject *po, *pkey, *pval;
    int ret;

    GetPyObject(self, po);
    pkey = rb2py(key);
    pval = rb2py(val);
    ret = PyObject_SetItem(po, pkey, pval);
    Py_DECREF(pkey);
    Py_DECREF(pval);
    if (ret == -1)
	py_raise();
    return val;
}

static VALUE
pymap_each(self)
    VALUE self;
{
    PyObject *po, *pitems, *pitem, *pkey, *pval;
    VALUE ary;
    int len, i;

    GetPyObject(self, po);
    pitems = PyMapping_Items(po);
    if (pitems == NULL)
	py_raise();
    len = PyList_Size(pitems);
    for (i = 0; i < len; i++) {
	pitem = PyList_GetItem(pitems, i);
	if (pitem == NULL)
	    py_raise();
	pkey = PyTuple_GetItem(pitem, 0);
	pval = PyTuple_GetItem(pitem, 1);
	if (pkey == NULL || pval == NULL)
	    py_raise();
	ary = rb_ary_new3(2, py2rb(pkey), py2rb(pval));
	rb_yield(ary);
    }
    Py_DECREF(pitems);
    return Qnil;
}

static VALUE
pymap_each_key(self)
    VALUE self;
{
    PyObject *po, *pkeys, *pkey;
    int len, i;

    GetPyObject(self, po);
    pkeys = PyMapping_Keys(po);
    if (pkeys == NULL)
	py_raise();
    len = PyList_Size(pkeys);
    for (i = 0; i < len; i++) {
	pkey = PyList_GetItem(pkeys, i);
	if (pkey == NULL)
	    py_raise();
	rb_yield(py2rb(pkey));
    }
    Py_DECREF(pkeys);
    return Qnil;
}

static VALUE
pymap_each_value(self)
    VALUE self;
{
    PyObject *po, *pvalues, *pval;
    int len, i;

    GetPyObject(self, po);
    pvalues = PyMapping_Values(po);
    if (pvalues == NULL)
	py_raise();
    len = PyList_Size(pvalues);
    for (i = 0; i < len; i++) {
	pval = PyList_GetItem(pvalues, i);
	if (pval == NULL)
	    py_raise();
	rb_yield(py2rb(pval));
    }
    Py_DECREF(pvalues);
    return Qnil;
}

/*
 * Misc.
 */

static VALUE
f_require_override(obj, v_fname)
    VALUE obj, v_fname;
{
    char *fname = RSTRING(v_fname)->ptr;
    char *prefix = "python/", *realname = NULL;

    PY_SECURE;
    Check_SafeStr(v_fname);
    /* prefix exists? */
    if (strncmp(fname, prefix, strlen(prefix)) == 0)
	realname = fname + strlen(prefix);

    if (realname == NULL)
	return rb_f_require(obj, v_fname);

    if (pytm_import_pymodule(mPy, realname) == Qnil)
	return Qfalse;
    return Qtrue;
}

static VALUE
kw_to_s(obj)
    VALUE obj;
{
    return rb_str_new2("AS_KEYWORD");
}

#ifdef DEBUG
extern long _Py_RefTotal;
static VALUE
get_Py_RefTotal()
{
    return INT2NUM((int)_Py_RefTotal);
}
#endif /* DEBUG */

void Init_python() {
    Py_Initialize();
    poZero = PyInt_FromLong(0);
    poOne  = PyInt_FromLong(1);

    mPy = rb_define_module("Py");
    rb_define_module_function(mPy, "import", py_import, 1);
    rb_define_module_function(mPy, "eval", py_eval, 1);
    rb_define_module_function(mPy, "eval?", py_eval_p, 1);
    rb_define_module_function(mPy, "exec", py_exec, 1);
    rb_define_module_function(mPy, "send_to", py_send_to, -1);
    rb_define_module_function(mPy, "Tuple", py_tuple, -1);
    oAS_KEYWORD = rb_obj_alloc(rb_cObject);
    rb_define_const(mPy, "AS_KEYWORD", oAS_KEYWORD);
    rb_define_const(mPy, "KW", oAS_KEYWORD);
    rb_define_singleton_method(oAS_KEYWORD, "to_s", kw_to_s, 0);
    
    mPyBuiltin = rb_define_module_under(mPy, "Builtin");

    cPyObject = rb_define_class_under(mPy, "Object", rb_cObject);
    rb_undef_method(rb_singleton_class(cPyObject), "new");
    rb_define_method(cPyObject, "==", pyobj_equal, 1);
    rb_define_method(cPyObject, "===", pyobj_equal, 1);
    rb_define_method(cPyObject, "eql?", pyobj_equal, 1);
    rb_define_method(cPyObject, "<=>", pyobj_cmp, 1);
    rb_define_method(cPyObject, "hash", pyobj_hash, 0);
    rb_define_method(cPyObject, "to_s", pyobj_to_s, 0);
    rb_define_method(cPyObject, "inspect", pyobj_inspect, 0);
    rb_define_method(cPyObject, "respond_to", pyobj_respond_to, -1);
    rb_define_method(cPyObject, "method_missing", pyobj_method_missing, -1);
    rb_include_module(cPyObject, rb_mComparable);

    mPySequence = rb_define_module_under(mPy, "Sequence");
    rb_include_module(mPySequence, rb_mEnumerable);
    rb_define_method(mPySequence, "length", pyobj_length, 0);
    rb_define_method(mPySequence, "size", pyobj_length, 0);
    rb_define_method(mPySequence, "[]", pyseq_aref, -1);
    rb_define_method(mPySequence, "[]=", pyseq_aset, 2);
    rb_define_method(mPySequence, "each", pyseq_each, 0);
    rb_define_method(mPySequence, "include?", pyseq_include, 1);

    mPyMapping = rb_define_module_under(mPy, "Mapping");
    rb_include_module(mPyMapping, rb_mEnumerable);
    rb_define_method(mPyMapping, "length", pyobj_length, 0);
    rb_define_method(mPyMapping, "size", pyobj_length, 0);
    rb_define_method(mPyMapping, "[]", pymap_aref, 1);
    rb_define_method(mPyMapping, "[]=", pymap_aset, 2);
    rb_define_method(mPyMapping, "each", pymap_each, 0);
    rb_define_method(mPyMapping, "each_pair", pymap_each, 0);
    rb_define_method(mPyMapping, "each_key", pymap_each_key, 0);
    rb_define_method(mPyMapping, "each_value", pymap_each_value, 0);

    pytm_init();

    /* override Kernel#require */
    rb_define_private_method(rb_cObject,"require", f_require_override, 1);

#ifdef DEBUG
    rb_define_virtual_variable("$Py_RefTotal", get_Py_RefTotal, NULL);
    if (RTEST(ruby_debug))
	atexit(Py_Finalize);
#endif /* DEBUG */

    if (PyErr_Occurred())
	rb_bug("Init_python: some python error");
}
