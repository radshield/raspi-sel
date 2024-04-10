#include <csignal>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <unistd.h>

#include "classify.h"
#include "ina3221.h"
#include "record_system.h"

static volatile bool sentinel = true;

void int_handler(int signum) { sentinel = false; }

int main(int argc, char **argv) {
  std::string model_file;
  std::tuple<double, double> predicted, actual;
  uint8_t trigger_count, latchup_count, output;

  if (argc != 3) {
    printf("Usage: %s MODEL_FILE\n", argv[0]);
    return -1;
  }

  signal(SIGINT, int_handler);

  model_file = argv[1];

  Model classify_model(model_file);
  RecordSystem system_stats;
  INA3221 current_stats;

  while (sentinel) {
    classify_model.add_datapoint(current_stats.read_currents(),
                                 system_stats.get_system_info());

    if (classify_model.test_model()) {
      std::ofstream output_file("one_byte_telemetry",
                                std::ios::out | std::ios::binary);

      output = 0x0;

      latchup_count += 1;
      if (latchup_count > 0b00001111)
        latchup_count = 0x1;

      output |= latchup_count & 0b00001111;
      output |= (trigger_count << 4);

      output_file << output;
      output_file.close();
    }

    // Wait for 5000 seconds
    usleep(5000);
  }

  return 0;
}
