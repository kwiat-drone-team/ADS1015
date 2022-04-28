#pragma once
#include <mutex>
#include <string>

static const char *I2C_BUS = "/dev/i2c-1";
constexpr float gain_voltage = 2.048;
constexpr size_t sleep_microseconds = 80;

constexpr size_t DARK_COUNT_SECONDS = 20;
constexpr size_t MICROSECONDS_IN_SECOND = 1000000;

constexpr size_t ADC_RESOLUTION = 32767;

class PSDInterface
{
public:
  double PSD_x, PSD_y, PSD_sum_x, PSD_sum_y =0;
  double normX, normY=0;
  PSDInterface()
  {
    setup_ADC();
  }
  // Config Register (Byte1)  Bit#  15   14  13  12  11  10  9     8
  //                                OS |  MUX[2:0]  | PGA[2:0]  | MODE
  // Config Register (Byte2)  Bit#  7  6  5       4           3           2            1  0
  //                                DR[2:0]  | COMP_Mode | COMP_Pol | COMP_Lat |   COMP_QUE[1:0]
  // Gain Settings:
  //  000   +/- 6.144
  //  001   +/- 4.096
  //  010   +/- 2.048
  //  011   +/- 1.024
  //  100   +/- 0.512
  //  101   +/- 0.256

  uint8_t byte2 = 0xC3;
  uint8_t byte1 = 0x04;

  // Save file logging
  bool save_to_file = true;
  std::mutex PSD_val_mtx;
  void start_PSD_loop(void);
  void start_dark_count(size_t seconds);
  void setup_ADC(void);

private:
  float PSD_y_offset, PSD_x_offset, PSD_sum_y_offset, PSD_sum_x_offset = 0;

  int ADS0_address = 0x48;
  int ADS1_address = 0x49;
  int ADS2_address = 0x4a;
  int ADS3_address = 0x4b;

  // 2 byte buffer to store the data read from the I2C device
  uint8_t readBuf0[2];
  uint8_t readBuf1[2];
  uint8_t readBuf2[2];
  uint8_t readBuf3[2];

  uint8_t writeBuf[3]; // Buffer to store the 3 bytes that we write to the I2C device

  int I2CFile0;
  int I2CFile1;
  int I2CFile2;
  int I2CFile3;
};
