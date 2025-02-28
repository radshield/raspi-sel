#include "record_system.h"

#include <cerrno>
#include <cstring>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include <linux/perf_event.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <sys/time.h>

perf_ptr RecordSystem::init_perf_event(int cpu) {
  struct perf_event_attr pea = {};
  perf_ptr ret;

  // Count perf events
  pea.type = PERF_TYPE_HARDWARE;
  pea.size = sizeof(pea);
  pea.config = PERF_COUNT_HW_CPU_CYCLES;
  pea.disabled = 1;
  pea.exclude_kernel = 0;
  pea.exclude_hv = 0;
  pea.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID;

  ret.fd[0] = syscall(__NR_perf_event_open, &pea, -1, cpu, -1, 0);
  if (ret.fd[0] == -1 && errno != 0)
    throw std::runtime_error(std::string("Failed to init perf events, errno=") +
                             std::strerror(errno));

  ioctl(ret.fd[0], PERF_EVENT_IOC_ID, &(ret.id[0]));
  if (ret.id[0] == 0 && errno != 0)
    throw std::runtime_error(std::string("Failed to init perf events, errno=") +
                             std::strerror(errno));

  pea = {};
  pea.type = PERF_TYPE_HARDWARE;
  pea.size = sizeof(pea);
  pea.config = PERF_COUNT_HW_INSTRUCTIONS;
  pea.disabled = 0;
  pea.exclude_kernel = 0;
  pea.exclude_hv = 0;
  pea.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID;

  ret.fd[1] = syscall(__NR_perf_event_open, &pea, -1, cpu, ret.fd[0], 0);
  if (ret.fd[1] == -1 && errno != 0)
    throw std::runtime_error(std::string("Failed to init perf events, errno=") +
                             std::strerror(errno));

  ioctl(ret.fd[1], PERF_EVENT_IOC_ID, &(ret.id[1]));
  if (ret.id[1] == 0 && errno != 0)
    throw std::runtime_error(std::string("Failed to init perf events, errno=") +
                             std::strerror(errno));

  pea = {};
  pea.type = PERF_TYPE_HARDWARE;
  pea.size = sizeof(pea);
  pea.config = PERF_COUNT_HW_CACHE_REFERENCES;
  pea.disabled = 0;
  pea.exclude_kernel = 0;
  pea.exclude_hv = 0;
  pea.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID;

  ret.fd[2] = syscall(__NR_perf_event_open, &pea, -1, cpu, ret.fd[0], 0);
  if (ret.fd[2] == -1 && errno != 0)
    throw std::runtime_error(std::string("Failed to init perf events, errno=") +
                             std::strerror(errno));

  ioctl(ret.fd[2], PERF_EVENT_IOC_ID, &(ret.id[2]));
  if (ret.id[2] == 0 && errno != 0)
    throw std::runtime_error(std::string("Failed to init perf events, errno=") +
                             std::strerror(errno));

  pea = {};
  pea.type = PERF_TYPE_HARDWARE;
  pea.size = sizeof(pea);
  pea.config = PERF_COUNT_HW_CACHE_MISSES;
  pea.disabled = 0;
  pea.exclude_kernel = 0;
  pea.exclude_hv = 0;
  pea.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID;

  ret.fd[3] = syscall(__NR_perf_event_open, &pea, -1, cpu, ret.fd[0], 0);
  if (ret.fd[3] == -1 && errno != 0)
    throw std::runtime_error(std::string("Failed to init perf events, errno=") +
                             std::strerror(errno));

  ioctl(ret.fd[3], PERF_EVENT_IOC_ID, &(ret.id[3]));
  if (ret.id[3] == 0 && errno != 0)
    throw std::runtime_error(std::string("Failed to init perf events, errno=") +
                             std::strerror(errno));

  pea = {};
  pea.type = PERF_TYPE_HARDWARE;
  pea.size = sizeof(pea);
  pea.config = PERF_COUNT_HW_BRANCH_INSTRUCTIONS;
  pea.disabled = 0;
  pea.exclude_kernel = 0;
  pea.exclude_hv = 0;
  pea.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID;

  ret.fd[4] = syscall(__NR_perf_event_open, &pea, -1, cpu, ret.fd[0], 0);
  if (ret.fd[4] == -1 && errno != 0)
    throw std::runtime_error(std::string("Failed to init perf events, errno=") +
                             std::strerror(errno));

  ioctl(ret.fd[4], PERF_EVENT_IOC_ID, &(ret.id[4]));
  if (ret.id[4] == 0 && errno != 0)
    throw std::runtime_error(std::string("Failed to init perf events, errno=") +
                             std::strerror(errno));

  pea = {};
  pea.size = sizeof(pea);
  pea.config = PERF_COUNT_HW_BRANCH_MISSES;
  pea.disabled = 0;
  pea.exclude_kernel = 0;
  pea.exclude_hv = 0;
  pea.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID;

  ret.fd[5] = syscall(__NR_perf_event_open, &pea, -1, cpu, ret.fd[0], 0);
  if (ret.fd[5] == -1 && errno != 0)
    throw std::runtime_error(std::string("Failed to init perf events, errno=") +
                             std::strerror(errno));

  ioctl(ret.fd[5], PERF_EVENT_IOC_ID, &(ret.id[5]));
  if (ret.id[5] == 0 && errno != 0)
    throw std::runtime_error(std::string("Failed to init perf events, errno=") +
                             std::strerror(errno));

  pea = {};
  pea.type = PERF_TYPE_HARDWARE;
  pea.size = sizeof(pea);
  pea.config = PERF_COUNT_HW_BUS_CYCLES;
  pea.disabled = 0;
  pea.exclude_kernel = 0;
  pea.exclude_hv = 0;
  pea.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID;

  ret.fd[6] = syscall(__NR_perf_event_open, &pea, -1, cpu, ret.fd[0], 0);
  if (ret.fd[6] == -1 && errno != 0)
    throw std::runtime_error(std::string("Failed to init perf events, errno=") +
                             std::strerror(errno));

  ioctl(ret.fd[6], PERF_EVENT_IOC_ID, &(ret.id[6]));
  if (ret.id[6] == 0 && errno != 0)
    throw std::runtime_error(std::string("Failed to init perf events, errno=") +
                             std::strerror(errno));

  return ret;
}

void RecordSystem::read_sysfs_file_stat_work(std::string filename) {
  FILE *fp;
  int i;
  unsigned int ios_pgr, tot_ticks, rq_ticks, wr_ticks, dc_ticks, fl_ticks;
  unsigned long rd_ios, rd_merges_or_rd_sec, wr_ios, wr_merges;
  unsigned long rd_sec_or_wr_ios, wr_sec, rd_ticks_or_wr_sec;
  unsigned long dc_ios, dc_merges, dc_sec, fl_ios;

  // Try to read given stat file
  if ((fp = fopen(filename.c_str(), "r")) == NULL)
    throw std::runtime_error("Failed to open disk stat file");

  i = fscanf(
      fp, "%lu %lu %lu %lu %lu %lu %lu %u %u %u %u %lu %lu %lu %u %lu %u",
      &rd_ios, &rd_merges_or_rd_sec, &rd_sec_or_wr_ios, &rd_ticks_or_wr_sec,
      &wr_ios, &wr_merges, &wr_sec, &wr_ticks, &ios_pgr, &tot_ticks, &rq_ticks,
      &dc_ios, &dc_merges, &dc_sec, &dc_ticks, &fl_ios, &fl_ticks);

  io_stats_curr = {};

  if (i >= 11) {
    // Device or partition
    io_stats_curr.rd_ios = rd_ios;
    io_stats_curr.rd_merges = rd_merges_or_rd_sec;
    io_stats_curr.rd_sectors = rd_sec_or_wr_ios;
    io_stats_curr.rd_ticks = (unsigned int)rd_ticks_or_wr_sec;
    io_stats_curr.wr_ios = wr_ios;
    io_stats_curr.wr_merges = wr_merges;
    io_stats_curr.wr_sectors = wr_sec;
    io_stats_curr.wr_ticks = wr_ticks;
    io_stats_curr.ios_pgr = ios_pgr;
    io_stats_curr.tot_ticks = tot_ticks;
    io_stats_curr.rq_ticks = rq_ticks;

    if (i >= 15) {
      // Discard I/O
      io_stats_curr.dc_ios = dc_ios;
      io_stats_curr.dc_merges = dc_merges;
      io_stats_curr.dc_sectors = dc_sec;
      io_stats_curr.dc_ticks = dc_ticks;
    }

    if (i >= 17) {
      // Flush I/O
      io_stats_curr.fl_ios = fl_ios;
      io_stats_curr.fl_ticks = fl_ticks;
    }
  } else if (i == 4) {
    // Partition without extended statistics
    io_stats_curr.rd_ios = rd_ios;
    io_stats_curr.rd_sectors = rd_merges_or_rd_sec;
    io_stats_curr.wr_ios = rd_sec_or_wr_ios;
    io_stats_curr.wr_sectors = rd_ticks_or_wr_sec;
  }

  fclose(fp);
}

void RecordSystem::read_cpu_freq() {
  FILE *fp;

  if ((fp = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq",
                  "r")) == NULL)
    throw std::runtime_error("Failed to open disk stat file");
  fscanf(fp, "%u", &perf_events[0].cpu_freq);
  fclose(fp);

  if ((fp = fopen("/sys/devices/system/cpu/cpu1/cpufreq/scaling_cur_freq",
                  "r")) == NULL)
    throw std::runtime_error("Failed to open disk stat file");
  fscanf(fp, "%u", &perf_events[1].cpu_freq);
  fclose(fp);

  if ((fp = fopen("/sys/devices/system/cpu/cpu2/cpufreq/scaling_cur_freq",
                  "r")) == NULL)
    throw std::runtime_error("Failed to open disk stat file");
  fscanf(fp, "%u", &perf_events[2].cpu_freq);
  fclose(fp);

  if ((fp = fopen("/sys/devices/system/cpu/cpu3/cpufreq/scaling_cur_freq",
                  "r")) == NULL)
    throw std::runtime_error("Failed to open disk stat file");
  fscanf(fp, "%u", &perf_events[3].cpu_freq);
  fclose(fp);
}

RecordSystem::RecordSystem() {
  // Set up perf events
  for (int i = 0; i < NUM_CPUS; i++)
    perf_events[i] = init_perf_event(i);

  // Initialize time and disk info
  clock_gettime(CLOCK_MONOTONIC_RAW, &time_last);
  this->read_sysfs_file_stat_work("/sys/block/mmcblk0/stat");
  this->io_stats_last = this->io_stats_curr;
}

// Close perf events
RecordSystem::~RecordSystem() {
  for (int i = NUM_CPUS; i >= 0; i--)
    for (int it = 0; it < NUM_EVENTS; it++)
      close(perf_events[i].fd[it]);
}

perf_data RecordSystem::get_system_info() {
  int usecs_elapsed;
  char buf[(NUM_EVENTS * 2 + 1) * sizeof(uint64_t)];
  struct read_format {
    uint64_t nr;
    struct {
      uint64_t value;
      uint64_t id;
    } values[];
  } *rf = (read_format *)buf;
  perf_data ret;

  // Read from perf counters
  for (int cpu = 0; cpu < NUM_CPUS; cpu++)
    ioctl(perf_events[cpu].fd[0], PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP);
  read_cpu_freq();

  // Populate into perf_events fields
  for (int cpu = 0; cpu < NUM_CPUS; cpu++) {
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
  read_sysfs_file_stat_work("/sys/block/mmcblk0/stat");

  // Get latest time and calculate measurement interval
  clock_gettime(CLOCK_MONOTONIC_RAW, &time_now);
  usecs_elapsed = (time_now.tv_sec - time_last.tv_sec) * 1000000 +
                  (time_now.tv_nsec - time_last.tv_nsec) / 1000;

  // Output rate-based events
  for (int cpu = 0; cpu < NUM_CPUS; cpu++) {
    ret.cpu_cycles[cpu] = (double)perf_events[cpu].cpu_cycles / usecs_elapsed;
    ret.insn_rate[cpu] = (double)perf_events[cpu].insns / usecs_elapsed;
    ret.cache_hit_rate[cpu] =
        (double)(perf_events[cpu].cache_hit - perf_events[cpu].cache_miss) /
        perf_events[cpu].cache_hit;
    ret.br_miss_rate[cpu] =
        (double)perf_events[cpu].br_miss / perf_events[cpu].br_insns;
    ret.bus_cycles[cpu] = (double)perf_events[cpu].bus_cycles / usecs_elapsed;
    ret.cpu_freq[cpu] = (double)perf_events[cpu].cpu_freq;
  }

  // Output disk read/write rates
  ret.rd_ios =
      (double)(io_stats_curr.rd_ios - io_stats_last.rd_ios) / usecs_elapsed;
  ret.wr_ios =
      (double)(io_stats_curr.wr_ios - io_stats_last.wr_ios) / usecs_elapsed;

  // Reset and restart perf counters
  for (int cpu = 0; cpu < NUM_CPUS; cpu++) {
    ioctl(perf_events[cpu].fd[0], PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
    ioctl(perf_events[cpu].fd[0], PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);
  }

  // Save previous io_stats and time values
  io_stats_last = io_stats_curr;
  time_last = time_now;

  return ret;
}
