#include <stdio.h>
#include <fcntl.h>         // open
#include <inttypes.h>      // uint8_t, etc
#include <linux/i2c-dev.h> // I2C bus definitions
#include <sys/ioctl.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "PSDInterface.hpp"

void PSDInterface::setup_ADC()
{

  // Load offsets
  std::ifstream fin;
  fin.open("config.txt", std::ios::in);
  if(!fin){
    std::cout << "Error opening configuration file in PSDInterface setup!" << std::endl;
    exit(1);
  }
  std::string config;
  std::getline(fin, config); // Skip header
  std::getline(fin,config,',');
  PSD_y_offset = std::stod(config);
  std::getline(fin,config,',');
  PSD_x_offset = std::stod(config);
  std::getline(fin,config,',');
  PSD_sum_y_offset = std::stod(config);
  std::getline(fin,config,',');
  PSD_sum_x_offset = std::stod(config);
  fin.close();

  std::cout << PSD_y_offset << std::endl;
  
  I2CFile0 = open(I2C_BUS, O_RDWR); // Open the I2C device
  if (I2CFile0 < 0)
  {
    std::cout << "Error opening i2c bus: 0" << std::endl;
  }
  I2CFile1 = open(I2C_BUS, O_RDWR); // Open the I2C device
  if (I2CFile1 < 0)
  {
    std::cout << "Error opening i2c bus: 1" << std::endl;
  }
  I2CFile2 = open(I2C_BUS, O_RDWR); // Open the I2C device
  if (I2CFile2 < 0)
  {
    std::cout << "Error opening i2c bus: 2" << std::endl;
  }
  I2CFile3 = open(I2C_BUS, O_RDWR); // Open the I2C device
  if (I2CFile3 < 0)
  {
    std::cout << "Error opening i2c bus: 3" << std::endl;
  }

  ioctl(I2CFile0, I2C_SLAVE, ADS0_address); // Specify the address of the I2C Slave to communicate with
  ioctl(I2CFile1, I2C_SLAVE, ADS1_address); // Specify the address of the I2C Slave to communicate with
  ioctl(I2CFile2, I2C_SLAVE, ADS2_address); // Specify the address of the I2C Slave to communicate with
  ioctl(I2CFile3, I2C_SLAVE, ADS3_address); // Specify the address of the I2C Slave to communicate with

  //  Datarate, set bits [7:5] in writeBuf[2]
  //  000 : 128SPS
  //  001 : 250SPS
  //  010 : 490SPS
  //  011 : 920SPS
  //  100 : 1600SPS (default)
  //  101 : 2400SPS
  //  110 : 3300SPS
  //  111 : 3300SPS

  // writeBuf[2] = 0x03;       // This sets the 8 LSBs of the config register (bits 7-0) to 00000011
  writeBuf[2] = byte2;
  writeBuf[1] = byte1;
  writeBuf[0] = 1; // This sets the pointer register so that the following two bytes write to the config register

  // Initialize the buffer used to read data from the ADS1115 to 0
  readBuf0[0] = 0;
  readBuf0[1] = 0;

  // Initialize the buffer used to read data from the ADS1115 to 0
  readBuf1[0] = 0;
  readBuf1[1] = 0;

  // Initialize the buffer used to read data from the ADS1115 to 0
  readBuf2[0] = 0;
  readBuf2[1] = 0;

  // Initialize the buffer used to read data from the ADS1115 to 0
  readBuf3[0] = 0;
  readBuf3[1] = 0;

  // Write writeBuf to the ADS1115, the 3 specifies the number of bytes we are writing,
  // this begins a single conversion
  // ioctl(I2CFile0, I2C_SLAVE, ADS0_address);   // Specify the address of the I2C Slave to communicate with
  write(I2CFile0, writeBuf, 3);
  write(I2CFile1, writeBuf, 3);
  write(I2CFile2, writeBuf, 3);
  write(I2CFile3, writeBuf, 3);

  writeBuf[0] = 0; // Set pointer register to 0 to read from the conversion register
  write(I2CFile0, writeBuf, 1);
  write(I2CFile1, writeBuf, 1);
  write(I2CFile2, writeBuf, 1);
  write(I2CFile3, writeBuf, 1);
}

void PSDInterface::start_dark_count(size_t seconds)
{

  // Save the piezo x and y offsets
  std::ifstream fin;
  fin.open("config.txt", std::ios::in);
  std::string config;
  std::getline(fin, config); // Skip header
  for (int i = 0; i < 5; i++)
  {
    std::getline(fin, config, ',');
  }
  size_t piezo_x_offset = std::stoi(config);
  std::getline(fin, config, ',');
  size_t piezo_y_offset = std::stoi(config);
  fin.close();

  double PSD_y_offset, PSD_x_offset, PSD_sum_y_offset, PSD_sum_x_offset = 0;
  double volts0, volts1, volts2, volts3;
  int16_t val0, val1, val2, val3; // Stores the 16 bit value of our ADC conversion

  auto begin = std::chrono::high_resolution_clock::now();
  size_t num_iterations = 0;
  auto taken = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - begin).count();
  while (taken < seconds * MICROSECONDS_IN_SECOND)
  {
    taken = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - begin).count();
    read(I2CFile0, readBuf0, 2); // Read the contents of the conversion register into readBuf
    read(I2CFile1, readBuf1, 2); // Read the contents of the conversion register into readBuf
    read(I2CFile2, readBuf2, 2); // Read the contents of the conversion register into readBuf
    read(I2CFile3, readBuf3, 2); // Read the contents of the conversion register into readBuf

    val0 = readBuf0[0] << 8 | readBuf0[1]; // Combine the two bytes of readBuf into a single 16 bit result
    val1 = readBuf1[0] << 8 | readBuf1[1]; // Combine the two bytes of readBuf into a single 16 bit result
    val2 = readBuf2[0] << 8 | readBuf2[1]; // Combine the two bytes of readBuf into a single 16 bit result
    val3 = readBuf3[0] << 8 | readBuf3[1]; // Combine the two bytes of readBuf into a single 16 bit result

    volts0 = (float)val0 * gain_voltage / 32767.0;
    volts1 = (float)val1 * gain_voltage / 32767.0;
    volts2 = (float)val2 * gain_voltage / 32767.0;
    volts3 = (float)val3 * gain_voltage / 32767.0;

    PSD_y_offset += volts0;
    PSD_x_offset += volts1;
    PSD_sum_y_offset += volts2;
    PSD_sum_x_offset += volts3;

    usleep(sleep_microseconds);
    num_iterations++;
  }

  std::ofstream fout;
  fout.open("config.txt", std::ios::out | std::ios::trunc);
  fout << "PSD_y_offset, PSD_x_offset, PSD_sum_y_offset, PSD_sum_x_offset, Piezo_X_offset, Piezo_Y_offset" << std::endl;
  fout << PSD_y_offset / num_iterations << ",";
  fout << PSD_x_offset / num_iterations << ",";
  fout << PSD_sum_y_offset / num_iterations << ",";
  fout << PSD_sum_x_offset / num_iterations << ",";
  fout << piezo_x_offset << ",";
  fout << piezo_y_offset << std::endl;
  fout.close();
}

void PSDInterface::start_PSD_loop()
{
  int16_t val0; // Stores the 16 bit value of our ADC conversion
  int16_t val1; // Stores the 16 bit value of our ADC conversion
  int16_t val2; // Stores the 16 bit value of our ADC conversion
  int16_t val3; // Stores the 16 bit value of our ADC conversion

  auto begin = std::chrono::high_resolution_clock::now();
  size_t iteration = 0;

  std::ofstream fp;
  fp.open("PSDlog.txt", std::ios::out | std::ios::trunc);
  fp << "index,volts0,volts1,volts2,volts3,PSD_x,PSD_y,PSD_sum_x,PSD_sum_y,normX,normY,errorX,errorY,time" << std::endl;

  while (true)
  {
    read(I2CFile0, readBuf0, 2); // Read the contents of the conversion register into readBuf
    read(I2CFile1, readBuf1, 2); // Read the contents of the conversion register into readBuf
    read(I2CFile2, readBuf2, 2); // Read the contents of the conversion register into readBuf
    read(I2CFile3, readBuf3, 2); // Read the contents of the conversion register into readBuf
    val0 = readBuf0[0] << 8 | readBuf0[1]; // Combine the two bytes of readBuf into a single 16 bit result
    val1 = readBuf1[0] << 8 | readBuf1[1]; // Combine the two bytes of readBuf into a single 16 bit result
    val2 = readBuf2[0] << 8 | readBuf2[1]; // Combine the two bytes of readBuf into a single 16 bit result
    val3 = readBuf3[0] << 8 | readBuf3[1]; // Combine the two bytes of readBuf into a single 16 bit result

    PSD_val_mtx.lock();
    volts0 = (float)val0 * gain_voltage / 32767.0;
    volts1 = (float)val1 * gain_voltage / 32767.0;
    volts2 = (float)val2 * gain_voltage / 32767.0;
    volts3 = (float)val3 * gain_voltage / 32767.0;

    PSD_y = volts0 - PSD_y_offset;
    PSD_x = volts1 - PSD_x_offset;
    PSD_sum_y = volts2 - PSD_sum_y_offset;
    PSD_sum_x = volts3 - PSD_sum_x_offset;

    normY = PSD_y / PSD_sum_y;
    normX = (PSD_x / PSD_sum_x) * -1;
    PSD_val_mtx.unlock();

    // Signal that there is fresh data
    // std::unique_lock<std::mutex> lock(PSD_new_data_mtx);
    // PSD_new_data = true;
    // PSD_new_data_cv.notify_all();
    // lock.unlock();
    PSD_new_data_mtx.lock();
    PSD_new_data = true;
    PSD_new_data_mtx.unlock();

    float time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - begin).count();

    fp << iteration << ",";
    fp << volts0 << "," << volts1 << "," << volts2 << "," << volts3 << ",";
    fp << PSD_x << "," << PSD_y << "," << PSD_sum_x << "," << PSD_sum_y << ",";
    fp << normX << "," << normY << ",";
    fp << 0 << "," << 0 << ",";
    fp << time << std::endl;
    iteration++;
    usleep(sleep_microseconds);
  }
}