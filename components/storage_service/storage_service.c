/**
 * @file storage_service.c
 * @brief Storage Service Implementation
 */

#include "storage_service.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

static const char *TAG = "STORAGE_SVC";

static bool s_initialized = false;

esp_err_t storage_service_init(void)
{
    if (s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Initializing storage service");
    
    // Initialize file system manager
    ESP_ERROR_CHECK(fs_manager_init());
    
    // Initialize configuration manager
    ESP_ERROR_CHECK(config_manager_init());
    
    // Mount default file systems
    ESP_ERROR_CHECK(storage_service_mount("/spiffs", "storage", FS_TYPE_SPIFFS));
    ESP_ERROR_CHECK(storage_service_mount("/apps", "apps", FS_TYPE_SPIFFS));
    ESP_ERROR_CHECK(storage_service_mount("/www", "www", FS_TYPE_SPIFFS));
    
    s_initialized = true;
    ESP_LOGI(TAG, "Storage service initialized");
    
    return ESP_OK;
}

esp_err_t storage_service_deinit(void)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Deinitializing storage service");
    
    // Unmount file systems
    storage_service_unmount("/spiffs");
    storage_service_unmount("/apps");
    storage_service_unmount("/www");
    
    s_initialized = false;
    ESP_LOGI(TAG, "Storage service deinitialized");
    
    return ESP_OK;
}

esp_err_t storage_service_mount(const char *mount_point, const char *partition_label, fs_type_t fs_type)
{
    if (!mount_point || !partition_label) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Mounting %s at %s", partition_label, mount_point);
    
    if (fs_type == FS_TYPE_SPIFFS) {
        esp_vfs_spiffs_conf_t conf = {
            .base_path = mount_point,
            .partition_label = partition_label,
            .max_files = 10,
            .format_if_mount_failed = true
        };
        
        esp_err_t ret = esp_vfs_spiffs_register(&conf);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to mount SPIFFS (%s): %s", partition_label, esp_err_to_name(ret));
            return ret;
        }
        
        // Check SPIFFS info
        size_t total = 0, used = 0;
        ret = esp_spiffs_info(partition_label, &total, &used);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "SPIFFS: %d KB total, %d KB used", total / 1024, used / 1024);
        }
    }
    
    ESP_LOGI(TAG, "Successfully mounted %s", mount_point);
    return ESP_OK;
}

esp_err_t storage_service_unmount(const char *mount_point)
{
    if (!mount_point) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Unmounting %s", mount_point);
    
    esp_err_t ret = esp_vfs_spiffs_unregister(strstr(mount_point, "apps") ? "apps" : 
                                              strstr(mount_point, "www") ? "www" : "storage");
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to unmount %s: %s", mount_point, esp_err_to_name(ret));
    }
    
    return ret;
}

esp_err_t storage_service_read_file(const char *filepath, void *buffer, size_t buffer_size, size_t *bytes_read)
{
    if (!filepath || !buffer || buffer_size == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    FILE *file = fopen(filepath, "r");
    if (!file) {
        ESP_LOGD(TAG, "Failed to open file for reading: %s", filepath);
        return ESP_ERR_NOT_FOUND;
    }
    
    size_t read_size = fread(buffer, 1, buffer_size, file);
    fclose(file);
    
    if (bytes_read) {
        *bytes_read = read_size;
    }
    
    ESP_LOGD(TAG, "Read %zu bytes from %s", read_size, filepath);
    return ESP_OK;
}

esp_err_t storage_service_write_file(const char *filepath, const void *data, size_t data_size)
{
    if (!filepath || !data || data_size == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (data_size > MAX_FILE_SIZE) {
        ESP_LOGE(TAG, "File too large: %zu bytes", data_size);
        return ESP_ERR_INVALID_SIZE;
    }
    
    FILE *file = fopen(filepath, "w");
    if (!file) {
        ESP_LOGE(TAG, "Failed to open file for writing: %s", filepath);
        return ESP_FAIL;
    }
    
    size_t written = fwrite(data, 1, data_size, file);
    fclose(file);
    
    if (written != data_size) {
        ESP_LOGE(TAG, "Failed to write complete data to %s", filepath);
        return ESP_FAIL;
    }
    
    ESP_LOGD(TAG, "Wrote %zu bytes to %s", data_size, filepath);
    return ESP_OK;
}

esp_err_t storage_service_delete_file(const char *filepath)
{
    if (!filepath) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (remove(filepath) != 0) {
        ESP_LOGD(TAG, "Failed to delete file: %s", filepath);
        return ESP_FAIL;
    }
    
    ESP_LOGD(TAG, "Deleted file: %s", filepath);
    return ESP_OK;
}

esp_err_t storage_service_create_dir(const char *dirpath)
{
    if (!dirpath) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (mkdir(dirpath, 0755) != 0) {
        ESP_LOGD(TAG, "Failed to create directory: %s", dirpath);
        return ESP_FAIL;
    }
    
    ESP_LOGD(TAG, "Created directory: %s", dirpath);
    return ESP_OK;
}

esp_err_t storage_service_list_dir(const char *dirpath, file_info_t *files, size_t max_files, size_t *num_files)
{
    if (!dirpath || !files || !num_files) {
        return ESP_ERR_INVALID_ARG;
    }
    
    DIR *dir = opendir(dirpath);
    if (!dir) {
        ESP_LOGD(TAG, "Failed to open directory: %s", dirpath);
        return ESP_ERR_NOT_FOUND;
    }
    
    size_t count = 0;
    struct dirent *entry;
    
    while ((entry = readdir(dir)) != NULL && count < max_files) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        strncpy(files[count].name, entry->d_name, sizeof(files[count].name) - 1);
        files[count].name[sizeof(files[count].name) - 1] = '\0';
        files[count].is_directory = (entry->d_type == DT_DIR);
        
        // Get file size if it's a regular file
        if (!files[count].is_directory) {
            char full_path[MAX_PATH_LEN];
            snprintf(full_path, sizeof(full_path), "%s/%s", dirpath, entry->d_name);
            
            struct stat st;
            if (stat(full_path, &st) == 0) {
                files[count].size = st.st_size;
                files[count].mtime = st.st_mtime;
            } else {
                files[count].size = 0;
                files[count].mtime = 0;
            }
        } else {
            files[count].size = 0;
            files[count].mtime = 0;
        }
        
        count++;
    }
    
    closedir(dir);
    *num_files = count;
    
    ESP_LOGD(TAG, "Listed %zu items in %s", count, dirpath);
    return ESP_OK;
}

esp_err_t storage_service_get_file_info(const char *filepath, file_info_t *info)
{
    if (!filepath || !info) {
        return ESP_ERR_INVALID_ARG;
    }
    
    struct stat st;
    if (stat(filepath, &st) != 0) {
        return ESP_ERR_NOT_FOUND;
    }
    
    // Extract filename from path
    const char *filename = strrchr(filepath, '/');
    if (filename) {
        filename++; // Skip the '/'
    } else {
        filename = filepath;
    }
    
    strncpy(info->name, filename, sizeof(info->name) - 1);
    info->name[sizeof(info->name) - 1] = '\0';
    info->size = st.st_size;
    info->mtime = st.st_mtime;
    info->is_directory = S_ISDIR(st.st_mode);
    
    return ESP_OK;
}

bool storage_service_exists(const char *path)
{
    if (!path) {
        return false;
    }
    
    struct stat st;
    return stat(path, &st) == 0;
}

esp_err_t storage_service_get_free_space(const char *mount_point, size_t *free_bytes)
{
    if (!mount_point || !free_bytes) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // For SPIFFS, get partition info
    const char *partition_label = "storage";
    if (strstr(mount_point, "apps")) {
        partition_label = "apps";
    } else if (strstr(mount_point, "www")) {
        partition_label = "www";
    }
    
    size_t total = 0, used = 0;
    esp_err_t ret = esp_spiffs_info(partition_label, &total, &used);
    if (ret != ESP_OK) {
        return ret;
    }
    
    *free_bytes = total - used;
    return ESP_OK;
}