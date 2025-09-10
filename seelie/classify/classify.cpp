#include "classify.h"

#include <algorithm>
#include <fstream>
#include <string>
#include <tuple>

bool compare_currs(data_point i, data_point j) { return i.curr < j.curr; }

bool Model::test_model() {
  double curr_diff = 0;
  for (size_t i = 0; i < data_fifo.size(); i++) {
    size_t first_data_point = (i >= 12) ? i - 12 : 0;
    size_t last_data_point = (i + 12 < data_fifo.size()) ? i + 12 : data_fifo.size();

    double running_min = 100.0f;
    for (size_t it = first_data_point; it < last_data_point; it++) {
      if (data_fifo.at(it).curr < running_min)
        running_min = data_fifo.at(it).curr;
    }

    curr_diff +=
        this->predict_current(data_fifo.at(i).perf_info, data_fifo.at(i).leech_curr) -
        running_min;
  }

  return curr_diff >= 0.06;
}

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

void Model::add_datapoint(const std::tuple<double, double, double> currents,
                          const perf_data perf_info) {
  data_point d;
  d.perf_info = perf_info;
  d.curr = std::get<1>(currents);
  d.leech_curr = std::get<2>(currents);
  this->data_fifo.push_back(d);
}

Model::Model(std::string &load_model) {
  this->data_fifo.set_capacity(300);

  std::ifstream input_file(load_model);

  input_file >> this->current_weight;

  input_file >> this->perf_weights.rd_ios;
  input_file >> this->perf_weights.wr_ios;

  for (int i = 0; i < 4; i++) {
    input_file >> this->perf_weights.cpu_freq[i];
    input_file >> this->perf_weights.insn_rate[i];
    input_file >> this->perf_weights.cpu_cycles[i];
    input_file >> this->perf_weights.cache_hit_rate[i];
    input_file >> this->perf_weights.br_miss_rate[i];
    input_file >> this->perf_weights.bus_cycles[i];
  }

  input_file >> this->bias;

  input_file.close();
}
