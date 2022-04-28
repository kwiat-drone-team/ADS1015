// //Piezo serial ports
#include "PiezoController.h"
#include "PiezoControl.hpp"
#include "serialib.h"
#include <chrono>
#include <thread>
#include <tuple>
#include <fstream>

constexpr size_t ADD180 = 16384;

int encoderY;
int encoderX;
int PSDCounter;
int psdCalFlag;

void PiezoControl::read_piezo_offset(){
    std::ifstream fin;
    fin.open("config.txt", std::ios::in);
    if(!fin){
        std::cout << "Error opening configuration file in Piezo setup!" << std::endl;
        exit(1);
    }
    std::string config;
    std::getline(fin, config); // Skip header
    for (int i = 0; i < 5; i++)
    {
        std::getline(fin, config, ',');
    }
    X_offset = std::stoi(config);
    std::getline(fin, config, ',');
    Y_offset = std::stoi(config);
    fin.close();
}

void PiezoControl::start_dark_count()
{
    pcy.MoveTo(Y_offset + ADD180);
    pcx.MoveTo(X_offset + ADD180);
    std::this_thread::sleep_for(std::chrono::seconds(4));
    return;
}

void PiezoControl::end_dark_count()
{
    pcy.MoveTo(Y_offset);
    pcx.MoveTo(X_offset);
    std::this_thread::sleep_for(std::chrono::seconds(4));
    return;
}

void PiezoControl::start_piezo_control()
{

    // while(1){

    //     std::cout << "Enter x-starting value?" << std::endl;
    //     std::cin >> X_offset;

    //     std::cout << "Enter y-starting value?" << std::endl;
    //     std::cin >> Y_offset;

    //     pc1.MoveTo(Y_offset);
    //     pc2.MoveTo(X_offset);

    // }

    pcy.MoveTo(Y_offset);
    pcx.MoveTo(X_offset);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    // while (true)
    // {
    //     pcx.Step(10000);
    //     std::this_thread::sleep_for(std::chrono::seconds(1));
    //     pcx.Step(-10000);
    //     std::this_thread::sleep_for(std::chrono::seconds(1));
    // }

    // while (true)
    // {
    //     pcx.MoveTo(X_offset);
    //     std::this_thread::sleep_for(std::chrono::seconds(1));
    //     pcx.MoveTo(X_offset - 200);
    //     std::this_thread::sleep_for(std::chrono::seconds(1));
    //     pcx.MoveTo(X_offset);
    //     std::this_thread::sleep_for(std::chrono::seconds(1));
    //     pcx.MoveTo(X_offset + 200);
    //     std::this_thread::sleep_for(std::chrono::seconds(1));
    // }

    float KpX = 2000; // Proportional gain constant in x direction 2000 works well
    float KpY = 700; // Proportional gain constant in y direction 700 works well
    float PSD_CENTER_X = 0;
    float PSD_CENTER_Y = 0;

    int errorX = 0;
    int errorY = 0;

    float PSD_y;
    float PSD_x;
    float PSD_sum_y;
    float PSD_sum_x;
    float normX;
    float normY;

    std::cout << "Get Ready" << std::endl;
    usleep(3000000);
    std::cout << "Starting Control" << std::endl;

    while (true)
    {
       
        // encoderY = pcy.GetPosition();
        // encoderX = pcx.GetPosition();
        // std::cout << encoderY << std::endl;
        // std::cout << encoderX << std::endl;

        // MaxRange
        float signal_threshold = 0.02;

        // Format PSD outputs
        psd->PSD_val_mtx.lock();
        PSD_x = psd->PSD_x;
        PSD_y = psd->PSD_y;
        PSD_sum_x = psd->PSD_sum_x;
        PSD_sum_y = psd->PSD_sum_y;
        normX = psd->normX;
        normY = psd->normY;
        psd->PSD_val_mtx.unlock();

        // Check if we have a valid signal
        if((PSD_sum_y > signal_threshold) && (-1*PSD_sum_x > signal_threshold)){

            //std::cout << normX << ", " << normY << std::endl;

            errorX = int(normY*KpX);      // Error calculation. Currently just a proportional feedback.
            errorY = int(-1*normX*KpY); // Error calculation. Currently just a proportional feedback.

            // Bound control signals to range [u_min,u_max]
            int u_max = 2000;

            if (errorX > u_max)
            {
                errorX = u_max;
            }
            if (errorX < -1*u_max)
            {
                errorX = -1*u_max;
            }
            if (errorY > u_max)
            {
                errorY = u_max;
            }
            if (errorY < -1*u_max)
            {
                errorY = -1*u_max;
            }

            // Take Action
            pcx.Step(errorX);
            pcy.Step(errorY);

            // Approximate the delay of PSD
            usleep(1600);
        }
    
    }

    return;
}