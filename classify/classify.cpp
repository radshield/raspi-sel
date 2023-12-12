#include "classify.h"
#include <string>
#include <tuple>

/**
 * Checks if a connection is an attack or not
 * @param in CSV string from leader_session
 * @return chance of attack from 0 to 1
 */
std::tuple<double, double, double> Model::test_model(const std::string &in) {
  double r1, r2, r3;
  PyObject *epoch, *ret;

  Py_XINCREF(ml_model);

  epoch = PyTuple_New(2);
  PyTuple_SetItem(epoch, 0, PyBytes_FromString(in.c_str()));
  PyTuple_SetItem(epoch, 1, ml_model);

  ret = PyObject_CallObject(test_func, epoch);
  r1 = PyLong_AsDouble(PyTuple_GetItem(ret, 0));
  r2 = PyLong_AsDouble(PyTuple_GetItem(ret, 1));
  r3 = PyLong_AsDouble(PyTuple_GetItem(ret, 2));

  Py_XDECREF(epoch);
  Py_XDECREF(ret);

  return std::make_tuple(r1, r2, r3);
}

// Close references to Python objects
Model::~Model() {
  Py_XDECREF(load_func);
  Py_XDECREF(test_func);
  Py_XDECREF(ml_model);
}

// Loads Python objects needed to do linear regression
Model::Model(std::string &load_model) {
  PyObject *module, *str;
  Py_Initialize();

  PyRun_SimpleString("import sys");
  PyRun_SimpleString("import os");
  PyRun_SimpleString("sys.path.append(os.getcwd())");
  module = PyImport_Import(PyUnicode_DecodeFSDefault("classify.model"));

  load_func = PyObject_GetAttrString(module, "load_model");
  test_func = PyObject_GetAttrString(module, "test_model");

  str = PyTuple_New(1);
  PyTuple_SetItem(str, 0, Py_BuildValue("y#", load_model.c_str(), load_model.size()));
  ml_model = PyObject_CallObject(load_func, str);

  Py_XDECREF(str);
  Py_XINCREF(ml_model);
}
