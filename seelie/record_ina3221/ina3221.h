#ifndef INA3221_H
#define INA3221_H

#include <tuple>

// INA3221 constants
#define DEVICE_ID 0x41
#define SIGNATURE 8242
#define REG_RESET 0x00
#define REG_DATA_ch1 0x01
#define REG_DATA_ch2 0x03
#define REG_DATA_ch3 0x05

class INA3221 {
private:
  int i2c;

public:
  /**
   * Initialize INA3221 current reader at I2C address 0x41
   */
  INA3221();

  /**
   * Close INA3221 current reader
   */
  ~INA3221();

  /**
   * Read current from initialized INA3221
   * @return a set of tuples for the current on each channel
   */
  std::tuple<double, double, double> read_currents();
};

#endif // INA3221_H
