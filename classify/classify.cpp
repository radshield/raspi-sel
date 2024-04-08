#include "classify.h"

#include <fstream>
#include <string>
#include <tuple>

/**
 * Checks if a connection is an attack or not based on previous data in ring
 * buffer
 * @return whether a latchup is suspected or not
 */
bool Model::test_model() {
  double curr_diff = 0;
  for (auto data_it = data_fifo.begin(); data_it != data_fifo.end();
       data_it++) {
    curr_diff += this->predict_current(data_it->perf_info,
                                       std::get<1>(data_it->current_info)) -
                 std::get<0>(data_it->current_info);
  }

  return curr_diff >= 0.06;
}

/**
 * Predicts a current using the imported model
 * @param perf_info data collected from perf
 * @param leech_curr additional current draw from sensors
 * @return predicted current draw of the system
 */
double Model::predict_current(perf_data perf_info, double leech_curr) {
  double pred = this->bias;

  pred += this->current_weight * leech_curr;
  pred += this->perf_weights.rd_ios * perf_info.rd_ios;
  pred += this->perf_weights.wr_ios * perf_info.wr_ios;
  for (int i = 0; i < 4; i++) {
    pred += this->perf_weights.cpu_freq[i] * perf_info.cpu_freq[i];
    pred += this->perf_weights.cpu_cycles[i] * perf_info.cpu_cycles[i];
    pred += this->perf_weights.insn_rate[i] * perf_info.insn_rate[i];
    pred += this->perf_weights.cache_hit_rate[i] * perf_info.cache_hit_rate[i];
    pred += this->perf_weights.br_miss_rate[i] * perf_info.br_miss_rate[i];
    pred += this->perf_weights.bus_cycles[i] * perf_info.bus_cycles[i];
  }

  return pred;
}

/**
 * Adds a datapoint into the ring buffer
 * @param currents input currents from INA3221
 * @param perf_info perf info from OS
 */
void Model::add_datapoint(const std::tuple<double, double> currents,
                          const perf_data perf_info) {
  data_point d;
  d.perf_info = perf_info;
  d.current_info = currents;
  this->data_fifo.push_back(d);
}

/**
 * Loads a current model from file
 * @param load_model input model file location
 */
Model::Model(std::string &load_model) {
  this->data_fifo.set_capacity(300);

  std::ifstream input_file(load_model);

  input_file >> this->bias;
  input_file >> this->current_weight;

  input_file >> this->perf_weights.rd_ios;
  input_file >> this->perf_weights.wr_ios;

  for (int i = 0; i < 4; i++) {
    input_file >> this->perf_weights.cpu_freq[i];
    input_file >> this->perf_weights.cpu_cycles[i];
    input_file >> this->perf_weights.insn_rate[i];
    input_file >> this->perf_weights.cache_hit_rate[i];
    input_file >> this->perf_weights.br_miss_rate[i];
    input_file >> this->perf_weights.bus_cycles[i];
  }

  input_file.close();
}
