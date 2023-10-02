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

enum perf_event_count {
  CPU_CYCLES,
  INSNS,
  CACHE_HIT,
  CACHE_MISS,
  BR_INSNS,
  BR_MISS,
  BUS_CYCLES
};

static volatile bool sentinel = 1;

struct read_format {
    uint64_t nr;
    struct {
        uint64_t value;
        uint64_t id;
    } values[];
};

struct perf_ptr {
  int fd;
  uint64_t id, count;
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

struct perf_ptr init_perf_event(int cpu, int root_fd, enum perf_event_count perf_event) {
  struct perf_event_attr pea;
  struct perf_ptr ret;
  enum perf_type_id perf_type;
  int perf_event_config;

  if (perf_event <= BUS_CYCLES)
    perf_type = PERF_TYPE_HARDWARE;
  else
    perf_type = PERF_TYPE_HW_CACHE;

  switch (perf_event) {
  case CPU_CYCLES:
    perf_event_config = PERF_COUNT_HW_CPU_CYCLES;
  case INSNS:
    perf_event_config = PERF_COUNT_HW_INSTRUCTIONS;
  case CACHE_HIT:
    perf_event_config = PERF_COUNT_HW_CACHE_REFERENCES;
  case CACHE_MISS:
    perf_event_config = PERF_COUNT_HW_CACHE_MISSES;
  case BR_INSNS:
    perf_event_config = PERF_COUNT_HW_BRANCH_INSTRUCTIONS;
  case BR_MISS:
    perf_event_config = PERF_COUNT_HW_BRANCH_MISSES;
  case BUS_CYCLES:
    perf_event_config = PERF_COUNT_HW_BUS_CYCLES;
  }

  // Count perf events
  memset(&pea, 0, sizeof(struct perf_event_attr));
  pea.type = PERF_TYPE_HARDWARE;
  pea.size = sizeof(struct perf_event_attr);
  pea.config = perf_event_config;
  pea.disabled = 1;
  pea.exclude_kernel = 0;
  pea.exclude_hv = 0;
  pea.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID;
  ret.fd = syscall(__NR_perf_event_open, &pea, -1, cpu, root_fd, 0);
  ioctl(ret.fd, PERF_EVENT_IOC_ID, &(ret.id));

  return ret;
}

int main(int argc, char **argv) {
  struct perf_ptr perf_events[sysconf(_SC_NPROCESSORS_ONLN)][NUM_EVENTS];
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

  // Make sure vendor ID matches INA3221
  int check_vendor_id = i2c_smbus_read_word_data(i2c, 0xFF);
  if (check_vendor_id != SIGNATURE)
    return -3;

  printf("I2C setup success\n");

  // Open logfile
  FILE *fd = fopen(argv[1], "w");

  // Set up perf events
  for (int cpu = 0; cpu < sysconf(_SC_NPROCESSORS_ONLN); cpu++) {
    // Set first fd to -1 to make groupless
    perf_events[cpu][0].fd = -1;

    // Init counters for each event
    for (int event = 0; event < NUM_EVENTS; event++) {
      init_perf_event(cpu, perf_events[cpu][0].fd, event);
    }

    printf("Init perf on core %d\n", cpu);
  }

  // Switch device to measurement mode
  i2c_smbus_write_word_data(i2c, REG_RESET, 0b1111111111111111);

  signal(SIGINT, int_handler);

  for (int cpu = 0; cpu < sysconf(_SC_NPROCESSORS_ONLN); cpu++) {
    ioctl(perf_events[cpu][0].fd, PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
    ioctl(perf_events[cpu][0].fd, PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);
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
      ioctl(perf_events[cpu][0].fd, PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP);

    for (int cpu = 0; cpu < sysconf(_SC_NPROCESSORS_ONLN); cpu++) {
      // Read event
      read(perf_events[cpu][0].fd, buf, sizeof(buf));

      // Go through all events
      for (int it = 0; it < rf->nr; it++) {
        // Go through and match against correct perf counter to increment
        for (int event = 0; event < NUM_EVENTS; event++) {
          if (rf->values[it].id == perf_events[cpu][event].id)
            perf_events[cpu][event].count = rf->values[it].value;
        }
      }
    }

    clock_gettime(CLOCK_MONOTONIC_RAW, &counter);

    // Print out current and perf data to file
    fprintf(fd, "%ld,", (counter.tv_sec - start.tv_sec) * 1000000 + (counter.tv_nsec - start.tv_nsec) / 1000);
    fprintf(fd, "%f", ch1_amp);
    for (int cpu = 0; cpu < sysconf(_SC_NPROCESSORS_ONLN); cpu++) {
      for (int event = 0; event < NUM_EVENTS; event++) {
        fprintf(fd, ",%llu", perf_events[cpu][event].count);
      }
    }
    fprintf(fd, "\n");

    // Reset and restart perf counters
    for (int cpu = 0; cpu < sysconf(_SC_NPROCESSORS_ONLN); cpu++) {
      ioctl(perf_events[cpu][0].fd, PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
      ioctl(perf_events[cpu][0].fd, PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);
    }

    usleep(100000);
  }

  close(i2c);
  printf("GPIO close\n");

  for (int cpu = sysconf(_SC_NPROCESSORS_ONLN); cpu >= 0; cpu--) {
    for (int event = 0; event < NUM_EVENTS; event++) {
      close(perf_events[cpu][event].fd);
    }
  }

  printf("Perf events closed\n");

  fclose(fd);
  return 0;
}
