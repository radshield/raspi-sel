#ifndef CLASSIFY_H
#define CLASSIFY_H

#include <string>

#include "Python.h"

class Model {
private:
  PyObject *ml_model;
  PyObject *load_func, *test_func;
public:
  int analyze_conn(const std::string &in);
  explicit Model(std::string &load_model);
  ~Model();
};

#endif // CLASSIFY_H
