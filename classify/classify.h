#ifndef CLASSIFY_H
#define CLASSIFY_H

#include "record_system.h"
#include <boost/circular_buffer.hpp>
#include <string>

struct data_point {
  perf_data perf_info;
  std::tuple<double, double> current_info;
};

class Model {
private:
  perf_data perf_weights;
  double bias, current_weight;
  boost::circular_buffer<data_point> data_fifo;
  double predict_current(perf_data perf_info, double leech_curr);

public:
  bool test_model();
  void add_datapoint(const std::tuple<double, double> currents,
                     const perf_data perf_info);
  explicit Model(std::string &load_model);
};

#endif // CLASSIFY_H
