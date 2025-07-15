#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define BOLD     "\033[1m"
#define RESET    "\033[0m"

int is_directory(const char *path, const char *name) {
    char full_path[4096];
    snprintf(full_path, sizeof(full_path), "%s/%s", path, name);

    struct stat st;
    if (stat(full_path, &st) == -1) {
        return 0; // No se pudo obtener info, lo tratamos como no directorio
    }
    return S_ISDIR(st.st_mode);
}

void printchar(char d_name[]) 
{
    for (size_t i = 0; d_name[i] != '\0'; i++) {
        printf("%c\n", d_name[i]);
    }
}

int main(int argc, char *argv[]) {
    const char *path = ".";

    if (argc > 1) {
        path = argv[1];
    }

    DIR *dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        return EXIT_FAILURE;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {

        printchar(entry->d_name);
        
        if (entry->d_name[0] == '.') {
            continue; // Omitimos ocultos
        }

        if (is_directory(path, entry->d_name)) {
            printf(BOLD "%s" RESET "\n", entry->d_name);
        } else {
            printf("%s\n", entry->d_name);
        }
    }

    closedir(dir);
    return EXIT_SUCCESS;
}
