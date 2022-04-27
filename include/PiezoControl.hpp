#pragma once
#include "PiezoController.h"
#include "PSDInterface.hpp"
// TODO: change these
#define SER1_PORT "/dev/ttyUSB1" 
#define SER2_PORT "/dev/ttyUSB0"

#define BAUD_RATE 57600

class PiezoControl {
    public:
        PiezoController pcy;
        PiezoController pcx;
        PiezoControl(PSDInterface* interface): pcy(SER1_PORT, BAUD_RATE), pcx(SER2_PORT, BAUD_RATE){
            psd = interface;
        }
        void start_piezo_control(void);
        
    private:
        PSDInterface* psd;
};