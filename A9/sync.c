/*
Name: Sanskar Mittal and Karthik Reddy
Roll Number: 21CS10057 and 21CS30058
Semester: 6
Lab Assignment: 9 : Synchronizing two directory trees
File: sync.c
*/
#define _GNU_SOURCE
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>

void deleteDirectory(char *path) {
    DIR *dir;
    struct dirent *entry;
    char fullpath[1000];
    dir = opendir(path);
    if (dir == NULL) {
        printf("Error: Unable to open directory: %s\n", path);
        return;
    }
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        memset(fullpath, '\0', 1000);
        sprintf(fullpath, "%s/%s", path, entry->d_name);
        if (entry->d_type == DT_DIR) {
            deleteDirectory(fullpath);
        } else {
            remove(fullpath);
            printf("[-] %s\n", fullpath);
        }
    }
    closedir(dir);
    rmdir(path);
    printf("[-] %s\n", path);
}

void synchronise(char *src, char *dst) {
    DIR *src_dir, *dst_dir;
    struct dirent *src_entry, *dst_entry;

    // Open the source directory
    if ((src_dir = opendir(src)) == NULL) {
        printf("Error: Unable to open source directory: %s\n", src);
        return;
    }

    // Open the destination directory
    if ((dst_dir = opendir(dst)) == NULL) {
        printf("Error: Unable to open destination directory: %s\n", dst);
        return;
    }
    
    // Delete files in destination which are not present in source
    while ((dst_entry = readdir(dst_dir)) != NULL) {
        // Ignore the current and parent directory
        if (strcmp(dst_entry->d_name, ".") == 0 || strcmp(dst_entry->d_name, "..") == 0) {
            continue;
        }

        src_entry = NULL;   // Reset the source entry
        rewinddir(src_dir); // Reset the source directory pointer

        // Create the full path of the file
        char src_file[1000], dst_file[1000];
        memset(src_file, '\0', 1000);
        memset(dst_file, '\0', 1000);
        sprintf(dst_file, "%s/%s", dst, dst_entry->d_name);

        // Check if the file is regular
        if (dst_entry->d_type == DT_REG) {
            // Go through all files of the source directory
            while ((src_entry = readdir(src_dir)) != NULL) {
                // Check if both source and destination file have same name and type
                if (src_entry->d_type == DT_REG) {
                    if (strcmp(src_entry->d_name, dst_entry->d_name) == 0) {
                        break;
                    }
                }
            }
            // If the file is not present in the source, delete it
            if (src_entry == NULL) {
                remove(dst_file);
                printf("[-] %s\n", dst_file);
            }

        } else if (dst_entry->d_type == DT_DIR) { // Check if the file is a directory
            // Go through all files of the source directory
            while ((src_entry = readdir(src_dir)) != NULL) {
                // Check if both source and destination file have same name
                if (src_entry->d_type == DT_DIR) {
                    if (strcmp(src_entry->d_name, dst_entry->d_name) == 0) {
                        break;
                    }
                }
            }

            // If the directory is not present in the source, delete it
            if (src_entry == NULL) {
                // Recursively delete the directory content
                deleteDirectory(dst_file);
            }
        }
    }

    // Reset the directory pointers
    rewinddir(src_dir);
    rewinddir(dst_dir);

    // Go through all files in the source directory
    while ((src_entry = readdir(src_dir)) != NULL) {
        // Ignore the current and parent directory
        if (strcmp(src_entry->d_name, ".") == 0 || strcmp(src_entry->d_name, "..") == 0) {
            continue;
        }
        // Create the full path of the file
        char src_file[1000], dst_file[1000];
        memset(src_file, '\0', 1000);
        memset(dst_file, '\0', 1000);
        sprintf(src_file, "%s/%s", src, src_entry->d_name);
        rewinddir(dst_dir); // Reset the destination directory pointer

        // Check if the file is regular
        if (src_entry->d_type == DT_REG) {
            struct stat src_stat, dst_stat;
            if (stat(src_file, &src_stat) == -1) {
                perror("Error: Unable to obtain attributes of source file");
                continue;
            }
            // Go through all files of the destination directory
            while ((dst_entry = readdir(dst_dir)) != NULL) {
                // Check if both source and destination file have same name
                if (dst_entry->d_type == DT_REG) {
                    if (strcmp(src_entry->d_name, dst_entry->d_name) == 0) {
                        memset(dst_file, '\0', 1000);
                        sprintf(dst_file, "%s/%s", dst, dst_entry->d_name);
                        if (stat(dst_file, &dst_stat) == -1) {
                            printf("Error: Unable to obtain attributes of destination file: %s\n", dst_file);
                            continue;
                        }
                        // If file size and modification timestamp is same
                        if (src_stat.st_size == dst_stat.st_size) {
                            if (src_stat.st_mtime == dst_stat.st_mtime) {
                                break;
                            }
                        }
                    }
                }
            }
            // Copy source to destination
            if (dst_entry == NULL) {
                memset(dst_file, '\0', 1000);
                sprintf(dst_file, "%s/%s", dst, src_entry->d_name);

                // Check if destination file exists already
                if (access(dst_file, F_OK) != -1) {
                    printf("[o] %s\n", dst_file); 
                } else {
                    printf("[+] %s\n", dst_file);
                }

                int src_fd = open(src_file, O_RDONLY);
                int dst_fd = open(dst_file, O_WRONLY | O_CREAT | O_TRUNC, 0777);
                off_t ret, len;
                len = src_stat.st_size;
                do {
                    ret = copy_file_range(src_fd, NULL, dst_fd, NULL, len, 0);
                    if (ret == -1) {
                        perror("copy_file_range");
                        exit(EXIT_FAILURE);
                    }

                    len -= ret;
                } while (len > 0 && ret > 0);
                close(src_fd);
                close(dst_fd);
            }

        } else if (src_entry->d_type == DT_DIR) { // Check if the file is a directory
            // Go through all files of the destination directory
            while ((dst_entry = readdir(dst_dir)) != NULL) {
                // Check if both source and destination file have same name
                if (dst_entry->d_type == DT_DIR) {
                    if (strcmp(src_entry->d_name, dst_entry->d_name) == 0) {
                        memset(dst_file, '\0', 1000);
                        sprintf(dst_file, "%s/%s", dst, dst_entry->d_name);
                        // Recursively call the function for the subdirectories
                        synchronise(src_file, dst_file);
                        break;
                    }
                }
            }

            // If the directory is not present in the destination, create it
            if (dst_entry == NULL) {
                memset(dst_file, '\0', 1000);
                sprintf(dst_file, "%s/%s", dst, src_entry->d_name);
                mkdir(dst_file, 0777);
                printf("[+] %s\n", dst_file);
                synchronise(src_file, dst_file);
            }
        }
    }

    closedir(src_dir);
    closedir(dst_dir);
}

void changePermAndTime(char *src, char *dst) {
    DIR *src_dir, *dst_dir;
    struct dirent *src_entry, *dst_entry;

    // Open the source directory
    if ((src_dir = opendir(src)) == NULL) {
        printf("Error: Unable to open source directory: %s\n", src);
        return;
    }

    // Open the destination directory
    if ((dst_dir = opendir(dst)) == NULL) {
        printf("Error: Unable to open destination directory: %s\n", dst);
        return;
    }

    // Change timestamps and permissions of destination files
    while ((src_entry = readdir(src_dir)) != NULL) {
        if (strcmp(src_entry->d_name, ".") == 0 || strcmp(src_entry->d_name, "..") == 0) {
            continue;
        }

        char src_file[1000], dst_file[1000];
        memset(src_file, '\0', 1000);
        memset(dst_file, '\0', 1000);
        sprintf(src_file, "%s/%s", src, src_entry->d_name);

        rewinddir(dst_dir); // Reset the destination directory pointer

        if (src_entry->d_type == DT_REG) {
            struct stat src_stat, dst_stat;
            if (stat(src_file, &src_stat) == -1) {
                perror("Error: Unable to obtain attributes of source file");
                continue;
            }

            while ((dst_entry = readdir(dst_dir)) != NULL) {
                if (dst_entry->d_type == DT_REG) {
                    if (strcmp(src_entry->d_name, dst_entry->d_name) == 0) {
                        memset(dst_file, '\0', 1000);
                        sprintf(dst_file, "%s/%s", dst, dst_entry->d_name);
                        if (stat(dst_file, &dst_stat) == -1) {
                            printf("Error: Unable to obtain attributes of destination file: %s\n", dst_file);
                            continue;
                        }

                        // Check if file permissions are same for source and destination
                        if (src_stat.st_mode != dst_stat.st_mode) {
                            if (chmod(dst_file, src_stat.st_mode) == -1) {
                                perror("Error: Unable to change permissions of destination file");
                            }
                            printf("[p] %s\n", dst_file);
                        }

                        struct utimbuf src_time;
                        src_time.modtime = src_stat.st_mtime;

                        // Check if file timestamps are same for source and destination
                        if (src_stat.st_mtime != dst_stat.st_mtime) {
                            if (utime(dst_file, &src_time) == -1) {
                                perror("Error: Unable to change timestamps of destination file");
                            }
                            printf("[t] %s\n", dst_file);
                        }
                        break;
                    }
                }
            }
        } else if (src_entry->d_type == DT_DIR) {
            struct stat src_stat, dst_stat;
            if (stat(src_file, &src_stat) == -1) {
                perror("Error: Unable to obtain attributes of source directory");
                continue;
            }
            while ((dst_entry = readdir(dst_dir)) != NULL) {
                if (dst_entry->d_type == DT_DIR) {
                    if (strcmp(src_entry->d_name, dst_entry->d_name) == 0) {
                        memset(dst_file, '\0', 1000);
                        sprintf(dst_file, "%s/%s", dst, dst_entry->d_name);
                        if (stat(dst_file, &dst_stat) == -1) {
                            printf("Error: Unable to obtain attributes of destination file: %s\n", dst_file);
                            continue;
                        }

                        // Check if directory permissions are same for source and destination
                        if (src_stat.st_mode != dst_stat.st_mode) {
                            if (chmod(dst_file, src_stat.st_mode) == -1) {
                                perror("Error: Unable to change permissions of destination file");
                            }
                            printf("[p] %s\n", dst_file);
                        }

                        struct utimbuf src_time;
                        src_time.actime = src_stat.st_atime;
                        src_time.modtime = src_stat.st_mtime;

                        // Check if directory timestamps are same for source and destination
                        if (src_stat.st_mtime != dst_stat.st_mtime) {
                            if (utime(dst_file, &src_time) == -1) {
                                perror("Error: Unable to change timestamps of destination file");
                            }
                            printf("[t] %s\n", dst_file);
                        }
                        changePermAndTime(src_file, dst_file);
                        break;
                    }
                }
            }
        }
    }
    closedir(src_dir);
    closedir(dst_dir);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: ./sync <src> <dst>\n");
        return 1;
    }

    char *src = argv[1];
    char *dst = argv[2];

    synchronise(src, dst);
    changePermAndTime(src, dst);

    return 0;
}