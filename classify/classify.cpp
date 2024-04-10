#include "classify.h"

#include <algorithm>
#include <fstream>
#include <string>
#include <tuple>

bool compare_currs(data_point i, data_point j) { return i.curr < j.curr; }

bool Model::test_model() {
  double curr_diff = 0;
  for (auto data_it = data_fifo.begin(); data_it != data_fifo.end();
       data_it++) {
    auto first_data_point =
        data_it - 12 > data_fifo.begin() ? data_it - 12 : data_fifo.begin();
    auto last_data_point =
        data_it + 12 < data_fifo.end() ? data_it + 12 : data_fifo.end();
    double running_min =
        std::min_element(first_data_point, last_data_point, compare_currs)
            ->curr;

    curr_diff +=
        this->predict_current(data_it->perf_info, data_it->leech_curr) -
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
