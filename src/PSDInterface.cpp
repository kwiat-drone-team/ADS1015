#include <stdio.h>
#include <fcntl.h>     // open
#include <inttypes.h>  // uint8_t, etc
#include <linux/i2c-dev.h> // I2C bus definitions
#include <sys/ioctl.h> 
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <chrono>
#include <thread>
#include <mutex>
#include "PSDInterface.hpp"

void PSDInterface::start_PSD_loop(){
  std::ofstream fp;
  if(save_to_file){
    fp.open("volts.txt",std::ios::out | std::ios::trunc);
    fp << "index,volts0,volts1,volts2,volts3,PSD_x,PSD_y,PSD_sum_x,PSD_sum_y,normX,normY,time"<<std::endl;
  }

  double volts0;
  double volts1;
  double volts2;
  double volts3;

  int I2CFile0;
  int I2CFile1;
  int I2CFile2;
  int I2CFile3;
  
  uint8_t writeBuf[3];      // Buffer to store the 3 bytes that we write to the I2C device
  uint8_t readBuf0[2];  
  uint8_t readBuf1[2];     // 2 byte buffer to store the data read from the I2C device
  uint8_t readBuf2[2];     // 2 byte buffer to store the data read from the I2C device
  uint8_t readBuf3[2];     // 2 byte buffer to store the data read from the I2C device

  //uint16_t output_code = 0;     // Output code [D11,D10,D9,D8,D7,D6,D5,D4,D3,D2,D1,D0] = 12-bit ADC raw value 2's compliment
  
  int16_t val0;              // Stores the 16 bit value of our ADC conversion
  int16_t val1;              // Stores the 16 bit value of our ADC conversion
  int16_t val2;              // Stores the 16 bit value of our ADC conversion
  int16_t val3;              // Stores the 16 bit value of our ADC conversion
  
  I2CFile0 = open("/dev/i2c-1", O_RDWR);     // Open the I2C device
  if(I2CFile0 < 0){
    std::cout << "Error opening i2c bus: 0" << std::endl;
  }
  I2CFile1 = open("/dev/i2c-1", O_RDWR);     // Open the I2C device
  if(I2CFile1 < 0){
    std::cout << "Error opening i2c bus: 1" << std::endl;
  }
  I2CFile2 = open("/dev/i2c-1", O_RDWR);     // Open the I2C device
  if(I2CFile2 < 0){
    std::cout << "Error opening i2c bus: 2" << std::endl;
  }
  I2CFile3 = open("/dev/i2c-1", O_RDWR);     // Open the I2C device
  if(I2CFile3 < 0){
    std::cout << "Error opening i2c bus: 3" << std::endl;
  }
  
  ioctl(I2CFile0, I2C_SLAVE, ADS0_address);   // Specify the address of the I2C Slave to communicate with
  ioctl(I2CFile1, I2C_SLAVE, ADS1_address);   // Specify the address of the I2C Slave to communicate with
  ioctl(I2CFile2, I2C_SLAVE, ADS2_address);   // Specify the address of the I2C Slave to communicate with
  ioctl(I2CFile3, I2C_SLAVE, ADS3_address);   // Specify the address of the I2C Slave to communicate with
      
  // These three bytes are written to the ADS1115 to set the config register and start a conversion 
  // writeBuf[1] = 0xC0;       // This sets the 8 MSBs of the config register (bits 15-8) to 11000011
  //writeBuf[1] = 0x00;       // This sets the 8 MSBs of the config register (bits 15-8) to 01000011 (Differential Mode)
  // writeBuf[1] = 0x0E;       // This sets the 8 MSBs of the config register (bits 15-8) to 01000011 (Differential Mode)


  //writeBuf[2] = 0xE3;       // This sets the 8 LSBs of the config register (bits 7-0) to 11100011

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
  writeBuf[0] = 1;          // This sets the pointer register so that the following two bytes write to the config register
  
  // Initialize the buffer used to read data from the ADS1115 to 0
  readBuf0[0]= 0;        
  readBuf0[1]= 0;
  
  // Initialize the buffer used to read data from the ADS1115 to 0
  readBuf1[0]= 0;        
  readBuf1[1]= 0;

  // Initialize the buffer used to read data from the ADS1115 to 0
  readBuf2[0]= 0;        
  readBuf2[1]= 0;

  // Initialize the buffer used to read data from the ADS1115 to 0
  readBuf3[0]= 0;        
  readBuf3[1]= 0;

  // Write writeBuf to the ADS1115, the 3 specifies the number of bytes we are writing,
  // this begins a single conversion
  // ioctl(I2CFile0, I2C_SLAVE, ADS0_address);   // Specify the address of the I2C Slave to communicate with
  write(I2CFile0, writeBuf, 3);
  
  // ioctl(I2CFile1, I2C_SLAVE, ADS1_address);   // Specify the address of the I2C Slave to communicate with
  write(I2CFile1, writeBuf, 3);
  
  // ioctl(I2CFile2, I2C_SLAVE, ADS2_address);   // Specify the address of the I2C Slave to communicate with
  write(I2CFile2, writeBuf, 3);   // Specify the address of the I2C Slave to communicate with

  // ioctl(I2CFile3, I2C_SLAVE, ADS3_address);   // Specify the address of the I2C Slave to communicate with
  write(I2CFile3, writeBuf, 3);

  writeBuf[0] = 0;                  // Set pointer register to 0 to read from the conversion register
  write(I2CFile0, writeBuf, 1);
  write(I2CFile1, writeBuf, 1);
  write(I2CFile2, writeBuf, 1);
  write(I2CFile3, writeBuf, 1);
  
  struct timeval start;
  struct timeval end;
  struct timeval i_time;
  double sleep = 80;
  // int NUM_ITERS = 100;

  // std::cout << "Enter a sleep time (microseconds)" << std::endl;
  // std::cin >> sleep;
  // std::cout << "Enter number of iterations" << std::endl;
  // std::cin >> NUM_ITERS;
  float gain_voltage = 6.144;
  
  // gettimeofday(&start,NULL);
  auto begin = std::chrono::high_resolution_clock::now();
  size_t iteration = 0;
  while(true){

    // Wait until data is ready
    //while (!conversionComplete(I2CFile0));

    auto begin_reads = std::chrono::high_resolution_clock::now();
    // ioctl(I2CFile0, I2C_SLAVE, ADS0_address);   // Specify the address of the I2C Slave to communicate with
    read(I2CFile0, readBuf0, 2);        // Read the contents of the conversion register into readBuf

    // ioctl(I2CFile0, I2C_SLAVE, ADS1_address);   // Specify the address of the I2C Slave to communicate with
    read(I2CFile1, readBuf1, 2);        // Read the contents of the conversion register into readBuf

    // ioctl(I2CFile0, I2C_SLAVE, ADS2_address);   // Specify the address of the I2C Slave to communicate with
    read(I2CFile2, readBuf2, 2);        // Read the contents of the conversion register into readBuf

    // ioctl(I2CFile0, I2C_SLAVE, ADS3_address);   // Specify the address of the I2C Slave to communicate with
    read(I2CFile3, readBuf3, 2);        // Read the contents of the conversion register into readBuf
    
    val0 = readBuf0[0] << 8 | readBuf0[1];   // Combine the two bytes of readBuf into a single 16 bit result 
    val1 = readBuf1[0] << 8 | readBuf1[1];   // Combine the two bytes of readBuf into a single 16 bit result 
    val2 = readBuf2[0] << 8 | readBuf2[1];   // Combine the two bytes of readBuf into a single 16 bit result 
    val3 = readBuf3[0] << 8 | readBuf3[1];   // Combine the two bytes of readBuf into a single 16 bit result 

    volts0 = (float)val0*gain_voltage/32767.0;
    volts1 = (float)val1*gain_voltage/32767.0;
    volts2 = (float)val2*gain_voltage/32767.0;
    volts3 = (float)val3*gain_voltage/32767.0;

    PSD_val_mtx.lock();
    PSD_y = volts0 - PSD_y_offset;
    PSD_x = volts1 - PSD_x_offset;
    PSD_sum_y = volts2 - PSD_sum_y_offset;
    PSD_sum_x = volts3 - PSD_sum_x_offset;

    normY = PSD_y / PSD_sum_y;
    normX = (PSD_x / PSD_sum_x)*-1;
    PSD_val_mtx.unlock();
    float time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - begin).count();
    
    // std::cout << volts3 << std::endl;/
    if(save_to_file){
      fp << iteration << ",";
      fp << volts0 << "," << volts1 << "," << volts2 << "," << volts3 << ",";
      fp << PSD_x << "," << PSD_y << "," << PSD_sum_x << "," << PSD_sum_y << ",";
      fp << normX << "," << normY << ",";
      fp << time << std::endl;
    }
    

    // //printf("Voltage Reading %f (V) \n", (float)val0*6.144/32767.0);    // Print the result to terminal, first convert from binary value to mV
    // //printf("Voltage Reading %f (V) \n", (float)val1*6.144/32767.0);    // Print the result to terminal, first convert from binary value to mV
    // //usleep(200);
    // usleep(20);
    // auto time_taken_to_read = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - begin_reads).count();
    // long int max_time = (1/(3300))*1e9;
    iteration++;
    // std::this_thread::sleep_for(std::chrono::nanoseconds(max_time-time_taken_to_read));
    usleep(sleep);
  }

  auto taken = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - begin).count();
  
  // close(I2CFile0);
  //close(I2CFile1);
  
  // printf("Elapsed time : %ld micro seconds\n", taken);
  // printf("Average Sample Rate : %f SPS\n", NUM_ITERS/(taken*1e-6));
}