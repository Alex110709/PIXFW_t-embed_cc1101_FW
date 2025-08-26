/**
 * @file storage_service.h
 * @brief Storage Service for JavaScript Apps
 */

#ifndef STORAGE_SERVICE_H
#define STORAGE_SERVICE_H

#include "esp_err.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_PATH_LEN 256
#define MAX_FILE_SIZE (1024 * 1024) // 1MB max file size

// File system types
typedef enum {
    FS_TYPE_SPIFFS,
    FS_TYPE_FATFS,
    FS_TYPE_LITTLEFS
} fs_type_t;

// File info structure
typedef struct {
    char name[64];
    size_t size;
    time_t mtime;
    bool is_directory;
} file_info_t;

/**
 * @brief Initialize storage service
 * @return ESP_OK on success
 */
esp_err_t storage_service_init(void);

/**
 * @brief Deinitialize storage service
 * @return ESP_OK on success
 */
esp_err_t storage_service_deinit(void);

/**
 * @brief Mount file system
 * @param mount_point Mount point path
 * @param partition_label Partition label
 * @param fs_type File system type
 * @return ESP_OK on success
 */
esp_err_t storage_service_mount(const char *mount_point, const char *partition_label, fs_type_t fs_type);

/**
 * @brief Unmount file system
 * @param mount_point Mount point path
 * @return ESP_OK on success
 */
esp_err_t storage_service_unmount(const char *mount_point);

/**
 * @brief Read file content
 * @param filepath File path
 * @param buffer Output buffer
 * @param buffer_size Buffer size
 * @param bytes_read Output bytes read
 * @return ESP_OK on success
 */
esp_err_t storage_service_read_file(const char *filepath, void *buffer, size_t buffer_size, size_t *bytes_read);

/**
 * @brief Write file content
 * @param filepath File path
 * @param data Data to write
 * @param data_size Data size
 * @return ESP_OK on success
 */
esp_err_t storage_service_write_file(const char *filepath, const void *data, size_t data_size);

/**
 * @brief Delete file
 * @param filepath File path
 * @return ESP_OK on success
 */
esp_err_t storage_service_delete_file(const char *filepath);

/**
 * @brief Create directory
 * @param dirpath Directory path
 * @return ESP_OK on success
 */
esp_err_t storage_service_create_dir(const char *dirpath);

/**
 * @brief List directory contents
 * @param dirpath Directory path
 * @param files Output file info array
 * @param max_files Maximum files to list
 * @param num_files Output number of files
 * @return ESP_OK on success
 */
esp_err_t storage_service_list_dir(const char *dirpath, file_info_t *files, size_t max_files, size_t *num_files);

/**
 * @brief Get file info
 * @param filepath File path
 * @param info Output file info
 * @return ESP_OK on success
 */
esp_err_t storage_service_get_file_info(const char *filepath, file_info_t *info);

/**
 * @brief Get free space
 * @param mount_point Mount point
 * @param free_bytes Output free bytes
 * @return ESP_OK on success
 */
esp_err_t storage_service_get_free_space(const char *mount_point, size_t *free_bytes);

/**
 * @brief Check if path exists
 * @param path Path to check
 * @return true if exists
 */
bool storage_service_exists(const char *path);

// File system manager functions
esp_err_t fs_manager_init(void);
esp_err_t fs_manager_format_partition(const char *partition_label, fs_type_t fs_type);
esp_err_t fs_manager_check_partition(const char *partition_label);

// Configuration manager functions
esp_err_t config_manager_init(void);
esp_err_t config_manager_set_string(const char *namespace, const char *key, const char *value);
esp_err_t config_manager_get_string(const char *namespace, const char *key, char *value, size_t max_len);
esp_err_t config_manager_set_int(const char *namespace, const char *key, int32_t value);
esp_err_t config_manager_get_int(const char *namespace, const char *key, int32_t *value, int32_t default_value);
esp_err_t config_manager_delete_key(const char *namespace, const char *key);
esp_err_t config_manager_delete_namespace(const char *namespace);

#ifdef __cplusplus
}
#endif

#endif // STORAGE_SERVICE_H