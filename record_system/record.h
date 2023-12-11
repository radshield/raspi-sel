#ifndef RECORD_H
#define RECORD_H

#include <cstdint>
#include <ctime>
#include <string>
#include <unistd.h>

// Perf helpers
#define NUM_EVENTS 7
#define NUM_CPUS 4

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

struct io_stats {
  unsigned long rd_sectors __attribute__ ((aligned (8)));
  unsigned long wr_sectors __attribute__ ((packed));
  unsigned long dc_sectors __attribute__ ((packed));
  unsigned long rd_ios     __attribute__ ((packed));
  unsigned long rd_merges  __attribute__ ((packed));
  unsigned long wr_ios     __attribute__ ((packed));
  unsigned long wr_merges  __attribute__ ((packed));
  unsigned long dc_ios     __attribute__ ((packed));
  unsigned long dc_merges  __attribute__ ((packed));
  unsigned long fl_ios     __attribute__ ((packed));
  unsigned int  rd_ticks   __attribute__ ((packed));
  unsigned int  wr_ticks   __attribute__ ((packed));
  unsigned int  dc_ticks   __attribute__ ((packed));
  unsigned int  fl_ticks   __attribute__ ((packed));
  unsigned int  ios_pgr    __attribute__ ((packed));
  unsigned int  tot_ticks  __attribute__ ((packed));
  unsigned int  rq_ticks   __attribute__ ((packed));
};

class RecordSystem {
private:
  perf_ptr perf_events[NUM_CPUS];
  io_stats io_stats_last, io_stats_curr;
  char buf[(NUM_EVENTS * 2 + 1) * sizeof(uint64_t)];
  read_format* rf = (read_format*) buf;
  timespec time_now, time_last;

  perf_ptr init_perf_event(int cpu);
  int read_sysfs_file_stat_work(char *filename, struct io_stats *ios);
public:
  RecordSystem();
  std::string get_system_info();
  ~RecordSystem();
};

#endif // RECORD_H
