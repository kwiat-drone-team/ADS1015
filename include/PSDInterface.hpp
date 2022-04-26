#pragma once
#include <mutex>

class PSDInterface {
  public:
    double PSD_x, PSD_y, PSD_sum_x, PSD_sum_y;
    double normX, normY;
    float PSD_y_offset = -0.036000604983499995;
    float PSD_x_offset = -0.05687868089950003;
    float PSD_sum_y_offset = -0.0870025349945;
    float PSD_sum_x_offset = 0.009000270000000005;
    uint8_t byte2 = 0xC3;
    uint8_t byte1 = 0x00;
    bool save_to_file = false;
    std::mutex PSD_val_mtx;
    void start_PSD_loop(void);
  private:
    int ADS0_address = 0x48;
    int ADS1_address = 0x49;
    int ADS2_address = 0x4a;
    int ADS3_address = 0x4b;
};
