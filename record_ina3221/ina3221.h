#ifndef INA3221_H
#define INA3221_H

#include <string>

// INA3221 constants
#define DEVICE_ID 0x40
#define SIGNATURE 8242
#define REG_RESET 0x00
#define REG_DATA_ch1 0x01
#define REG_DATA_ch2 0x03
#define REG_DATA_ch3 0x05

class INA3221 {
private:
	int i2c;
public:
	INA3221();
	~INA3221();
	std::tuple<double, double, double> read_currents();
};

#endif // INA3221_H
