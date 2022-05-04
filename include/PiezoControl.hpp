#pragma once
#include "PiezoController.h"
#include "PSDInterface.hpp"
static const char *SER1_PORT = "/dev/ttyUSB1";
static const char *SER2_PORT = "/dev/ttyUSB0";

constexpr size_t BAUD_RATE = 57600;
constexpr float signal_threshold = 0.02;

class PiezoControl
{
public:
    PiezoController pcy;
    PiezoController pcx;
    PiezoControl(PSDInterface *interface) : pcy(SER1_PORT, BAUD_RATE), pcx(SER2_PORT, BAUD_RATE)
    {
        psd = interface;
        read_piezo_offset();
    }
    void read_piezo_offset(void);
    void start_piezo_control(void);
    void write_piezo_offset(int, int);
    void start_dark_count(void);
    void end_dark_count(void);

private:
    PSDInterface *psd;
    size_t X_offset, Y_offset;
};