/*
 *  rubymodule.c
 *
 *  $Id: rubymodule.c,v 1.2 1999/11/08 05:45:00 fukusima Exp $
 *
 *  Copyright (C) 1999  Masaki Fukushima
 *
 *  Implements 'ruby' module for Python.
 */

#include "ruby-python.h"

typedef struct RbObject {
    PyObject_HEAD
    VALUE value;
    struct RbObject *prev;
    struct RbObject *next;
} RbObject;

VALUE
RbObject_GetValue(self)
    PyObject *self;
{
    if (!RbObject_Check(self))
	rb_bug("not RbObject");
    return ((RbObject *)self)->value;
}

/*
 * Implementation of RbObject.
 */

static PyObject *RbMethod_New(RbObject *, ID);
static RbObject object_list;

PyObject *
RbObject_New(value)
    VALUE value;
{
    RbObject *self;

    self = PyObject_NEW(RbObject, &RbObject_Type);
    if (self == NULL)
	return NULL;

    self->value = value;
    self->prev = object_list.prev;
    self->next = &object_list;
    object_list.prev->next = self;
    object_list.prev = self;
    return (PyObject *)self;
}

static void
RbObject_dealloc(self)
    RbObject *self;
{
    self->prev->next = self->next;
    self->next->prev = self->prev;
    PyMem_DEL(self);
}

static int
RbObject_print(self, fp, flags)
    RbObject *self;
    FILE *fp;
    int flags;
{
    fprintf(fp, RSTRING(rb_obj_as_string(self->value))->ptr);
    return 0;
}

static PyObject *
RbObject_repr(self)
    RbObject *self;
{
    VALUE s = rb_inspect(self->value);
    return PyString_FromStringAndSize(RSTRING(s)->ptr, RSTRING(s)->len);
}

static int
RbObject_compare(self, other)
    RbObject *self;
    PyObject *other;
{
    VALUE v;
    v = rb_funcall(self->value, rb_intern("<=>"), 1, py2rb(other));
    return FIX2INT(v);
}

static PyObject *
RbObject_insert(self, args)
    RbObject *self;
    PyObject *args;
{
    PyObject *item;
    int index;
    VALUE ret;

    if (!PyArg_ParseTuple(args, "iO", &index, &item))
	return NULL;
    ret = rb_funcall(self->value, rb_intern("[]="),
		     3, INT2NUM(index), INT2FIX(0), py2rb(item));
    return rb2py(ret);
}

static PyMethodDef RbObject_methods[] = {
    {"insert",	(PyCFunction)RbObject_insert,	1},
    {NULL,		NULL}		/* sentinel */
};

static PyObject *
RbObject_getattr(self, name)
    RbObject *self;
    char *name;
{
    PyObject *m;
    int id;

    m = Py_FindMethod(RbObject_methods, (PyObject *)self, name);
    if (m != NULL)
	return m;
    PyErr_Clear();

    id = rb_intern(name);
    if ((TYPE(self->value) == T_MODULE || TYPE(self->value) == T_CLASS)
	&& rb_is_const_id(id)) {
	return rb2py(rb_const_get_at(self->value, id));
    }
    if (!rb_respond_to(self->value, id)) {
	/* add '?' and check again */
	int len = strlen(name) + 2;
	char *name2 = ALLOCA_N(char, len);
	strcpy(name2, name);
	name2[len-2] = '?';
	name2[len-1] = '\0';
	id = rb_intern(name2);
	if (!rb_respond_to(self->value, id)) {
	    char buf[BUFSIZ];
	    snprintf(buf, BUFSIZ, "undefined method `%s' for %s",
		     name, RSTRING(rb_inspect(self->value))->ptr);
	    PyErr_SetString(PyExc_AttributeError, buf);
	    return NULL;
	}
    }
    return RbMethod_New(self, id);
}

static int
RbObject_setattr(self, name, v)
    RbObject *self;
    char *name;
    PyObject *v;
{
    return 0;
}

static int
RbObject_length(self)
    RbObject *self;
{
     return NUM2INT(rb_funcall(self->value, rb_intern("length"), 0));
}

static PyObject *
RbObject_concat(self, other)
    RbObject *self;
    PyObject *other;
{
    VALUE v;

    v = rb_funcall(self->value, rb_intern("+"), 1, py2rb(other));
    return rb2py(v);
}

static PyObject *
RbObject_repeat(self, n)
    RbObject *self;
    int n;
{
    VALUE v;

    v = rb_funcall(self->value, rb_intern("*"), 1, INT2NUM(n));
    return rb2py(v);
}

static PyObject *
RbObject_item(self, index)
    RbObject *self;
    int index;
{
    VALUE v;
    int len;

    len = NUM2INT(rb_funcall(self->value, rb_intern("length"), 0));
    if (index < 0 || index >= len) {
	PyErr_SetString(PyExc_IndexError, "index out of range");
	return NULL;
    }
    v = rb_funcall(self->value, rb_intern("[]"), 1, INT2NUM(index));
    return rb2py(v);
}

static PyObject *
RbObject_slice(self, i1, i2)
    RbObject *self;
    int i1, i2;
{
    VALUE v;

    v = rb_funcall(self->value, rb_intern("[]"), 2,
		   INT2NUM(i1), INT2NUM(i2-i1));
    return rb2py(v);
}

static PyObject *
RbObject_ass_item(self, index, val)
    RbObject *self;
    int index;
    PyObject *val;
{
    VALUE v;

    v = rb_funcall(self->value, rb_intern("[]="),
		   2, INT2NUM(index), py2rb(val));
    return rb2py(v);
}

static int
RbObject_ass_slice(self, i1, i2, val)
    RbObject *self;
    int i1, i2;
    PyObject *val;
{
    rb_funcall(self->value, rb_intern("[]="),
	       3, INT2NUM(i1), INT2NUM(i2-i1), py2rb(val));
    return 0;
}

static PyObject *
RbObject_subscript(self, key)
    RbObject *self;
    PyObject *key;
{
    return rb2py(rb_funcall(self->value, rb_intern("[]"), 1, py2rb(key)));
}

static int
RbObject_ass_sub(self, key, val)
    RbObject *self;
    PyObject *key, *val;
{
    VALUE rval;
    PyObject *ret;

    if (val == NULL)
	rval = Qnil;
    else
	rval = py2rb(val);

    ret = rb2py(rb_funcall(self->value, rb_intern("[]="), 2,
			   py2rb(key), rval));
    Py_DECREF(ret);
    return 0;
}

static void
pytuple2rbarg(tuple, argc, argv)
    PyObject *tuple;
    int argc;
    VALUE *argv;
{
    int i;

    for (i = 0; i < argc; i++)
	argv[i] = py2rb(PyTuple_GET_ITEM(tuple, i));
}

static PyObject *
RbObject_call(self, arg, kw)
    RbObject *self;
    PyObject *arg, *kw;
{
    VALUE *argv, ret;
    int argc;

    argc = PyTuple_Size(arg);
    argv = ALLOCA_N(VALUE, argc);
    pytuple2rbarg(arg, argc, argv);
    ret = rb_funcall2(self->value, rb_intern("call"), argc, argv);
    return rb2py(ret);
}

static PySequenceMethods rbobject_as_sequence = {
    (inquiry)RbObject_length,		/*sq_length*/
    (binaryfunc)RbObject_concat,	/*sq_concat*/
    (intargfunc)RbObject_repeat,	/*sq_repeat*/
    (intargfunc)RbObject_item,		/*sq_item*/
    (intintargfunc)RbObject_slice,	/*sq_slice*/
    (intobjargproc)RbObject_ass_item,	/*sq_ass_item*/
    (intintobjargproc)RbObject_ass_slice,/*sq_ass_slice*/
};

static PyMappingMethods rbobject_as_mapping = {
    (inquiry)RbObject_length,		/*mp_length*/
    (binaryfunc)RbObject_subscript,	/*mp_subscript*/
    (objobjargproc)RbObject_ass_sub,	/*mp_ass_subscript*/
};

PyTypeObject RbObject_Type = {
    PyObject_HEAD_INIT(&PyType_Type)
    0,			/*ob_size*/
    "ruby object",	/*tp_name*/
    sizeof(RbObject),	/*tp_basicsize*/
    0,			/*tp_itemsize*/
    /* methods */
    (destructor)RbObject_dealloc,	/*tp_dealloc*/
    (printfunc)RbObject_print,		/*tp_print*/
    (getattrfunc)RbObject_getattr,	/*tp_getattr*/
    (setattrfunc)RbObject_setattr,	/*tp_setattr*/
    (cmpfunc)RbObject_compare,		/*tp_compare*/
    (reprfunc)RbObject_repr,		/*tp_repr*/
    0,					/*tp_as_number*/
    &rbobject_as_sequence,		/*tp_as_sequence*/
    &rbobject_as_mapping,		/*tp_as_mapping*/
    0,					/*tp_hash*/
    (ternaryfunc)RbObject_call,		/*tp_call*/
};

/*
 * Ruby method object.
 */
typedef struct RbMethod {
    PyObject_HEAD
    RbObject *self;
    ID id;
} RbMethod;

static RbMethod *meth_free_list = NULL;
static PyTypeObject RbMethod_Type;

static PyObject *
RbMethod_New(self, id)
    RbObject *self;
    ID id;
{
    RbMethod *meth;

    if (meth_free_list == NULL) {
	meth = PyObject_NEW(RbMethod, &RbMethod_Type);
	if (meth == NULL)
	    return NULL;
    } else {
	meth = meth_free_list;
	meth_free_list = (RbMethod *)meth->self;
	meth->ob_type = &RbMethod_Type;
	_Py_NewReference((PyObject *)meth);
    }

    Py_INCREF(self);
    meth->self = self;
    meth->id = id;
    return (PyObject *)meth;
}

static void
RbMethod_dealloc(meth)
    RbMethod *meth;
{
    Py_DECREF(meth->self);
    meth->self = (RbObject *)meth_free_list;
    meth_free_list = meth;
}

static PyObject *
RbMethod_Call(meth, arg, kw)
    RbMethod *meth;
    PyObject *arg, *kw;
{
    VALUE *argv, ret;
    int argc;

    argc = PyTuple_Size(arg);
    argv = ALLOCA_N(VALUE, argc);
    pytuple2rbarg(arg, argc, argv);
    ret = rb_funcall2(meth->self->value, meth->id, argc, argv);
    return rb2py(ret);
}

static PyTypeObject RbMethod_Type = {
    PyObject_HEAD_INIT(&PyType_Type)
    0,			/*ob_size*/
    "ruby method",	/*tp_name*/
    sizeof(RbMethod),	/*tp_basicsize*/
    0,			/*tp_itemsize*/
    /* methods */
    (destructor)RbMethod_dealloc,	/*tp_dealloc*/
    0,					/*tp_print*/
    0,					/*tp_getattr*/
    0,					/*tp_setattr*/
    0,					/*tp_compare*/
    0,					/*tp_repr*/
    0,					/*tp_as_number*/
    0,					/*tp_as_sequence*/
    0,					/*tp_as_mapping*/
    0,					/*tp_hash*/
    (ternaryfunc)RbMethod_Call,		/*tp_call*/
};


/*
 * Module "ruby"
 */

/* List of functions defined in ruby module */
static PyMethodDef Rb_methods[] = {
    {NULL,		NULL}		/* sentinel */
};


/*
 * For Ruby GC, all RbObject and RbMethod is maintained in a linked-list.
 */

static void
mark_all_rbobjects(dummy)
    PyObject *dummy;
{
    RbObject *rbobj;
#if DEBUG
    RbMethod *rbmeth;
    int c = 0;
#endif

    /* mark objects referenced from Python */
    rbobj = object_list.next;
    while (rbobj != &object_list) {
	rb_gc_mark(rbobj->value);
	rbobj = rbobj->next;
#if DEBUG
	c++;
#endif
    }

#if DEBUG
    fprintf(stderr, "[%d,", c);
    c = 0;
    rbmeth = (RbMethod *)meth_free_list;
    while (rbmeth != NULL) {
	c++;
	rbmeth = (RbMethod *)rbmeth->self;
    }
    fprintf(stderr, "%d]", c);
    fflush(stderr);
#endif
}

static VALUE dummy_value;

void
initruby()
{
    PyObject *m, *d;

    /* setup lists */
    object_list.value = Qnil;
    object_list.next  = &object_list;
    object_list.prev  = &object_list;

    /* dummy ruby global variable */
    dummy_value = Data_Wrap_Struct(rb_cObject, mark_all_rbobjects,
				   0, NULL);
    rb_global_variable(&dummy_value);
    
    /* Create the module and add the functions */
    m = Py_InitModule("ruby", Rb_methods);
    PyDict_SetItemString(pdMainDict, "ruby", m);
    d = PyModule_GetDict(m);
    PyDict_SetItemString(d, "Object", (PyObject *)&RbObject_Type);
    PyDict_SetItemString(d, "Method", (PyObject *)&RbMethod_Type);
}
