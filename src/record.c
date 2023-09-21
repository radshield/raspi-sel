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
  float amp1mv = (163.8 / 4096) * shunt;

  // without external shunt R on device is 0.1 ohm
  return amp1mv / 0.1;
}

int main(int argc, char **argv) {
  // Setup I2C communication
  gpioInitialise();
  int fd = i2cOpen(1, DEVICE_ID, 0);
  if (fd == -1) {
    printf("Failed to init I2C communication.\n");
    return -1;
  }
  int check_vendor_id = i2cReadWordData(fd, 0xFF);
  if (check_vendor_id == SIGNATURE) {
    printf("I2C communication successfully setup with INA3221 device at addess "
           "0x%x.\n",
           DEVICE_ID);
  } else {
    printf("Device at address 0x%x is not an INA3221 device; exiting\n",
           DEVICE_ID);
    return -1;
  }

  // Switch device to measurement mode (reset when connect,continous mode, max
  // average) to modify this in next version
  i2cWriteWordData(fd, REG_RESET, 0b1111111111111111);

  while (1) {
    int shunt1 = i2cReadWordData(fd, REG_DATA_ch1);
    // change endian, strip last 3 bits provide raw value
    shunt1 = change_endian(shunt1) / 8;
    float ch1_amp = shunt_to_amp(shunt1);

    printf("ch1 raw:%d , ch1 A:%f\n", shunt1, ch1_amp);
    sleep(1);
  }

  i2cClose(fd);
  gpioTerminate();
  return 0;
}