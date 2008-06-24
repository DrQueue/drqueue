/*
 *  ruby-python.h
 *
 *  $Id: ruby-python.h,v 1.2 2000/09/10 17:09:58 fukusima Exp $
 *
 *  Copyright (C) 1999,2000  Masaki Fukushima
 */

#ifndef RUBY_PYTHON_H
#define RUBY_PYTHON_H

#include "ruby.h"
#include "version.h"
#if (defined RUBY_VERSION_CODE) && (RUBY_VERSION_CODE >= 160)
# define HAVE_RB_CSTR2INUM 1
#endif

#ifdef DEBUG
# define Py_DEBUG
#endif
#include "Python.h"

#define PY_P(po) (po == NULL ? puts("<NULL>") :\
    (PyObject_Print((PyObject *)po, stdout, 0), puts("")))

/* python.c */
extern VALUE mPy, mPyBuiltin, cPyObject;
extern VALUE mPySequence, mPyMapping;
void py_p(PyObject *);
void py_raise(void);
PyObject *rb2py(VALUE);
VALUE py2rb(PyObject *);
PyObject *pycall_call(int, VALUE *, PyObject *);
int py_respond_to(PyObject *, ID);
VALUE pyobj_send(int, VALUE *, PyObject *, ID);
VALUE pyobj_forward(int, VALUE *, VALUE);

/* typemap.c */
extern PyObject *pmBuiltin, *pmMain, *pmSys;
extern PyObject *pdModules, *pdMainDict, *pdSysDict;
void pytm_init(void);
void pytm_add_module(VALUE, PyObject *);
PyObject *pytm_pymodule_noex(VALUE);
PyObject *pytm_pymodule(VALUE);
VALUE pytm_rbmodule(PyObject *);
void pytm_add_class(VALUE, PyObject *);
PyObject *pytm_pyclass_noex(VALUE);
PyObject *pytm_pyclass(VALUE);
VALUE pytm_rbclass(PyObject *);
VALUE pytm_rbclass_of_pyobj(PyObject *, PyTypeObject *);
VALUE pytm_import_pymodule(VALUE, char *);
VALUE pytm_rbexception(PyObject *, PyObject *, PyObject *);

/* rubymodule.c */
extern PyTypeObject RbObject_Type;
#define RbObject_Check(v) ((v)->ob_type == &RbObject_Type)
VALUE RbObject_GetValue(PyObject *);
PyObject *RbObject_New(VALUE);

#endif /* RUBY_PYTHON_H */
