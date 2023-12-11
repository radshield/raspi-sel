#include "record.h"
#include <cpufreq.h>
#include <i2c/smbus.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/perf_event.h>

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

int read_sysfs_file_stat_work(char *filename, struct io_stats *ios) {
  FILE *fp;
  struct io_stats sdev;
  int i;
  unsigned int ios_pgr, tot_ticks, rq_ticks, wr_ticks, dc_ticks, fl_ticks;
  unsigned long rd_ios, rd_merges_or_rd_sec, wr_ios, wr_merges;
  unsigned long rd_sec_or_wr_ios, wr_sec, rd_ticks_or_wr_sec;
  unsigned long dc_ios, dc_merges, dc_sec, fl_ios;

  // Try to read given stat file
  if ((fp = fopen(filename, "r")) == NULL)
    return -1;

  i = fscanf(fp, "%lu %lu %lu %lu %lu %lu %lu %u %u %u %u %lu %lu %lu %u %lu %u",
       &rd_ios, &rd_merges_or_rd_sec, &rd_sec_or_wr_ios, &rd_ticks_or_wr_sec,
       &wr_ios, &wr_merges, &wr_sec, &wr_ticks, &ios_pgr, &tot_ticks, &rq_ticks,
       &dc_ios, &dc_merges, &dc_sec, &dc_ticks,
       &fl_ios, &fl_ticks);

  memset(&sdev, 0, sizeof(struct io_stats));

  if (i >= 11) {
    // Device or partition
    sdev.rd_ios     = rd_ios;
    sdev.rd_merges  = rd_merges_or_rd_sec;
    sdev.rd_sectors = rd_sec_or_wr_ios;
    sdev.rd_ticks   = (unsigned int) rd_ticks_or_wr_sec;
    sdev.wr_ios     = wr_ios;
    sdev.wr_merges  = wr_merges;
    sdev.wr_sectors = wr_sec;
    sdev.wr_ticks   = wr_ticks;
    sdev.ios_pgr    = ios_pgr;
    sdev.tot_ticks  = tot_ticks;
    sdev.rq_ticks   = rq_ticks;

    if (i >= 15) {
      // Discard I/O
      sdev.dc_ios     = dc_ios;
      sdev.dc_merges  = dc_merges;
      sdev.dc_sectors = dc_sec;
      sdev.dc_ticks   = dc_ticks;
    }

    if (i >= 17) {
      // Flush I/O
      sdev.fl_ios     = fl_ios;
      sdev.fl_ticks   = fl_ticks;
    }
  }
  else if (i == 4) {
    // Partition without extended statistics
    sdev.rd_ios     = rd_ios;
    sdev.rd_sectors = rd_merges_or_rd_sec;
    sdev.wr_ios     = rd_sec_or_wr_ios;
    sdev.wr_sectors = rd_ticks_or_wr_sec;
  }
  
  *ios = sdev;

  fclose(fp);

  return 0;
}