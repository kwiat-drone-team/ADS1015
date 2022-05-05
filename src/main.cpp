#include "PiezoControl.hpp"
#include "PSDInterface.hpp"
#include <thread>
#include <string>

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
        if (strcmp(argv[i], "-a") == 0){
            PSDInterface *psd = new PSDInterface();
            std::thread psd_record(&PSDInterface::start_PSD_loop, psd);
            PiezoControl *control = new PiezoControl(psd);
            control->pcx.MoveTo(control->X_offset);
            control->pcy.MoveTo(control->Y_offset);
            float normX;
            float normY;
            while(1){
                psd->PSD_val_mtx.lock();
                normX = psd->normX;
                normY = psd->normY;
                psd->PSD_val_mtx.unlock();
                std::printf("normX: %.3f normY: %.3f     \r", normX, normY);
                // std::cout << "normX: " <<  normX  << " normY: " << normY << "\r";
            }
        }
        if (strcmp(argv[i], "-o") == 0)
        {
            PSDInterface *psd = new PSDInterface();
            std::thread psd_record(&PSDInterface::start_PSD_loop, psd);
            PiezoControl *control = new PiezoControl(psd);
            // std::thread piezo_start(&PiezoControl::start_piezo_control, control);
    
            std::cout << "Key Mapping: " << std::endl;
            std::cout << "w = move up, a = move left" << std::endl;
            std::cout << "s = move down, d = move right" << std::endl;
            std::cout << "r = type new delta" << std::endl;
            std::cout << "q = save values to config.txt and exit" << std::endl;
            char response;
            int delta = 50;
            std::string new_delta;
            while(1){
                std::cout << "Offsets are: (X) " << control->X_offset << " (Y) " << control->Y_offset << std::endl;
                std::cin >> response;

                switch(response){
                    case 'w':
                        control->Y_offset -= delta;
                        control->pcy.MoveTo(control->Y_offset);
                        break;
                    case 'a':
                        control->X_offset -= delta;
                        control->pcx.MoveTo(control->X_offset);
                        break;
                    case 's':
                        control->Y_offset += delta;
                        control->pcy.MoveTo(control->Y_offset);
                        break;
                    case 'd':
                        control->X_offset += delta;
                        control->pcx.MoveTo(control->X_offset);
                        break;
                    case 'r':
                        std::cout << "Type in new delta value" << std::endl;
                        std::cin >> new_delta;
                        delta = std::stoi(new_delta);
                        break;
                    case 'q':
                        control->write_piezo_offset(control->X_offset, control->Y_offset);
                        std::cout << "X and Y offsets are written to config.txt" << std::endl;
                        exit(1);
                        break;
                    default:
                        continue;
                }
            }
            // piezo_start.join();
            psd_record.join();
            return 0;
        }
        if (strcmp(argv[i], "-h") == 0)
        {
            std::cout << "Command line help:" << std::endl;
            std::cout << "_____________________________________________________________________" << std::endl;
            std::cout << "-h        Show this help menu."                                       << std::endl;
            std::cout << "-d    	Perform dark count measurement."                            << std::endl;
            std::cout << "-o    	Update Piezo Offsets."                                      << std::endl;
            std::cout << "-a    	Adjust pan/tilt on mirrors (PSD output with no control)."   << std::endl;
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