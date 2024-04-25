#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h> // format of directory entries
#include <sys/stat.h>

// Function to enter the directory and create an archive
void makeArchive(const char *dir_name, FILE *archive) {
    fprintf(archive, "#DIRECTORY_START#\n%s\n#DIRECTORY_END#\n", dir_name); // Write directory name
    DIR *directory = opendir(dir_name); // Open directory stream
    if (!directory) {
        printf("Failed to open %s\n", dir_name);
        return;
    }

    struct dirent *dir_entity;
    /*      According to the Man page this is what is contained in dirent.h struct
    ino_t  d_ino       File serial number.
    char   d_name[]    Name of entry.

    The  character  array d_name is of unspecified size, but the number of bytes preceding the
    terminating null byte shall not exceed {NAME_MAX}.

    The following shall be declared as functions and may also be defined as  macros.  Function
    prototypes shall be provided.

              int            closedir(DIR *);
              DIR           *opendir(const char *);
              struct dirent *readdir(DIR *);

              int            readdir_r(DIR *restrict, struct dirent *restrict,
                                 struct dirent **restrict);

              void           rewinddir(DIR *);

              void           seekdir(DIR *, long);
              long           telldir(DIR *);
    */
    while ((dir_entity = readdir(directory)) != NULL) {
        // Exclude current and previous directories ('.' and '..')
        if (strcmp(dir_entity->d_name, ".") != 0 && strcmp(dir_entity->d_name, "..") != 0) { //d_name[] is defined by dirent as name of the entry its of type char
            // Print the current entity (file or directory)
            printf("%s/%s\n", dir_name, dir_entity->d_name);

            // Check if the entity is a directory
            struct stat fileInformation; // Properties of the file
            /*      According to the Man page this is what is contained in sys/stat.h struct
            dev_t       st_dev        ID of device containing file
            ino_t       st_ino        file serial number
            mode_t      st_mode       mode of file (see below)
            nlink_t     st_nlink      number of links to the file
            uid_t       st_uid        user ID of file
            gid_t       st_gid        group ID of file
            dev_t       st_rdev       device ID (if file is character or block special)
            off_t       st_size       file size in bytes (if file is a regular file)
            time_t      st_atime      time of last access
            time_t      st_mtime      time of last data modification
            time_t      st_ctime      time of last status change
            blksize_t   st_blksize    a filesystem-specific preferred I/O block size for this object.  In some filesystem types, this may vary from file to file
            blkcnt_t    st_blocks     number of blocks allocated for this object
            */

            /*      Also from the man page for stat
            The following macros shall be provided to test whether a file is of  the  specified  type.
            The  value  m  supplied  to the macros is the value of st_mode from a stat structure.  The
            macro shall evaluate to a non-zero value if the test is true; 0 if the test is false.

            S_ISDIR(m) - Test for a directory.

            */
            char path[1024];
            snprintf(path, sizeof(path), "%s/%s", dir_name, dir_entity->d_name);
            if (stat(path, &fileInformation) == 0 && S_ISDIR(fileInformation.st_mode)) {
                // Recursively traverse the subdirectory
                makeArchive(path, archive);
            } else {
                // If it's a file, write its metadata and contents to the archive
                FILE *file = fopen(path, "rb"); // Open file in read-binary mode
                if (file) {
                    // Write metadata: file name and size
                    fprintf(archive, "#FILE_START#\n%s\n%ld\n", dir_entity->d_name, fileInformation.st_size);

                    // Write file content to the archive file
                    char buffer[1024];
                    size_t bytes_read;
                    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
                        fwrite(buffer, 1, bytes_read, archive);
                    }

                    fprintf(archive, "#FILE_END#\n");

                    fclose(file);
                }
            }
        }
    }
    closedir(directory); // Close the directory stream
}
int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <directoryToArchive> <archiveFile.data>\n", argv[0]);
        return 1;
    }

    // argv[0] is ./bvArchive
    char *dir_name = argv[1]; // Directory to archive
    char *archive_file = argv[2]; // Archive file to create

    // Open the archive file for writing
    FILE *archive = fopen(archive_file, "wb"); // Open in write-binary mode
    if (!archive) {
        printf("Failed to create %s\n", archive_file);
        return 1;
    }

    // Start traversing the directory and create the archive
    makeArchive(dir_name, archive);

    // Close the archive file
    fclose(archive);
    return 0;
}
