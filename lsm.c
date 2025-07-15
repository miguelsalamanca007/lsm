#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <limits.h>

#define BOLD "\033[1m"
#define RESET "\033[0m"

int is_directory(const char *path, const char *name) {
    char full_path[PATH_MAX];
    snprintf(full_path, sizeof(full_path), "%s/%s", path, name);
    struct stat st;
    if (stat(full_path, &st) == -1) {
        return 0; // Could not get info, treat as non-directory
    }
    return S_ISDIR(st.st_mode);
}

int get_term_size() {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
        perror("ioctl");
        return 80; // Default terminal width instead of returning 1
    }
    int term_size = w.ws_col;
    return term_size;
}

int calculate_columns(int longest_name, int term_size) {
    int columns = term_size / (longest_name + 6);
    return columns > 0 ? columns : 1; // Ensure at least 1 column
}

void printchar(char *d_name) {
    for (size_t i = 0; d_name[i] != '\0'; i++) {
        printf("%c\n", d_name[i]);
    }
}

char *pad_string(const char *input, size_t q, char fill_char) {
    size_t len = strlen(input);
    if (len >= q) {
        char *result = malloc(len + 1);
        if (!result) return NULL;
        strcpy(result, input);
        return result;
    }
    char *result = malloc(q + 1);
    if (!result) return NULL;
    strcpy(result, input);
    for (size_t i = len; i < q; i++) {
        result[i] = fill_char;
    }
    result[q] = '\0';
    return result;
}

char **create_array_files(DIR *dir, int n_files) {
    struct dirent *entry;
    char **files = malloc(sizeof(char*) * n_files);
    if (!files) {
        perror("malloc");
        return NULL;
    }
    
    int i = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        files[i] = strdup(entry->d_name);
        if (!files[i]) {
            // Clean up on failure
            for (int j = 0; j < i; j++) {
                free(files[j]);
            }
            free(files);
            perror("strdup");
            return NULL;
        }
        i++;
    }
    return files;
}

void display_ls(char **files, int n_files, int columns, int longest_fname) {
    int characters_per_column = longest_fname + 6;
    int i = 0;
    while (i < n_files) {
        for (int j = 0; j < columns; j++) {
            if (i >= n_files) break;
            char *padded = pad_string(files[i], characters_per_column, ' ');
            if (padded) {
                printf("%s", padded);
                free(padded);
            }
            i++;
        }
        printf("\n");
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
    long longest_name = 0;
    long n_files = 0;
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        n_files++;
        if (strlen(entry->d_name) > longest_name) {
            longest_name = strlen(entry->d_name);
        }
    }
    
    // Reset directory stream instead of closing and reopening
    rewinddir(dir);
    
    char **files = create_array_files(dir, n_files);
    closedir(dir);
    
    if (!files) {
        return EXIT_FAILURE;
    }
    
    int term_size = get_term_size();
    int columns = calculate_columns(longest_name, term_size);
    
    display_ls(files, n_files, columns, longest_name);
    
    // Clean up allocated memory
    for (int i = 0; i < n_files; i++) {
        free(files[i]);
    }
    free(files);
    
    return EXIT_SUCCESS;
}