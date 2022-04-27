#include "PiezoControl.hpp"
#include "PSDInterface.hpp"
#include <thread>

int main(int argc, char **argv)
{
    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-d") == 0)
        {
            PSDInterface *psd = new PSDInterface();
            std::cout << "Turning fast steering mirrors" << std::endl;
            PiezoControl *control = new PiezoControl(psd);
            std::thread piezo_start_dark(&PiezoControl::start_dark_count, control);
            piezo_start_dark.join();

            std::cout << "Taking dark counts" << std::endl;
            std::thread psd_thread(&PSDInterface::start_dark_count, psd, 20);
            psd_thread.join();

            std::cout << "Returning mirrors to original position" << std::endl;
            std::thread piezo_end_dark(&PiezoControl::end_dark_count, control);
            piezo_end_dark.join();
            return 0;
        }
        if (strcmp(argv[i], "-h") == 0)
        {
            std::cout << "Command line help:" << std::endl;
            std::cout << "_____________________________________________________________________" << std::endl;
            std::cout << "-h        Show this help menu." << std::endl;
            std::cout << "-d    	Perform dark count measurement." << std::endl;
            exit(1);
        }
    }
    if(argc == 1){
        PSDInterface *psd = new PSDInterface();
        std::thread psd_record(&PSDInterface::start_PSD_loop, psd);
        PiezoControl *control = new PiezoControl(psd);
        std::thread piezo_start(&PiezoControl::start_piezo_control, control);
        piezo_start.join();
        psd_record.join();
        return 0;
    }
}