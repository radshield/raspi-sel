#ifndef RECORD_SYSTEM_H
#define RECORD_SYSTEM_H

#include <cstdint>
#include <ctime>
#include <string>
#include <unistd.h>

// Perf helpers
#define NUM_EVENTS 7
#define NUM_CPUS 4

struct perf_ptr {
  int fd[NUM_EVENTS], cpu_freq;
  uint64_t id[NUM_EVENTS], cpu_cycles, insns, cache_hit, cache_miss, br_insns,
      br_miss, bus_cycles;
};

struct io_stats {
  unsigned long rd_sectors __attribute__((aligned(8)));
  unsigned long wr_sectors __attribute__((packed));
  unsigned long dc_sectors __attribute__((packed));
  unsigned long rd_ios __attribute__((packed));
  unsigned long rd_merges __attribute__((packed));
  unsigned long wr_ios __attribute__((packed));
  unsigned long wr_merges __attribute__((packed));
  unsigned long dc_ios __attribute__((packed));
  unsigned long dc_merges __attribute__((packed));
  unsigned long fl_ios __attribute__((packed));
  unsigned int rd_ticks __attribute__((packed));
  unsigned int wr_ticks __attribute__((packed));
  unsigned int dc_ticks __attribute__((packed));
  unsigned int fl_ticks __attribute__((packed));
  unsigned int ios_pgr __attribute__((packed));
  unsigned int tot_ticks __attribute__((packed));
  unsigned int rq_ticks __attribute__((packed));
};

struct perf_data {
  double rd_ios, wr_ios, cpu_freq[4], cpu_cycles[4], insn_rate[4],
      cache_hit_rate[4], br_miss_rate[4], bus_cycles[4];
};

class RecordSystem {
private:
  perf_ptr perf_events[NUM_CPUS];
  io_stats io_stats_last, io_stats_curr;
  timespec time_now, time_last;

  /**
   * Create perf file descriptors for selected CPU
   * @param cpu CPU number
   * @return perf_ptr struct containing perf file descriptors
   */
  perf_ptr init_perf_event(int cpu);

  /**
   * Read I/O stats from /sys/block/___/stat
   * @param filename target stat file to read from
   */
  void read_sysfs_file_stat_work(std::string filename);

  /**
   * Read CPU frequency from /sys/devices/system/cpu/cpu*
   */
  void read_cpu_freq();

public:
  /**
   * Initialize perf counters and other file descriptors for system stats
   */
  RecordSystem();

  /**
   * Close perf counters and other file descriptors
   */
  ~RecordSystem();

  /**
   * Read system metrics using perf to struct
   * @return system info as a perf_data struct
   */
  perf_data get_system_info();
};

#endif // RECORD_H
