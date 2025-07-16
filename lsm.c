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

int calculate_columns(int longest_name, int term_size)
{
    int columns = term_size / (longest_name + PADDING_SPACES);
    return columns > 0 ? columns : 1;
}

char *pad_string(const char *input, size_t q, char fill_char)
{
    size_t len = strlen(input);
    if (len >= q)
    {
        char *result = malloc(len + 1);
        if (!result)
            return NULL;
        strcpy(result, input);
        return result;
    }
    char *result = malloc(q + 1);
    if (!result)
        return NULL;
    strcpy(result, input);
    for (size_t i = len; i < q; i++)
    {
        result[i] = fill_char;
    }
    result[q] = '\0';
    return result;
}

char **create_array_files(DIR *dir, bool hide_dots) {
    struct dirent *entry;
    char **files = NULL;
    int size = 0;
    
    while ((entry = readdir(dir)) != NULL) {
        if(entry->d_name[0]=='.' && hide_dots) continue;
        
        char **tmp = realloc(files, (size + 1) * sizeof(char *));
        if (!tmp) {
            for (int j = 0; j < size; j++) free(files[j]);
            free(files);
            perror("realloc");
            return NULL;
        }
        files = tmp;
        
        if (entry->d_type == DT_DIR) {
            int len = strlen(entry->d_name) + 9;
            files[size] = malloc(len);
            if (!files[size]) {
                for (int j = 0; j < size; j++) free(files[j]);
                free(files);
                perror("malloc");
                return NULL;
            }
            snprintf(files[size], len, BOLD "%s" RESET, entry->d_name);
        } else {
            files[size] = strdup(entry->d_name);
            if (!files[size]) {
                for (int j = 0; j < size; j++) free(files[j]);
                free(files);
                perror("strdup");
                return NULL;
            }
        }
        size++;
    }
    return files;
}

void display_as_columns(char **files, int n_files, int columns, int longest_fname)
{
    int characters_per_column = longest_fname + PADDING_SPACES;
    int i = 0;
    while (i < n_files)
    {
        for (int j = 0; j < columns; j++)
        {
            if (i >= n_files)
                break;
            char *padded = pad_string(files[i], characters_per_column, ' ');
            if (padded)
            {
                printf("%s", padded);
                free(padded);
            }
            i++;
        }
        printf("\n");
    }
}

void display_as_list(char **files, int n_files)
{
    for (int i = 0; i < n_files; i++)
    {
        printf("%s\n", files[i]);
    }
}

bool is_directory(const char *path)
{
    DIR *dir = opendir(path);
    if (dir == NULL)
    {
        perror("lsm");
        return false;
    }
    closedir(dir);

    return true;
}

bool is_flag(const char *flag)
{
    if (strlen(flag) <= 1)
    {
        return false;
    }

    if (flag[0] != '-')
    {
        return false;
    }

    for (int i = 1; flag[i] != '\0'; i++)
    {
        if (strchr(VALID_FLAGS, flag[i]) == NULL)
        {
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

    DIR *dir = opendir(path);
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

    rewinddir(dir);
    char **files = create_array_files(dir, true);
    closedir(dir);

    if (!files) {
        return EXIT_FAILURE;
    }

    int term_size = get_term_size();
    int columns = calculate_columns(longest_name, term_size);

    display_as_list(files, n_files);

    for (int i = 0; i < n_files; i++) {
        free(files[i]);
    }
    free(files);

    return EXIT_SUCCESS;
}
