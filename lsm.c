#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <limits.h>
#include <stdbool.h>

#define BOLD "\033[1m"
#define RESET "\033[0m"
#define VALID_FLAGS "la"
#define PADDING_SPACES 4
#define DEFAULT_FILLER ' '

int get_term_size()
{
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1)
    {
        perror("ioctl");
        return 80; // Default terminal width instead of returning 1
    }
    int term_size = w.ws_col;
    return term_size;
}

int calculate_columns(int longest_name) {
    int columns = get_term_size() / (longest_name + PADDING_SPACES);
    return columns > 0 ? columns : 1;
}

char *pad_string(const char *input, size_t q, char fill_char) {
    size_t len = strlen(input);
    if (len >= q) {
        char *result = malloc(len + 1);
        if (!result)
            return NULL;
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

char **create_array_files(const char *path, bool hide_dots, int file_arr_size, int size_column) {
    DIR *dir = opendir(path);
    struct dirent *entry;
    char **files = malloc(file_arr_size * sizeof(char *));
    if (!files) return NULL;
    int i = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (hide_dots && entry->d_name[0] == '.') continue;
        // files[i] = strdup(entry->d_name);
        files[i] = pad_string(entry->d_name, size_column, DEFAULT_FILLER);
        i++;
    }
    while (i < file_arr_size) {
        // files[i] = strdup(" ");
        files[i] = pad_string(" ", size_column, DEFAULT_FILLER);
        i++;
    }
    closedir(dir);

    return files;
}

void display_as_columns(const char *path, bool hide_dots) {
    DIR *dir = opendir(path);
    struct dirent *entry;
    int longest_fn = 0;
    int n_files = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (hide_dots && entry->d_name[0] == '.') continue;
        if (strlen(entry->d_name) > longest_fn) {
            longest_fn = strlen(entry->d_name);
        }
        n_files++;
    }

    int n_columns = calculate_columns(longest_fn);
    int files_per_column = n_files % n_columns > 0 ? (n_files / n_columns) + 1: n_files / n_columns;
    int column_size = longest_fn + PADDING_SPACES;
    char **files = create_array_files(path, hide_dots, files_per_column * n_columns, column_size);

    for(int i = 0; i < files_per_column; i++) {
        for (int j = 0; j < n_columns; j++) {
            printf("%s ", files[i + j * files_per_column]);
        }
        printf("\n");
    }

    closedir(dir);
}

void display_as_list(char **files, int n_files) {
    for (int i = 0; i < n_files; i++) {
        printf("%s\n", files[i]);
    }
}

bool is_directory(const char *path) {
    struct stat statbuf;
    
    if (stat(path, &statbuf) != 0) {
        return false;  // Path doesn't exist or can't be accessed
    }
    
    return S_ISDIR(statbuf.st_mode);
}

bool is_flag(const char *flag) {
    if (strlen(flag) <= 1) return false;
    if (flag[0] != '-') return false; 
    for (int i = 1; flag[i] != '\0'; i++) {
        if (strchr(VALID_FLAGS, flag[i]) == NULL) { 
            return false;
        } 
    }

    return true;
}

int main(int argc, char *argv[]) {
    const char *path;
    if (argc > 1) {
        if (is_flag(argv[argc-1])) {
            path = ".";
        } else if (is_directory(argv[argc-1])) {
            path = argv[argc-1];
        } else {
            return EXIT_FAILURE;
        }
    } else {
        path = ".";
    }

    bool hide_dots = true;
    display_as_columns(path, hide_dots);

    return EXIT_SUCCESS;
}
