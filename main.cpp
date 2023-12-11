#include <Python.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

static volatile bool sentinel = true;

void int_handler(int signum) {
  sentinel = false;
}


int main(int argc, char **argv) {
  struct io_stats io_stats, io_stats_last;
  struct perf_ptr perf_events[sysconf(_SC_NPROCESSORS_ONLN)];
  char buf[(NUM_EVENTS * 2 + 1) * sizeof(uint64_t)];
  struct read_format* rf = (struct read_format*) buf;
  struct timespec start, counter;

  if (argc != 3) {
    printf("Usage: %s MODEL_FILE TARGET_PID\n", argv[0]);
    return -1;
  }

  // Load 
  Py_Initialize();
  PyRun_SimpleString("import sys");
  PyRun_SimpleString("import os");
  PyRun_SimpleString("sys.path.append(os.getcwd())");
  PyObject *module = PyImport_Import(PyUnicode_DecodeFSDefault("src.classify"));

  // Setup I2C communication
  int i2c = open("/dev/i2c-1", O_RDWR);
  if (i2c == -1)
    return -2;

  if(ioctl(i2c, I2C_SLAVE, 0x40) < 0) {
    close(i2c);
    return -4;
  }

  int check_vendor_id = i2c_smbus_read_word_data(i2c, 0xFF);
  if (check_vendor_id != SIGNATURE)
    return -3;

  // Set up perf events
  for (int i = 0; i < sysconf(_SC_NPROCESSORS_ONLN); i++)
    perf_events[i] = init_perf_event(i);

  // Switch device to measurement mode
  i2c_smbus_write_word_data(i2c, REG_RESET, 0b1111111111111111);

  signal(SIGINT, int_handler);

  for (int i = 0; i < sysconf(_SC_NPROCESSORS_ONLN); i++) {
    ioctl(perf_events[0].fd[0], PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
    ioctl(perf_events[0].fd[0], PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);
  }

  clock_gettime(CLOCK_MONOTONIC_RAW, &start);

  while (sentinel) {
    // Read from INA3221
    int shunt1 = i2c_smbus_read_word_data(i2c, REG_DATA_ch1);
    int shunt2 = i2c_smbus_read_word_data(i2c, REG_DATA_ch2);
    int shunt3 = i2c_smbus_read_word_data(i2c, REG_DATA_ch3);
    // change endian, strip last 3 bits provide raw value
    float ch1_amp = shunt_to_amp(change_endian(shunt1) / 8);
    float ch2_amp = shunt_to_amp(change_endian(shunt2) / 8);
    float ch3_amp = shunt_to_amp(change_endian(shunt3) / 8);

    // Read from perf counters
    for (int cpu = 0; cpu < sysconf(_SC_NPROCESSORS_ONLN); cpu++)
      ioctl(perf_events[cpu].fd[0], PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP);

    for (int cpu = 0; cpu < sysconf(_SC_NPROCESSORS_ONLN); cpu++) {
      perf_events[cpu].cpu_freq = cpufreq_get(cpu);
      read(perf_events[cpu].fd[0], buf, sizeof(buf));
      for (int event = 0; event < rf->nr; event++) {
        if (rf->values[event].id == perf_events[cpu].id[0])
          perf_events[cpu].cpu_cycles = rf->values[event].value;
        else if (rf->values[event].id == perf_events[cpu].id[1])
          perf_events[cpu].insns = rf->values[event].value;
        else if (rf->values[event].id == perf_events[cpu].id[2])
          perf_events[cpu].cache_hit = rf->values[event].value;
        else if (rf->values[event].id == perf_events[cpu].id[3])
          perf_events[cpu].cache_miss = rf->values[event].value;
        else if (rf->values[event].id == perf_events[cpu].id[4])
          perf_events[cpu].br_insns = rf->values[event].value;
        else if (rf->values[event].id == perf_events[cpu].id[5])
          perf_events[cpu].br_miss = rf->values[event].value;
        else if (rf->values[event].id == perf_events[cpu].id[6])
          perf_events[cpu].bus_cycles = rf->values[event].value;
      }
    }

    // Read disk IO information
    read_sysfs_file_stat_work("/sys/block/mmcblk0/stat", &io_stats);

    clock_gettime(CLOCK_MONOTONIC_RAW, &counter);

    // Print out current and perf data to file
    int usecs_elapsed = (counter.tv_sec - start.tv_sec) * 1000000 + (counter.tv_nsec - start.tv_nsec) / 1000;
    printf("%f,%f,%f", ch1_amp, ch2_amp, ch3_amp);
    for (int cpu = 0; cpu < sysconf(_SC_NPROCESSORS_ONLN); cpu++) {
      printf(",%llu,%llu,%llu,%llu,%llu,%u",
        perf_events[cpu].cpu_cycles / usecs_elapsed,
        perf_events[cpu].insns / usecs_elapsed,
        (perf_events[cpu].cache_hit - perf_events[cpu].cache_miss) / perf_events[cpu].cache_hit,
        perf_events[cpu].br_miss / perf_events[cpu].br_insns,
        perf_events[cpu].bus_cycles / usecs_elapsed,
        perf_events[cpu].cpu_freq);
    }
    printf(",%lu,%lu",
      io_stats.rd_ios - io_stats_last.rd_ios / usecs_elapsed,
      io_stats.wr_ios - io_stats_last.wr_ios / usecs_elapsed);
    printf("\n");

    // Reset and restart perf counters
    for (int cpu = 0; cpu < sysconf(_SC_NPROCESSORS_ONLN); cpu++) {
      ioctl(perf_events[cpu].fd[0], PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
      ioctl(perf_events[cpu].fd[0], PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);
    }

    // Save previous io_stats value
    io_stats_last = io_stats;

    usleep(10);
  }

  close(i2c);

  for (int i = sysconf(_SC_NPROCESSORS_ONLN); i >= 0; i--)
    for (int it = 0; it < NUM_EVENTS; it++)
      close(perf_events[i].fd[it]);

  return 0;
}