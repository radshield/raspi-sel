#ifndef CLASSIFY_H
#define CLASSIFY_H

#include "record_system.h"
#include <boost/circular_buffer.hpp>
#include <string>

struct data_point {
  perf_data perf_info;
  double curr;
  double leech_curr;
};

class Model {
private:
  perf_data perf_weights;
  double bias, current_weight;
  boost::circular_buffer<data_point> data_fifo;

  /**
   * Predicts a current using the imported model
   * @param perf_info data collected from perf
   * @param leech_curr additional current draw from sensors
   * @return predicted current draw of the system
   */
  double predict_current(perf_data perf_info, double leech_curr);

public:
  /**
   * Checks if a latchup is suspected or not based on previous data in ring
   * buffer
   * @return whether a latchup is there
   */
  bool test_model();

  /**
   * Adds a datapoint into the ring buffer
   * @param currents input currents from INA3221
   * @param perf_info perf info from OS
   */
  void add_datapoint(const std::tuple<double, double, double> currents,
                     const perf_data perf_info);

  /**
   * Loads a current model from file
   * @param load_model input model file location
   */
  explicit Model(std::string &load_model);
};

#endif // CLASSIFY_H
