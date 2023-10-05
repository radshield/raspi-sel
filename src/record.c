#include <cpufreq.h>
#include <fcntl.h>
#include <i2c/smbus.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

// INA3221 constants
#define DEVICE_ID 0x40
#define SIGNATURE 8242
#define REG_RESET 0x00
#define REG_DATA_ch1 0x01
#define REG_DATA_ch2 0x03
#define REG_DATA_ch3 0x05

// Perf helpers
#define NUM_EVENTS 7

static volatile bool sentinel = 1;

struct read_format {
    uint64_t nr;
    struct {
        uint64_t value;
        uint64_t id;
    } values[];
};

struct perf_ptr {
  int fd[NUM_EVENTS], cpu_freq;
  uint64_t id[NUM_EVENTS],
           cpu_cycles,
           insns,
           cache_hit,
           cache_miss,
           br_insns,
           br_miss,
           bus_cycles;
};

void int_handler(int signum) {
    sentinel = 0;
}

unsigned int change_endian(unsigned int x) {
  unsigned char *ptr = (unsigned char *)&x;
  return ((ptr[0] << 8) | ptr[1]);
}

float shunt_to_amp(int shunt) {
  // sign change for negative value (bit 13 is sign)
  if (shunt > 4096)
    shunt = -(8192 - shunt);

  // shunt raw value to mv (40μV datasheet)
  float amp1mv = 0.0004 * shunt;

  // without external shunt R on device is 0.1 ohm
  return amp1mv / 0.1;
}

struct perf_ptr init_perf_event(int cpu) {
  struct perf_event_attr pea;
  struct perf_ptr ret;

  // Count perf events
  memset(&pea, 0, sizeof(struct perf_event_attr));
  pea.type = PERF_TYPE_HARDWARE;
  pea.size = sizeof(struct perf_event_attr);
  pea.config = PERF_COUNT_HW_CPU_CYCLES;
  pea.disabled = 1;
  pea.exclude_kernel = 0;
  pea.exclude_hv = 0;
  pea.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID;
  ret.fd[0] = syscall(__NR_perf_event_open, &pea, -1, cpu, -1, 0);
  ioctl(ret.fd[0], PERF_EVENT_IOC_ID, &(ret.id[0]));

  memset(&pea, 0, sizeof(struct perf_event_attr));
  pea.type = PERF_TYPE_HARDWARE;
  pea.size = sizeof(struct perf_event_attr);
  pea.config = PERF_COUNT_HW_INSTRUCTIONS;
  pea.disabled = 0;
  pea.exclude_kernel = 0;
  pea.exclude_hv = 0;
  pea.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID;
  ret.fd[1] = syscall(__NR_perf_event_open, &pea, -1, cpu, ret.fd[0], 0);
  ioctl(ret.fd[1], PERF_EVENT_IOC_ID, &(ret.id[1]));

  memset(&pea, 0, sizeof(struct perf_event_attr));
  pea.type = PERF_TYPE_HARDWARE;
  pea.size = sizeof(struct perf_event_attr);
  pea.config = PERF_COUNT_HW_CACHE_REFERENCES;
  pea.disabled = 0;
  pea.exclude_kernel = 0;
  pea.exclude_hv = 0;
  pea.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID;
  ret.fd[2] = syscall(__NR_perf_event_open, &pea, -1, cpu, ret.fd[0], 0);
  ioctl(ret.fd[2], PERF_EVENT_IOC_ID, &(ret.id[2]));

  memset(&pea, 0, sizeof(struct perf_event_attr));
  pea.type = PERF_TYPE_HARDWARE;
  pea.size = sizeof(struct perf_event_attr);
  pea.config = PERF_COUNT_HW_CACHE_MISSES;
  pea.disabled = 0;
  pea.exclude_kernel = 0;
  pea.exclude_hv = 0;
  pea.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID;
  ret.fd[3] = syscall(__NR_perf_event_open, &pea, -1, cpu, ret.fd[0], 0);
  ioctl(ret.fd[3], PERF_EVENT_IOC_ID, &(ret.id[3]));

  memset(&pea, 0, sizeof(struct perf_event_attr));
  pea.type = PERF_TYPE_HARDWARE;
  pea.size = sizeof(struct perf_event_attr);
  pea.config = PERF_COUNT_HW_BRANCH_INSTRUCTIONS;
  pea.disabled = 0;
  pea.exclude_kernel = 0;
  pea.exclude_hv = 0;
  pea.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID;
  ret.fd[4] = syscall(__NR_perf_event_open, &pea, -1, cpu, ret.fd[0], 0);
  ioctl(ret.fd[4], PERF_EVENT_IOC_ID, &(ret.id[4]));

  memset(&pea, 0, sizeof(struct perf_event_attr));
  pea.type = PERF_TYPE_HARDWARE;
  pea.size = sizeof(struct perf_event_attr);
  pea.config = PERF_COUNT_HW_BRANCH_MISSES;
  pea.disabled = 0;
  pea.exclude_kernel = 0;
  pea.exclude_hv = 0;
  pea.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID;
  ret.fd[5] = syscall(__NR_perf_event_open, &pea, -1, cpu, ret.fd[0], 0);
  ioctl(ret.fd[5], PERF_EVENT_IOC_ID, &(ret.id[5]));

  memset(&pea, 0, sizeof(struct perf_event_attr));
  pea.type = PERF_TYPE_HARDWARE;
  pea.size = sizeof(struct perf_event_attr);
  pea.config = PERF_COUNT_HW_BUS_CYCLES;
  pea.disabled = 0;
  pea.exclude_kernel = 0;
  pea.exclude_hv = 0;
  pea.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID;
  ret.fd[6] = syscall(__NR_perf_event_open, &pea, -1, cpu, ret.fd[0], 0);
  ioctl(ret.fd[6], PERF_EVENT_IOC_ID, &(ret.id[6]));

  return ret;
}

int main(int argc, char **argv) {
  struct perf_ptr perf_events[sysconf(_SC_NPROCESSORS_ONLN)];
  char buf[(NUM_EVENTS * 2 + 1) * sizeof(uint64_t)];
  struct read_format* rf = (struct read_format*) buf;
  struct timespec start, counter;

  if (argc != 2) {
    printf("Usage: %s LOGFILE\n", argv[0]);
    return -1;
  }

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

  printf("I2C setup success\n");

  // Open logfile
  FILE *fd = fopen(argv[1], "w");

  // Set up perf events
  for (int i = 0; i < sysconf(_SC_NPROCESSORS_ONLN); i++) {
    perf_events[i] = init_perf_event(i);
    printf("Init perf on core %d\n", i);
  }

  // Switch device to measurement mode
  i2c_smbus_write_word_data(i2c, REG_RESET, 0b1111111111111111);

  signal(SIGINT, int_handler);

  for (int i = 0; i < sysconf(_SC_NPROCESSORS_ONLN); i++) {
    ioctl(perf_events[0].fd[0], PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
    ioctl(perf_events[0].fd[0], PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);
  }
  printf("Logging start\n");

  clock_gettime(CLOCK_MONOTONIC_RAW, &start);

  while (sentinel) {
    // Read from INA3221
    int shunt1 = i2c_smbus_read_word_data(i2c, REG_DATA_ch1);
    shunt1 = change_endian(shunt1) / 8; // change endian, strip last 3 bits provide raw value
    float ch1_amp = shunt_to_amp(shunt1);

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

    clock_gettime(CLOCK_MONOTONIC_RAW, &counter);

    // Print out current and perf data to file
    fprintf(fd, "%ld,", (counter.tv_sec - start.tv_sec) * 1000000 + (counter.tv_nsec - start.tv_nsec) / 1000);
    fprintf(fd, "%f", ch1_amp);
    for (int cpu = 0; cpu < sysconf(_SC_NPROCESSORS_ONLN); cpu++) {
      fprintf(fd, ",%llu,%llu,%llu,%llu,%llu,%llu,%llu,%u",
        perf_events[cpu].cpu_cycles,
        perf_events[cpu].insns,
        perf_events[cpu].cache_hit,
        perf_events[cpu].cache_miss,
        perf_events[cpu].br_insns,
        perf_events[cpu].br_miss,
        perf_events[cpu].bus_cycles,
        perf_events[cpu].cpu_freq);
    }
    fprintf(fd, "\n");

    // Reset and restart perf counters
    for (int cpu = 0; cpu < sysconf(_SC_NPROCESSORS_ONLN); cpu++) {
      ioctl(perf_events[cpu].fd[0], PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
      ioctl(perf_events[cpu].fd[0], PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);
    }

    usleep(100000);
  }

  close(i2c);
  printf("GPIO close\n");

  for (int i = sysconf(_SC_NPROCESSORS_ONLN); i >= 0; i--)
    for (int it = 0; it < NUM_EVENTS; it++)
      close(perf_events[i].fd[it]);

  printf("Perf events closed\n");

  fclose(fd);
  return 0;
}