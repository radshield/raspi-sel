#include <csignal>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <unistd.h>

#include "CPUSnapshot.h"
#include "classify.h"
#include "ina3221.h"
#include "record_system.h"

static volatile bool sentinel = true;

struct OutputData {
  uint8_t trigger_count, latchup_count;
};

void int_handler(int signum) { sentinel = false; }

inline void latchup_test(Model &classify_model, RecordSystem &system_stats,
                         INA3221 &current_stats, OutputData &output_data) {
  // Test for 3 seconds and write result to disk
  std::cout << "Testing for 3 seconds!" << std::endl;
  for (int i = 0; i < 300; i++) {
    classify_model.add_datapoint(current_stats.read_currents(),
                                 system_stats.get_system_info());
    if (classify_model.test_model()) {
      std::cout << "Potential latchup detected!" << std::endl;

      uint8_t output = 0x0;

      output_data.latchup_count += 1;
      if (output_data.latchup_count > 0b00001111)
        output_data.latchup_count = 0x1;

      output |= output_data.latchup_count & 0b00001111;
      output |= (output_data.trigger_count << 4);

      std::ofstream output_file("one_byte_telemetry",
                                std::ios::out | std::ios::binary);
      output_file << output;
      output_file.close();
    }

    // Wait for 1 millisecond
    usleep(1000);
  }
}

int main(int argc, char **argv) {
  std::string model_file;
  std::tuple<double, double> predicted, actual;
  OutputData output_data;

  if (argc != 2) {
    printf("Usage: %s MODEL_FILE\n", argv[0]);
    return -1;
  }

  signal(SIGINT, int_handler);

  model_file = argv[1];

  Model classify_model(model_file);
  RecordSystem system_stats;
  INA3221 current_stats;

  // Ensure write of one_byte_telemetry
  std::ofstream output_file("one_byte_telemetry",
                            std::ios::out | std::ios::binary);
  output_file << 0x0;
  output_file.close();

  while (sentinel) {
    CPUSnapshot previousSnap;
    sleep(1);
    CPUSnapshot curSnap;

    float active_time =
        curSnap.GetActiveTimeTotal() - previousSnap.GetActiveTimeTotal();
    float idle_time =
        curSnap.GetIdleTimeTotal() - previousSnap.GetIdleTimeTotal();
    float usage = active_time / (active_time + idle_time);

    // Check if usage is idle
    if (usage < 0.05f && usage > 0.00f) {
      std::cout << "CPU active time low" << std::endl;
      // Increase trigger count now that idle is detected
      output_data.trigger_count += 1;
      if (output_data.trigger_count > 0b00001111)
        output_data.trigger_count = 0x1;

      uint8_t output = 0x0;
      output |= output_data.latchup_count & 0b00001111;
      output |= (output_data.trigger_count << 4);

      std::ofstream output_file("one_byte_telemetry",
                                std::ios::out | std::ios::binary);
      output_file << output;
      output_file.close();

      latchup_test(classify_model, system_stats, current_stats, output_data);

      // Sleep for the next 3 minutes before detecting again
      sleep(180);
    } else {
      sleep(60);
    }
  }

  return 0;
}
