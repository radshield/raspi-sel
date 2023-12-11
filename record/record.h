#ifndef RECORD_H
#define RECORD_H

#include <cstdint>

// INA3221 constants
#define DEVICE_ID 0x40
#define SIGNATURE 8242
#define REG_RESET 0x00
#define REG_DATA_ch1 0x01
#define REG_DATA_ch2 0x03
#define REG_DATA_ch3 0x05

// Perf helpers
#define NUM_EVENTS 7

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


#endif