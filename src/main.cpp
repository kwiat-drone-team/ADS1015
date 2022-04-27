#include "PiezoControl.hpp"
#include "PSDInterface.hpp"
#include <thread>

int main(){
    PSDInterface* psd = new PSDInterface();
    PiezoControl* control = new PiezoControl(psd);

    std::thread psd_thread(&PSDInterface::start_PSD_loop, psd);
    std::thread control_thread(&PiezoControl::start_piezo_control, control);
    psd_thread.join();
    control_thread.join();
}