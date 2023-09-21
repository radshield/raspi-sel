#include <stdio.h>
#include <unistd.h>
#include <pigpio.h>
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>
#include <sys/syscall.h>

// INA3221 constants
#define DEVICE_ID 0x40
#define SIGNATURE 8242
#define REG_RESET 0x00
#define REG_DATA_ch1 0x01
#define REG_DATA_ch2 0x03
#define REG_DATA_ch3 0x05

unsigned int change_endian(unsigned int x) {
  unsigned char *ptr = (unsigned char *)&x;
  return ((ptr[0] << 8) | ptr[1]);
}

float shunt_to_amp(int shunt) {
  // sign change for negative value (bit 13 is sign)
  if (shunt > 4096) {
    shunt = -(8192 - shunt);
  }

  // shunt raw value to mv (163.8mV LSB (SD0):40μV) datasheet
  float amp1mv = 0.0004 * shunt;

  // without external shunt R on device is 0.1 ohm
  return amp1mv / 0.1;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: %s LOGFILE", argv[0]);
    return -1;
  }

  // Setup I2C communication
  gpioInitialise();

  int i2c = i2cOpen(1, DEVICE_ID, 0);
  if (i2c == -1)
    return -2;

  int check_vendor_id = i2cReadWordData(i2c, 0xFF);
  if (check_vendor_id != SIGNATURE)
    return -3;

  FILE *fd = fopen(argv[1], "w");
  fprintf(fd, "current\n");

  // Switch device to measurement mode
  i2cWriteWordData(i2c, REG_RESET, 0b1111111111111111);

  while (1) {
    int shunt1 = i2cReadWordData(i2c, REG_DATA_ch1);
    shunt1 = change_endian(shunt1) / 8; // change endian, strip last 3 bits provide raw value
    float ch1_amp = shunt_to_amp(shunt1);

    fprintf(fd, "%f\n", ch1_amp);
    sleep(1);
  }

  fclose(fd);
  i2cClose(i2c);
  gpioTerminate();
  return 0;
}