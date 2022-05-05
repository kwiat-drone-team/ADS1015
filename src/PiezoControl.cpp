// //Piezo serial ports
#include "PiezoController.h"
#include "PiezoControl.hpp"
#include "serialib.h"
#include <chrono>
#include <thread>
#include <tuple>
#include <fstream>
#include <cmath>

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
    std::cout << X_offset << std::endl;
    std::getline(fin, config, ',');
    Y_offset = std::stoi(config);
    std::cout << Y_offset << std::endl;
    fin.close();
}

void PiezoControl::write_piezo_offset(int x_offset, int y_offset){
    std::ifstream fin;
    fin.open("config.txt", std::ios::in);
    if(!fin){
        std::cout << "Error opening configuration file in PSDInterface setup!" << std::endl;
        exit(1);
    }
    std::string config;
    std::getline(fin, config); // Skip header
    std::getline(fin,config,',');
    float PSD_y_offset = std::stod(config);
    std::getline(fin,config,',');
    float PSD_x_offset = std::stod(config);
    std::getline(fin,config,',');
    float PSD_sum_y_offset = std::stod(config);
    std::getline(fin,config,',');
    float PSD_sum_x_offset = std::stod(config);
    fin.close();

    std::ofstream fout;
    fout.open("config.txt", std::ios::out | std::ios::trunc);
    if(!fout){
        std::cout << "Error opening configuration file in PSDInterface setup!" << std::endl;
        exit(1);
    }
    fout << "PSD_y_offset, PSD_x_offset, PSD_sum_y_offset, PSD_sum_x_offset, Piezo_X_offset, Piezo_Y_offset" << std::endl;
    fout << PSD_y_offset << "," << PSD_x_offset << "," << PSD_sum_y_offset << "," << PSD_sum_x_offset << "," << x_offset << "," << y_offset << std::endl;
    fout.close();
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

    std::this_thread::sleep_for(std::chrono::seconds(2));

    // while (true)
    // {
    //     int step_size = 237;

    //     pcx.Step(step_size);
    //     usleep(370);
    //     pcy.Step(step_size);
    //     usleep(370);
    //     pcx.Step(-1*step_size);
    //     usleep(370);
    //     pcy.Step(-1*step_size);
    //     usleep(370);
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

    // while (true)
    // {
    //     pcy.Step(40000);
    //     // pcx.MoveTo(X_offset - 15000);
    //     std::this_thread::sleep_for(std::chrono::seconds(1));
    //     // pcx.MoveTo(X_of)
    //     pcy.Step(-40000);
    //     std::this_thread::sleep_for(std::chrono::seconds(1));
    // }

    std::ofstream fp;
    fp.open("volts.txt", std::ios::out | std::ios::trunc);
    fp << "index,volts0,volts1,volts2,volts3,PSD_x,PSD_y,PSD_sum_x,PSD_sum_y,normX,normY,errorX,errorY,time" << std::endl;

    float KpX = 1000; // Proportional gain constant in x direction 2000 works well
    float KpY = 1000; // Proportional gain constant in y direction 700 works well
    // float KpX = 500; // Proportional gain constant in x direction 2000 works well
    // float KpY = 500; // Proportional gain constant in x direction 2000 works well

    float PSD_CENTER_X = 0;
    float PSD_CENTER_Y = 0;

    int errorX = 0;
    int errorY = 0;

    float PSD_y, PSD_x, PSD_sum_y, PSD_sum_x = 0;
    float normX, normY = 0;
    double volts0, volts1, volts2, volts3 = 0;

    std::cout << "Get Ready" << std::endl;
    usleep(3000000);
    std::cout << "Starting Control" << std::endl;
    size_t iteration = 0;
    // std::unique_lock<std::mutex> lck(psd->PSD_new_data_mtx);
    auto begin = std::chrono::high_resolution_clock::now();

    while (true)
    {
       
        // encoderY = pcy.GetPosition();
        // encoderX = pcx.GetPosition();
        // std::cout << encoderY << std::endl;
        // std::cout << encoderX << std::endl;

        // MaxRange

        // Wait until new data but with condition variable
        // std::unique_lock<std::mutex> lock(psd->PSD_new_data_mtx);
        // while(!psd->PSD_new_data){
        //     psd->PSD_new_data_cv.wait(lock);
        // }

        // Wait until new data is available
        psd->PSD_new_data_mtx.lock();
        if(psd->PSD_new_data){
            psd->PSD_new_data = false;
            psd->PSD_new_data_mtx.unlock();
        }
        else{
            psd->PSD_new_data_mtx.unlock();
            continue;
        }
        // Format PSD outputs
        psd->PSD_val_mtx.lock();
        volts0 = psd->volts0;
        volts1 = psd->volts1;
        volts2 = psd->volts2;
        volts3 = psd->volts3;
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

            errorX = int(round(normY*KpX));      // Error calculation. Currently just a proportional feedback.
            //errorY = int(round(-1*normX*KpY)); // Error calculation. Currently just a proportional feedback.
            // New Bench
            errorY = int(round(normX*KpY)); // Error calculation. Currently just a proportional feedback.

            // Bound control signals to range [u_min,u_max]
            int u_max = 1000;

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

            // std::cout << errorX << std::endl;

            // Approximate the delay of PSD
            //usleep(1600);
            usleep(2000);
        }

        float time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - begin).count();

        fp << iteration << ",";
        fp << volts0 << "," << volts1 << "," << volts2 << "," << volts3 << ",";
        fp << PSD_x << "," << PSD_y << "," << PSD_sum_x << "," << PSD_sum_y << ",";
        fp << normX << "," << normY << ",";
        fp << errorX << "," << errorY << ",";
        fp << time << std::endl;
        iteration++;
    }

    return;
}