#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

// Структура для хранения информации о файле
typedef struct {
    char name[1024];  // Имя файла
    off_t size;       // Размер файла
    time_t mtime;     // Время последней модификации
    mode_t mode;      // Права доступа
    ino_t inode;      // Номер индексного дескриптора
} FileInfo;

// Функция для рекурсивного обхода каталога
void traverse_directory(const char *dir_path, FileInfo *files, int *count) {
    DIR *dir = opendir(dir_path);
    if (!dir) {
        perror("Ошибка открытия каталога");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Пропускаем специальные записи . и ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Формируем полный путь к файлу
        char full_path[2048];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        // Получаем информацию о файле
        struct stat st;
        if (stat(full_path, &st) == -1) {
            perror("Ошибка получения информации о файле");
            continue;
        }

        // Если это каталог, рекурсивно обходим его
        if (S_ISDIR(st.st_mode)) {
            traverse_directory(full_path, files, count);
        }
        // Если это файл, сохраняем информацию
        else if (S_ISREG(st.st_mode)) {
            strcpy(files[*count].name, full_path);
            files[*count].size = st.st_size;
            files[*count].mtime = st.st_mtime;
            files[*count].mode = st.st_mode;
            files[*count].inode = st.st_ino;
            (*count)++;
        }
    }

    closedir(dir);
}

// Функция для сравнения содержимого файлов
int compare_files(const char *file1, const char *file2) {
    FILE *f1 = fopen(file1, "rb");
    FILE *f2 = fopen(file2, "rb");
    if (!f1 || !f2) {
        perror("Ошибка открытия файла");
        return 0;
    }

    int result = 1;
    char ch1, ch2;
    while ((ch1 = fgetc(f1)) != EOF && (ch2 = fgetc(f2)) != EOF) {
        if (ch1 != ch2) {
            result = 0;
            break;
        }
    }

    // Проверяем, что оба файла закончились одновременно
    if (fgetc(f1) != EOF || fgetc(f2) != EOF) {
        result = 0;
    }

    fclose(f1);
    fclose(f2);
    return result;
}

// Функция для вывода информации о файле
void print_file_info(FileInfo *file, FILE *output) {
    fprintf(output, "Имя: %s\n", file->name);
    fprintf(output, "Размер: %ld байт\n", file->size);
    fprintf(output, "Дата создания: %s", ctime(&file->mtime));
    fprintf(output, "Права доступа: %o\n", file->mode);
    fprintf(output, "Индексный дескриптор: %lu\n", file->inode);
    fprintf(output, "----------------------------\n");
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Использование: %s <каталог1> <каталог2> <выходной_файл>\n", argv[0]);
        return 1;
    }

    // Массивы для хранения информации о файлах
    FileInfo files1[1000], files2[1000];
    int count1 = 0, count2 = 0;

    // Рекурсивно обходим оба каталога
    traverse_directory(argv[1], files1, &count1);
    traverse_directory(argv[2], files2, &count2);

    // Открываем файл для вывода результатов
    FILE *output = fopen(argv[3], "w");
    if (!output) {
        perror("Ошибка открытия выходного файла");
        return 1;
    }

    // Сравниваем файлы
    for (int i = 0; i < count1; i++) {
        for (int j = 0; j < count2; j++) {
            if (files1[i].size == files2[j].size && compare_files(files1[i].name, files2[j].name)) {
                printf("Найдены совпадающие файлы:\n");
                print_file_info(&files1[i], stdout);
                print_file_info(&files2[j], stdout);

                fprintf(output, "Совпадающие файлы:\n");
                print_file_info(&files1[i], output);
                print_file_info(&files2[j], output);
            }
        }
    }

    fclose(output);
    printf("Результаты сохранены в файл: %s\n", argv[3]);
    return 0;
}
