#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h> // Include ctype.h for isspace function
#define MAX_LINE_SIZE 1024


void unarchive(const char *archive_file) {
    FILE *archive = fopen(archive_file, "r");
    if (!archive) {
        printf("Failed to open %s\n", archive_file);
        return;
    }

    char directory_start[] = "#DIRECTORY_START#";
    char directory_end[] = "#DIRECTORY_END#";
    char file_start[] = "#FILE_START#";
    char file_end[] = "#FILE_END#";

    char dir_name[MAX_LINE_SIZE];

    while (fgets(dir_name, sizeof(dir_name), archive)) {
        char *new_dir_name = dir_name;
        // Get rid of leading spaces from dir_name
        while (isspace((unsigned char)*new_dir_name)) {
            new_dir_name++;
        }
        // Remove trailing newline character if present
        if (new_dir_name[strlen(new_dir_name) - 1] == '\n') {
            new_dir_name[strlen(new_dir_name) - 1] = '\0';
        }
        // Read directory name
        if (strcmp(new_dir_name, directory_start) == 0) {
            fgets(dir_name, sizeof(dir_name), archive);
            dir_name[strcspn(dir_name, "\n")] = 0;
            printf("Creating directory: %s\n", dir_name);

            // Create directory
            int result = mkdir(dir_name, 0700);
            if (result != 0) {
                printf("Failed to create directory: %s\n", dir_name);
                perror("Error message");
                return;
            }
            printf("Directory created: %s\n", dir_name); // Debug line

            // Change working directory to the created directory
            if (chdir(dir_name) != 0) {
                printf("Failed to change directory: %s\n", dir_name);
                perror("Error message");
                return;
            }
        } else if (strcmp(new_dir_name, file_start) == 0) {
            char file_name[MAX_LINE_SIZE];
            long file_size;
            // Read file name
            fgets(file_name, sizeof(file_name), archive);
            file_name[strcspn(file_name, "\n")] = 0;

            // Read file size
            fgets(dir_name, sizeof(dir_name), archive);
            sscanf(dir_name, "%ld", &file_size);

            // Read file content
            unsigned char buffer[MAX_LINE_SIZE];
            fgets(buffer, sizeof(buffer), archive);
            buffer[strcspn(buffer, "\n")] = 0;

            char full_path[MAX_LINE_SIZE * 2];
            snprintf(full_path, sizeof(full_path), "%s", file_name);
            printf("Creating file: %s\n", full_path);

            FILE *file = fopen(full_path, "wb");
            if (file) {
                fwrite(buffer, 1, file_size, file);
                fclose(file);
                printf("File created: %s\n", full_path);
            } else {
                printf("Failed to create file: %s\n", full_path);
                return;
            }
        }
    }
    fclose(archive);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <archiveFile.data>\n", argv[0]);
        return 1;
    }
    // argv[0] is ./bvUnarchive
    char *archive_file = argv[1]; // Archive file to unarchive
    unarchive(archive_file);

    return 0;
}
