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

    while (true)
    {
        pcx.MoveTo(X_offset);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        pcx.MoveTo(X_offset - 200);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        pcx.MoveTo(X_offset);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        pcx.MoveTo(X_offset + 200);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    float KpX = 0.0; // Proportional gain constant in x direction
    float KpY = 0.0; // Proportional gain constant in y direction
    float KdX = 0.0;
    float KdY = 0.0;
    int PIEZO_CENTER_X = 0;
    int PIEZO_CENTER_Y = 0;
    float PSD_CENTER_X = 0;
    float PSD_CENTER_Y = 0;

    int errorX = 0;
    int errorY = 0;

    bool signal_lost = true; // Flag for signal loss between while loop iterations

    float PSD_y;
    float PSD_x;
    float PSD_sum_y;
    float PSD_sum_x;
    float normX;
    float normY;
    long long int PSD_timestamp;
    long long int local_timestamp;
    struct timeval timer_usec;

    std::tuple<int, int> position = std::make_tuple(0, 0);
    std::tuple<int, int> last_lock_pos = std::make_tuple(0, 0);

    std::chrono::high_resolution_clock::time_point tstart = std::chrono::high_resolution_clock::now();

    while (true)
    {
        // POSSIBLY DON'T NEED
        //*******************************
        // int x =0;
        // int y =0;
        // int counts = 0;
        // position_mutex.lock();
        // x = x_pos;
        // y = y_pos;
        // counts = bright_counts;
        // position_mutex.unlock();
        //*******************************

        // TODO: Insert and format/scale PSD data for use in feedback alg
        //  anywhere var's x, y show up is where new PSD data will go. May also create new vars based on what PSD data is able to give us.
        // PSD sums will be used for measuring intensity (this will be counts)

        // update has structure:
        //   adc0
        //   adc1
        //   adc2
        //   adc3
        //   timestamp

        // //Read piezo encoder positions
        encoderY = pcy.GetPosition();
        encoderX = pcx.GetPosition();
        std::cout << encoderY << std::endl;
        std::cout << encoderX << std::endl;

        // MaxRange
        float maxRange = 0.3;

        // PSD Calibration
        //  if (psdCalFlag == 1){

        //     //Change offset ~second
        //     if (PSDCounter > 200){

        //         //Increment Offset
        //         PSD_CENTER_X = PSD_CENTER_X + 0.1;

        //         //Reset X-Offset
        //         if (PSD_CENTER_X > maxRange){

        //             //Make X-Offset wrap around
        //             PSD_CENTER_X = -1*maxRange;

        //             //Increment Y
        //             PSD_CENTER_Y = PSD_CENTER_Y + 0.1;

        //             //Reset Y-Offset
        //             if (PSD_CENTER_Y > maxRange){

        //                 //Make X-Offset wrap around
        //                 PSD_CENTER_Y = -1*maxRange;
        //             }
        //         }

        //         //Reset PSDCounter
        //         PSDCounter = 0;
        //     }

        //     //Increment PSDCounter
        //     PSDCounter++;
        // }

        // Receive new ZMQ Message
        //  Message PSD_update = s_recv_dat(&subscriber);

        // Generate Local Timestamp
        //  if (!gettimeofday(&timer_usec, NULL)) {
        //      local_timestamp = ((long long int) timer_usec.tv_sec) * 1000000ll + (long long int) timer_usec.tv_usec;
        //  }
        //  else{
        //  local_timestamp = -1;
        //  }

        // Save local timestamp (To analyze instantaneous network latency)

        // Format PSD outputs
        psd->PSD_val_mtx.lock();
        PSD_x = psd->PSD_x;
        PSD_y = psd->PSD_y;
        PSD_sum_x = psd->PSD_sum_x;
        PSD_sum_y = psd->PSD_sum_y;
        normX = psd->normX;
        normY = psd->normY;
        psd->PSD_val_mtx.unlock();

        /*         //Project normX & normY positions to the range [-1,1]
            if (normX > 1){
                normX = 1;
            }
            if (normX < -1){
                normX = -1;
            }
            if (normY > 1){
                normY = 1;
            }
            if (normY < -1){
                normY = -1;
            } */

        // Update Previous_Long_Range values
        //  if (Long_Range_x != 0){
        //      Previous_Long_Range_x = Long_Range_x;
        //  }
        //  if (Long_Range_y != 0){
        //      Previous_Long_Range_y = Long_Range_y;
        //  }

        // Extract Position of Long-Range Zoom Camera
        //  IRG_m_data.lock();
        //  Long_Range_x = IR_x;
        //  Long_Range_y = IR_y;
        //  IRG_m_data.unlock();

        // Use last good location if current value is zero
        //  if (Long_Range_x == 0){
        //      Long_Range_x = Previous_Long_Range_x;
        //  }
        //  if (Long_Range_y == 0){
        //      Long_Range_y = Previous_Long_Range_y;
        //  }

        errorX = int(((normX - PSD_CENTER_X) * KpX));      // Error calculation. Currently just a proportional feedback.
        errorY = int((-1 * (normY - PSD_CENTER_Y) * KpY)); // Error calculation. Currently just a proportional feedback.

        // Bound control signals to range [u_min,u_max]
        int u_min = -2000;
        int u_max = 2000;

        if (errorX > u_max)
        {
            errorX = u_max;
        }
        if (errorX < u_min)
        {
            errorX = u_min;
        }
        if (errorY > u_max)
        {
            errorY = u_max;
        }
        if (errorY < u_min)
        {
            errorY = u_min;
        }

        // pc1->MoveTo(0);
        // pc2->MoveTo(0);

        // Display Current State (Quad Cell x, Quad Cell y, Long Range x, Long Range y, u_x, u_y)
        // TODO: update this to "PSD X-Position, PSD Y-Position, PSD X-Sum, PSD Y-Sum, Long-range x, Long-range y, u_x, u_y"
        //  cout << PSD_x << ", " << PSD_y << ", " << PSD_sum_x << ", " << PSD_sum_y << ", " << Long_Range_x << ", " << Long_Range_y << ", " << errorX << ", " << errorY << "," << encoderX << "," << encoderY << "," << PSD_CENTER_X << "," << PSD_CENTER_Y << endl;

        // Write values to Control Log File
        //  myFile << PSD_x << ", " << PSD_y << ", " << PSD_sum_x << ", " << PSD_sum_y << ", " << Long_Range_x << ", " << Long_Range_y << ", " << errorX << ", " << errorY << ", " << PSD_timestamp << ", " << local_timestamp << ", " << encoderX << "," << encoderY << "," << PSD_CENTER_X << "," << PSD_CENTER_Y << endl;

        // printf("Error: %d, %d %d\n", errorX, errorY, counts);
        //  THIS PARAMETER NEEDS TO BE TUNED
        //  if (PSD_sum_y < 0.05) { // Check if we believe the beam is not hitting the quad cell
        //      if (!signal_lost) { // If we hadn't already lost the signal on the previous loop iteration
        //          std::chrono::high_resolution_clock::time_point t = std::chrono::high_resolution_clock::now();
        //          printf("LOST SIGNAL\n");
        //          auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t - tstart).count();
        //          std::cout << duration << std::endl; // Print out the time that the lock was lost at
        //          position = make_tuple(0,0); // Reset to center if position is lost
        //      }
        //      signal_lost = true;

        //     /* Acquisition code */
        //     if (get<0>(position) == -5 && get<1>(position) == 5) { // Check if the end of the spiral pattern is reached
        //         position = make_tuple(0,0); // Reset to center of spiral
        //     }
        //     if (get<0>(last_lock_pos) > 250 || get<0>(last_lock_pos) < -250 ||
        //             get<1>(last_lock_pos) > 250 || get<1>(last_lock_pos) < -250) { // Check if the beam is drifting too far from the center
        //         position = make_tuple(0,0); // Reset to center of spiral
        //         last_lock_pos = make_tuple(0,0); // Reset the center offset
        //     }
        //     position = acq.NextStep(position);
        //     // printf("x:%d y:%d\n", get<0>(position), get<1>(position));
        //     /* Calculate new mirror-encoder positions */
        //     //cout << "Searching" << endl;

        //     //Factor in Long-Range Camera Input
        //     // Error_Long_Range_y = int(round((Long_Range_x - Long_Range_Reference_x)*KLx));
        //     // Error_Long_Range_x = int(round((Long_Range_y - Long_Range_Reference_y)*KLy));

        //     //Long-Range Camera updates tracking
        //     //pc1.MoveTo(PIEZO_CENTER_X + Error_Long_Range_x);
        //     //pc2.MoveTo(PIEZO_CENTER_Y + Error_Long_Range_y);

        //     //Serpentine Search with Long-Range Camera updates:
        //     // pc1.MoveTo(get<0>(position)+PIEZO_CENTER_X + Error_Long_Range_x);
        //     // pc2.MoveTo(get<1>(position)+PIEZO_CENTER_Y + Error_Long_Range_y);

        //     //Original Code from Kyle:
        //     //pc1.MoveTo(get<0>(position)*2+PIEZO_CENTER_X +2*get<0>(last_lock_pos)); // TODO refactor this calculation to a class
        //     //pc2.MoveTo(get<1>(position)*2+PIEZO_CENTER_Y+2*get<1>(last_lock_pos)); // TODO refactor this calculation to a class
        //     // Give the mirror time to turn. TODO refactor this calculation to a class
        //     this_thread::sleep_for (chrono::microseconds(acq.quad_delay_us+acq.turn_delay_us*200));//need to look at this library for potential delay reductions

        //     continue;
        // }
        //     if(signal_lost) {
        //         std::chrono::high_resolution_clock::time_point t = std::chrono::high_resolution_clock::now();
        //         printf("LOCKED...\n");
        //         auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t - tstart).count();
        //         std::cout << duration << std::endl; // Print out the time that the lock was regained
        //         //last_lock_pos = make_tuple(get<0>(position)+get<0>(last_lock_pos),
        //                 //get<1>(position)+get<1>(last_lock_pos))
        //         last_lock_pos = std::make_tuple(0,0);
        // ; // Move the center offset to the last known lock start. TODO: replace with reading out the current position
        //             //printf("LOCK POS: %d %d\n", get<0>(last_lock_pos), get<1>(last_lock_pos));
        //         }
        //         signal_lost = false;
        //         // Ix += v1norm/100.0; // Quad cell integral calculation
        //         // Iy += v2norm/100.0; // Quad cell integral calculation
        //         // std::cout << "Correction in X,Y:" << errXstr << ", " << errYstr << std::endl;
        //         if(errorX != 0) { // TODO: consider replacing with an absolute threshold instead of just 0, refactor to a top level DEFINE
        //             //printf("Moving in X\n");
        //             pc1->Step(errorX);
        //         }
        //         if(errorY != 0) { // TODO: consider replacing with an absolute threshold instead of just 0, refactor to a top level DEFINE
        //         //  printf("Moving in Y\n");
        //             pc2->Step(errorY);
        //         }
        //         //This may need to be removed later
        //         //this_thread::sleep_for (chrono::milliseconds(10)); // Give the motors time to move for the main feedback
        //     }
        // myFile.close();
        return;
    }
}