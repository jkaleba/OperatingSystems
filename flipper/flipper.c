#define _GNU_SOURCE
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>


void flip_line(const char* line, char* flipped) {
    int len = strlen(line);
    for (int i = 0; i < len; i++) {
        flipped[i] = line[len - i - 1];
    }
    flipped[len] = '\0';
}


void process_file(const char* sourcePath, const char* destPath) {
    FILE* src = fopen(sourcePath, "r");
    if (src == NULL) {
        perror("Error opening source file");
        return;
    }

    FILE* dest = fopen(destPath, "w");
    if (dest == NULL) {
        perror("Error opening destination file");
        fclose(src);
        return;
    }

    char line[1024];
    while (fgets(line, sizeof(line), src) != NULL) {
        line[strcspn(line, "\n")] = '\0';
        char flipped[1024];
        flip_line(line, flipped);
        fprintf(dest, "%s\n", flipped);
    }

    fclose(src);
    fclose(dest);
}


void process_directory(const char* source_dir, const char* dest_dir) {
    DIR* dir = opendir(source_dir);
    if (dir == NULL) {
        perror("Error opening directory");
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {

            const char* ext = strrchr(entry->d_name, '.');

            if (ext && strcmp(ext, ".txt") == 0) {
                char source_path[1024];
                snprintf(source_path, sizeof(source_path), "%s/%s", source_dir, entry->d_name);

                char dest_path[1024];
                snprintf(dest_path, sizeof(dest_path), "%s/%s", dest_dir, entry->d_name);

                process_file(source_path, dest_path);
            }
        }
    }

    closedir(dir);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source directory> <destination directory>\n", argv[0]);
        return 1;
    }

    const char* source_dir = argv[1];
    const char* dest_dir = argv[2];

    mkdir(dest_dir, 0777);
    process_directory(source_dir, dest_dir);

    return 0;
}
