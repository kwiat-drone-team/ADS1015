#pragma once
#include "PiezoController.h"
#include "PSDInterface.hpp"
// TODO: change these
#define SER2_PORT "/dev/ttyUSB0"
#define SER1_PORT "/dev/ttyUSB1"

class PiezoControl {
    public:
        PiezoController* pc1;
        PiezoController* pc2;
        size_t baud_rate = 57600;
        PiezoControl(PSDInterface* interface){
            psd = interface;
            PiezoController p1(SER1_PORT, baud_rate);
            PiezoController p2(SER2_PORT, baud_rate);
            pc1 = &p1;
            pc2 = &p2;
        }
        void start_piezo_control(void);
        
    private:
        PSDInterface* psd;
};