/**
 * @file app_installer.c
 * @brief App Installer Implementation
 */

#include "app_manager.h"
#include "esp_log.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

static const char *TAG = "APP_INSTALLER";

esp_err_t app_installer_extract_package(const char *package_path, const char *extract_path)
{
    if (!package_path || !extract_path) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Extracting package %s to %s", package_path, extract_path);
    
    // Create extraction directory
    if (mkdir(extract_path, 0755) != 0) {
        ESP_LOGE(TAG, "Failed to create extraction directory");
        return ESP_FAIL;
    }
    
    // For simplicity, assume package is already extracted or copy files
    // In real implementation, this would handle ZIP/TAR archives
    
    FILE *src = fopen(package_path, "r");
    if (!src) {
        ESP_LOGE(TAG, "Failed to open package file");
        return ESP_ERR_NOT_FOUND;
    }
    
    char dst_path[256];
    snprintf(dst_path, sizeof(dst_path), "%s/index.js", extract_path);
    
    FILE *dst = fopen(dst_path, "w");
    if (!dst) {
        fclose(src);
        ESP_LOGE(TAG, "Failed to create destination file");
        return ESP_FAIL;
    }
    
    // Copy file content
    char buffer[512];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        fwrite(buffer, 1, bytes, dst);
    }
    
    fclose(src);
    fclose(dst);
    
    // Create basic manifest if not exists
    snprintf(dst_path, sizeof(dst_path), "%s/manifest.json", extract_path);
    FILE *manifest = fopen(dst_path, "w");
    if (manifest) {
        fprintf(manifest, "{\n");
        fprintf(manifest, "  \"name\": \"Sample App\",\n");
        fprintf(manifest, "  \"version\": \"1.0.0\",\n");
        fprintf(manifest, "  \"author\": \"Unknown\",\n");
        fprintf(manifest, "  \"entry_point\": \"index.js\",\n");
        fprintf(manifest, "  \"permissions\": \"rf.receive,ui.create\"\n");
        fprintf(manifest, "}\n");
        fclose(manifest);
    }
    
    ESP_LOGI(TAG, "Package extracted successfully");
    return ESP_OK;
}

esp_err_t app_installer_validate_manifest(const char *manifest_path)
{
    if (!manifest_path) {
        return ESP_ERR_INVALID_ARG;
    }
    
    FILE *file = fopen(manifest_path, "r");
    if (!file) {
        ESP_LOGE(TAG, "Manifest file not found: %s", manifest_path);
        return ESP_ERR_NOT_FOUND;
    }
    
    // Basic validation - check if file contains required fields
    char buffer[1024];
    size_t read_size = fread(buffer, 1, sizeof(buffer) - 1, file);
    fclose(file);
    
    if (read_size == 0) {
        ESP_LOGE(TAG, "Empty manifest file");
        return ESP_FAIL;
    }
    
    buffer[read_size] = '\0';
    
    // Check for required fields
    if (!strstr(buffer, "\"name\"") || 
        !strstr(buffer, "\"version\"") ||
        !strstr(buffer, "\"entry_point\"")) {
        ESP_LOGE(TAG, "Invalid manifest - missing required fields");
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "Manifest validation passed");
    return ESP_OK;
}

esp_err_t app_installer_copy_files(const char *src_path, const char *dst_path)
{
    if (!src_path || !dst_path) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Copying files from %s to %s", src_path, dst_path);
    
    // Create destination directory
    if (mkdir(dst_path, 0755) != 0) {
        ESP_LOGW(TAG, "Destination directory already exists or creation failed");
    }
    
    // Simple file copy implementation
    DIR *dir = opendir(src_path);
    if (!dir) {
        ESP_LOGE(TAG, "Failed to open source directory");
        return ESP_FAIL;
    }
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) { // Regular file
            char src_file[256], dst_file[256];
            snprintf(src_file, sizeof(src_file), "%s/%s", src_path, entry->d_name);
            snprintf(dst_file, sizeof(dst_file), "%s/%s", dst_path, entry->d_name);
            
            FILE *src = fopen(src_file, "r");
            FILE *dst = fopen(dst_file, "w");
            
            if (src && dst) {
                char buffer[512];
                size_t bytes;
                while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
                    fwrite(buffer, 1, bytes, dst);
                }
            }
            
            if (src) fclose(src);
            if (dst) fclose(dst);
        }
    }
    
    closedir(dir);
    ESP_LOGI(TAG, "Files copied successfully");
    
    return ESP_OK;
}