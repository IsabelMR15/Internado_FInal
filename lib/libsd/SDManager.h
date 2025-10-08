#ifndef SDMANAGER_H_INCLUDED
#define SDMANAGER_H_INCLUDED

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "esp_timer.h"


#define SDSPI_HOST_ID SPI3_HOST
#define SD_MOSI    GPIO_NUM_11 
#define SD_MISO    GPIO_NUM_13  // Set this PIN for using shared SPI option
#define SD_SCLK    GPIO_NUM_12
#define SD_CS      GPIO_NUM_10
#define EXAMPLE_MAX_CHAR_SIZE 64
#define EXAMPLE_MAX_CHAR_SIZE 64

//#define MOUNT_POINT "/sdcard"

class SDManager {
    bool is_sdcard_enabled=false;
    //sdmmc_card_t* sdcard;
    const char *TAG = "i2c-slave";

    public:
        SDManager();
        virtual ~SDManager();
        //bool init_sdspi();
        bool init_sdspi(sdmmc_card_t *sdcard, const char *mountpoint); // configuración inicial        
        int s_example_write_file(const char *path, char *data);
        int s_example_read_file(const char *path);
        bool writeFileDVG(const char *filesd, char *data);
        void setSDcard(bool is_sdcard_enabled );
    protected:
        
    private:

};

#endif