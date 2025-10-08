#include "../lib/libsd/SDManager.h"
//static const char *TAG = "i2c-slave";


SDManager::SDManager() {

}

SDManager::~SDManager(){

}

//bool SDManager::init_sdspi(){
bool SDManager::init_sdspi(sdmmc_card_t *sdcard, const char *mountpoint) {  
    static const char *TAG2 = "SPI DVG";    
    sdspi_device_config_t device_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    device_config.host_id = SDSPI_HOST_ID;
    device_config.gpio_cs = SD_CS;  

    //device_config.gpio_cd = -1;   // SD Card detect    

    ESP_LOGI(TAG2, "Initializing SD card");
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = device_config.host_id;

    esp_vfs_fat_mount_config_t mount_config = 
    {
        //.format_if_mount_failed = true,
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024,
    };

    ESP_LOGI(TAG2, "Initializing SPI BUS");
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = SD_MOSI,
        .miso_io_num = SD_MISO,
        .sclk_io_num = SD_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4092,
    };
    esp_err_t ret = spi_bus_initialize(SDSPI_HOST_ID, &bus_cfg, SDSPI_DEFAULT_DMA);    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return ESP_FAIL;
    }
    else{ESP_LOGI(TAG2, "Bus initialized");}

    ESP_LOGI(TAG2, "Mounting filesystem");
    //ret = esp_vfs_fat_sdspi_mount(MOUNT_POINT, &host, &device_config, &mount_config, &sdcard);
    ret = esp_vfs_fat_sdspi_mount(mountpoint, &host, &device_config, &mount_config, &sdcard);    
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG2, "Failed to mount filesystem. \n"
                     "If you want the card to be formatted, enable above in mount_config.");
            return ESP_FAIL;
        } else {
            ESP_LOGE(TAG2, "Failed to initialize the card (%s). \n"
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
            return ESP_FAIL;
        }
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Filesystem mounted");

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, sdcard);

    return ESP_OK;
}

int SDManager::s_example_write_file(const char *path, char *data){
    int64_t fr_start = esp_timer_get_time();
    ESP_LOGI(TAG, "Opening file %s", path);
    //FILE *f = fopen(path, "w");
    FILE *f = fopen(path, "r"); //en modo lectura para verificar su existencia
    if (f != NULL) {
        fclose(f);
        f = fopen(path, "a"); //si ya existe, abre el archivo en modo añadir
    } else {
        f = fopen(path, "w"); //si no existe, abre el archivo en modo escritura
    }
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        //f = fopen(path, "w");
        return ESP_FAIL;
    }
    fprintf(f, "%s", data);
    fclose(f);
    ESP_LOGI(TAG, "File written");
    int64_t fr_end = esp_timer_get_time();
    ESP_LOGI(TAG,"time: %10lums\n", (uint32_t)((fr_end - fr_start)/1000)); //tiempo de escritura en milisegundos
    return ESP_OK;
}

int SDManager::s_example_read_file(const char *path){
    ESP_LOGI(TAG, "Reading file %s", path);
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return ESP_FAIL;
    }
    char line[EXAMPLE_MAX_CHAR_SIZE];
    fgets(line, sizeof(line), f);
    fclose(f);

    // strip newline
    char *pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(TAG, "Read from file: '%s'", line);

    return ESP_OK;
}

bool SDManager::writeFileDVG(const char *filesd, char *data){
    bool reswrf=false;
    if(is_sdcard_enabled){
        ESP_LOGI(TAG, "Entrando a writeFileDVG");
        reswrf=true;
        esp_err_t ret = s_example_write_file(filesd, data);
        if (ret != ESP_OK) {
            ESP_LOGI(TAG, "Error al crear el archivo: '%s'", filesd);
        } else{
            ESP_LOGI(TAG, "Archivo: '%s' creado con éxito!", filesd);
            reswrf=true;
        }
    }
    return reswrf;
}

void SDManager::setSDcard(bool sdcard_enabled ){
    this->is_sdcard_enabled=sdcard_enabled;
}