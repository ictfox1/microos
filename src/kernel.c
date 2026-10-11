// Прототипы функций
void puts(char c);
char gets(void);
void puts_str(const char* str);
void clear_screen(void);
void reboot(void);
int strcmp(const char* s1, const char* s2);
char* strcpy(char* dest, const char* src);
int strlen(const char* str);
void puts_dec(int num);
void print_system_info(void);
void process_command(const char* cmd);
void gets_line(char* buffer, int max_len);
char* strchr(const char* str, int ch);
int strncmp(const char* s1, const char* s2, int n);
void poweroff(void);


// Прототипы функций файловой системы
void fs_init(void);
int fs_create(const char* filename, const char* content);
int fs_read(const char* filename, char* buffer);
int fs_write(const char* filename, const char* content);
int fs_delete(const char* filename);
void fs_list(void);
int fs_exists(const char* filename);
void fs_format(void);
int fs_get_size(const char* filename);

// Прототипы для cut
void cut_command(const char* args);
int parse_cut_range(const char* range_str, int* start, int* end, int* is_single);
void simple_cut(const char* range_str, const char* filename);
int atoi(const char* str);

// Файловая система в памяти
#define MAX_FILES 16
#define MAX_FILENAME 16
#define FILE_SIZE 1024  // Максимальный размер файла
#define NULL ((void*)0)


typedef struct {
    char name[MAX_FILENAME];
    char data[FILE_SIZE];
    int size;
    int used;  // 1 = используется, 0 = свободен
} FileEntry;

static FileEntry files[MAX_FILES];
static int file_count = 0;

// Структура для истории команд
#define HISTORY_SIZE 10
static char history[HISTORY_SIZE][64];
static int history_index = 0;
static int history_current = 0;

// Точка входа в C-код
void _start() {
    puts_str("\r\n");
    puts_str("MicroOS Kernel v0.1 - Echo Shell\r\n");
    puts_str("Type 'help' for available commands\r\n");
    
    // Инициализация файловой системы
    fs_init();
    
    // Создаем несколько тестовых файлов при первом запуске
    if (file_count == 0) {
        fs_create("readme", "Welcome to MicroOS!.\r\nType 'help' for commands.");
        fs_create("alphabet", "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    }
    
    puts_str("> ");

    char buff[128];
    int index = 0;
    char c;
    
    // Инициализация истории команд
    for (int i = 0; i < HISTORY_SIZE; i++) {
        history[i][0] = '\0';
    }
    
    // Бесконечный цикл эхо-шелла
    while (1) {
        c = gets();
        
        // Enter
        if (c == '\r') {
            puts('\r');
            puts('\n');
            buff[index] = '\0'; // Завершаем строку
            
            if (index > 0) {
                // Сохраняем в историю
                strcpy(history[history_index % HISTORY_SIZE], buff);
                history_index++;
                history_current = history_index;
                
                // Обрабатываем команду
                process_command(buff);
            }
            
            puts_str("> ");
            index = 0;
            buff[0] = '\0';
        }
        // Backspace
        else if (c == 8 && index > 0) {
            puts(8);
            puts(' ');
            puts(8);
            index--;
            buff[index] = '\0';
        }
        // предыдущая команда из истории
        else if (c == 0x55) { 
            if (history_current > 0) {
                history_current--;
                // Очищаем текущую строку
                while (index > 0) {
                    puts(8);
                    puts(' ');
                    puts(8);
                    index--;
                }
                // Загружаем команду из истории
                int hist_idx = history_current % HISTORY_SIZE;
                if (history[hist_idx][0] != '\0') {
                    puts_str(history[hist_idx]);
                    strcpy(buff, history[hist_idx]);
                    index = strlen(buff);
                }
            }
        }
        // следующая команда из истории
        else if (c == 0x44) { // Код стрелки вниз
            if (history_current < history_index) {
                history_current++;
                // Очищаем текущую строку
                while (index > 0) {
                    puts(8);
                    puts(' ');
                    puts(8);
                    index--;
                }
                // Загружаем команду из истории или пустую строку
                buff[0] = '\0';
                if (history_current < history_index) {
                    int hist_idx = history_current % HISTORY_SIZE;
                    if (history[hist_idx][0] != '\0') {
                        puts_str(history[hist_idx]);
                        strcpy(buff, history[hist_idx]);
                        index = strlen(buff);
                    }
                }
            }
        }
        // Обычные печатные символы
        else if (c >= 32 && c <= 126 && index < 127) {
            puts(c);
            buff[index++] = c;
            buff[index] = '\0';
        }
        // Tab
        else if (c == '\t') {
            puts_str("    ");
            for (int i = 0; i < 4 && index < 127; i++) {
                buff[index++] = ' ';
            }
            buff[index] = '\0';
        }
    }
}

//ФУНКЦИИ ФАЙЛОВОЙ СИСТЕМЫ

// Инициализация 
void fs_init(void) {
    for (int i = 0; i < MAX_FILES; i++) {
        files[i].used = 0;
        files[i].name[0] = '\0';
        files[i].size = 0;
        files[i].data[0] = '\0';
    }
    file_count = 0;
}

// Создание файла
int fs_create(const char* filename, const char* content) {
    if (file_count >= MAX_FILES) return -1;
    if (fs_exists(filename)) return -2;
    
    for (int i = 0; i < MAX_FILES; i++) {
        if (!files[i].used) { // ищем свободный слот
            strcpy(files[i].name, filename);
            int len = 0;
            while (content[len] != '\0' && len < FILE_SIZE - 1) {
                files[i].data[len] = content[len];
                len++;
            }
            files[i].data[len] = '\0';
            files[i].size = len;
            files[i].used = 1;
            file_count++;
            return 0;
        }
    }
    return -1;
}

// Чтение файла
int fs_read(const char* filename, char* buffer) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used && strcmp(files[i].name, filename) == 0) {
            strcpy(buffer, files[i].data);
            return files[i].size;
        }
    }
    return -1;
}

// Запись в файл
int fs_write(const char* filename, const char* content) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used && strcmp(files[i].name, filename) == 0) {
            int len = 0;
            while (content[len] != '\0' && len < FILE_SIZE - 1) {
                files[i].data[len] = content[len];
                len++;
            }
            files[i].data[len] = '\0';
            files[i].size = len;
            return 0;
        }
    }
    return fs_create(filename, content);
}

// Удаление файла
int fs_delete(const char* filename) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used && strcmp(files[i].name, filename) == 0) {
            files[i].used = 0;
            files[i].name[0] = '\0';
            files[i].size = 0;
            files[i].data[0] = '\0';
            file_count--;
            return 0;
        }
    }
    return -1;
}

// Проверка существования файла
int fs_exists(const char* filename) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used && strcmp(files[i].name, filename) == 0) {
            return 1;
        }
    }
    return 0;
}

// Получение размера файла
int fs_get_size(const char* filename) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used && strcmp(files[i].name, filename) == 0) {
            return files[i].size;
        }
    }
    return -1;
}

// Вывод списка файлов
void fs_list(void) {
    puts_str("Files in memory:\r\n");
    puts_str("================\r\n");
    
    int found = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used) {
            found++;
            puts_str("  ");
            puts_str(files[i].name);
            
            int spaces = 16 - strlen(files[i].name);
            for (int s = 0; s < spaces; s++) puts(' ');
            
            puts_str("Size: ");
            puts_dec(files[i].size);
            puts_str(" bytes\r\n");
        }
    }
    
    if (found == 0) puts_str("  No files found\r\n");
    
    puts_str("================\r\n");
    puts_str("Total: ");
    puts_dec(found);
    puts_str(" files\r\n");
}

// Форматирование файловой системы
void fs_format(void) {
    for (int i = 0; i < MAX_FILES; i++) {
        files[i].used = 0;
        files[i].name[0] = '\0';
        files[i].size = 0;
        files[i].data[0] = '\0';
    }
    file_count = 0;
    puts_str("Filesystem formatted\r\n");
}

// ФУНКЦИИ ДЛЯ CUT

// atoi
int atoi(const char* str) {
    int result = 0;
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    return result;
}

// Парсинг диапазона для cut
int parse_cut_range(const char* range_str, int* start, int* end, int* is_single) {
    *is_single = 0;
    
    // Одиночное число: N
    char* dash = strchr(range_str, '-');
    if (dash == NULL) {
        *start = atoi(range_str);
        *end = *start;
        *is_single = 1;
        return (*start > 0) ? 0 : -1;
    }
    
    // Диапазон: N-M
    if (dash > range_str && *(dash + 1) != '\0') {
        *dash = '\0';
        *start = atoi(range_str);
        *end = atoi(dash + 1);
        *dash = '-'; // Восстанавливаем строку
        return (*start > 0 && *end >= *start) ? 0 : -1;
    }
    
    // До конца: N-
    if (dash > range_str && *(dash + 1) == '\0') {
        *dash = '\0';
        *start = atoi(range_str);
        *end = -1; // До конца файла
        *dash = '-';
        return (*start > 0) ? 0 : -1;
    }
    
    // От начала: -M
    if (dash == range_str && *(dash + 1) != '\0') {
        *start = 1;
        *end = atoi(dash + 1);
        return (*end > 0) ? 0 : -1;
    }
    
    return -1;
}

// Команда cut
void cut_command(const char* args) {
    char range_str[32];
    char filename[MAX_FILENAME];
    int i = 0, j = 0;
    
    // Пропускаем пробелы
    while (*args == ' ') args++;
    
    // Извлекаем диапазон
    while (*args != ' ' && *args != '\0' && i < 31) {
        range_str[i++] = *args++;
    }
    range_str[i] = '\0';
    
    // Пропускаем пробелы до имени файла
    while (*args == ' ') args++;
    
    // Извлекаем имя файла
    while (*args != ' ' && *args != '\0' && j < MAX_FILENAME - 1) {
        filename[j++] = *args++;
    }
    filename[j] = '\0';
    
    if (strlen(range_str) == 0 || strlen(filename) == 0) {
        puts_str("Usage: cut <range> <filename>\r\n");
        puts_str("Examples:\r\n");
        puts_str("  cut 5 test.txt      - 5th character\r\n");
        puts_str("  cut 1-5 test.txt    - characters 1 to 5\r\n");
        puts_str("  cut 5- test.txt     - from 5th to end\r\n");
        puts_str("  cut -5 test.txt     - first 5 characters\r\n");
        return;
    }
    
    simple_cut(range_str, filename);
}

// Основная функция cut
void simple_cut(const char* range_str, const char* filename) {
    int start, end, is_single;
    char temp_range[32];
    
    strcpy(temp_range, range_str);
    
    if (parse_cut_range(temp_range, &start, &end, &is_single) != 0) {
        puts_str("Error: invalid range format\r\n");
        return;
    }
    
    char content[FILE_SIZE];
    int size = fs_read(filename, content);
    
    if (size < 0) {
        puts_str("File not found: ");
        puts_str(filename);
        puts_str("\r\n");
        return;
    }
    
    // Если end = -1, значит до конца файла
    if (end == -1) end = size;
    
    // Проверяем границы
    if (start > size) {
        puts_str("Error: start position beyond file size\r\n");
        return;
    }
    
    if (end > size) end = size;
     
    // Обрабатываем построчно для многострочных файлов
    int line_start = 0;
    int in_range = 0;
    int output_count = 0;
    
    for (int i = 0; i <= size; i++) {
        char c = content[i];
        
        // Конец строки или файла
        if (c == '\n' || c == '\0') {
            if (in_range) {
                puts_str("\r\n");
                output_count++;
            }
            line_start = i + 1;
            in_range = 0;
            if (c == '\0') break;
            continue;
        }
        
        // Вычисляем позицию в текущей строке
        int pos = i - line_start + 1;
        
        // Проверяем, входит ли позиция в диапазон
        if (pos >= start && pos <= end) {
            puts(c);
            in_range = 1;
        }
    }
    
    if (output_count == 0) {
        puts_str("[no output]\r\n");
    } else {
        puts_str("\r\n");
    }
}

//ОБРАБОТКА КОМАНД

void process_command(const char* cmd) {
    while (*cmd == ' ') cmd++;
    
    // Команда help
    if (strcmp(cmd, "help") == 0 || strcmp(cmd, "?") == 0) {
        puts_str("Available commands:\r\n");
        puts_str("  help               - show this help message\r\n");
        puts_str("  clear/cls          - clear the screen\r\n");
        puts_str("  reboot/reset       - reboot the system\r\n");
	puts_str("  shutdown/poweroff  - poweroff the system\r\n");
        puts_str("  echo <text>        - echo arguments\r\n");
        puts_str("  version/ver        - show kernel version\r\n");
        puts_str("  history            - show command history\r\n");
        puts_str("\r\n");
        puts_str("File system commands:\r\n");
        puts_str("  ls/dir         - list files\r\n");
        puts_str("  cat <file>     - display file contents\r\n");
        puts_str("  create <file>  - create new file\r\n");
        puts_str("  write <file>   - write to file (interactive)\r\n");
        puts_str("  del <file>     - delete file\r\n");
        puts_str("  format         - format filesystem\r\n");
        puts_str("  info <file>    - file information\r\n");
        puts_str("\r\n");
        puts_str("Text processing:\r\n");
        puts_str("  cut <range> <file> - extract characters\r\n");
        puts_str("    Examples:\r\n");
        puts_str("      cut 5 file.txt      - 5th character\r\n");
        puts_str("      cut 1-5 file.txt    - characters 1-5\r\n");
        puts_str("      cut 5- file.txt     - from 5th to end\r\n");
        puts_str("      cut -5 file.txt     - first 5 chars\r\n");
    }
    //  clear
    else if (strcmp(cmd, "clear") == 0 || strcmp(cmd, "cls") == 0) {
        clear_screen();
    }
    //  reboot
    else if (strcmp(cmd, "reboot") == 0 || strcmp(cmd, "reset") == 0) {
        puts_str("Rebooting system...\r\n");
        reboot();
    }
    //  echo
    else if (strncmp(cmd, "echo ", 5) == 0) {
        puts_str(cmd + 5);
        puts_str("\r\n");
    }
    //  version
    else if (strcmp(cmd, "version") == 0 || strcmp(cmd, "ver") == 0) {
        puts_str("MicroOS Kernel Version 0.1\r\n");
        puts_str("Build date: ");
        puts_str(__DATE__);
        puts_str(" ");
        puts_str(__TIME__);
        puts_str("\r\n");
        puts_str("16-bit Real Mode OS\r\n");
        puts_str("Filesystem: In-memory (" );
        puts_dec(file_count);
        puts_str("/");
        puts_dec(MAX_FILES);
        puts_str(" files)\r\n");
    }
    // history
    else if (strcmp(cmd, "history") == 0) {
        puts_str("Command history:\r\n");
        int start = (history_index > HISTORY_SIZE) ? history_index - HISTORY_SIZE : 0;
        for (int i = start; i < history_index; i++) {
            puts_dec(i + 1);
            puts_str(": ");
            puts_str(history[i % HISTORY_SIZE]);
            puts_str("\r\n");
        }
    }
    // ls
    else if (strcmp(cmd, "ls") == 0 || strcmp(cmd, "dir") == 0) {
        fs_list();
    }
    else if (strcmp(cmd, "shutdown") == 0 || strcmp(cmd, "poweroff") == 0) {
    puts_str("System powering off...\r\n");
    poweroff();
}
    // cat
    else if (strncmp(cmd, "cat ", 4) == 0) {
        char filename[MAX_FILENAME];
        char* filename_ptr = (char*)cmd + 4;
        int i = 0;
        while (*filename_ptr != ' ' && *filename_ptr != '\0' && i < MAX_FILENAME - 1) {
            filename[i++] = *filename_ptr++;
        }
        filename[i] = '\0';
        
        if (strlen(filename) > 0) {
            char buffer[FILE_SIZE];
            int result = fs_read(filename, buffer);
            if (result >= 0) {
                puts_str("File: ");
                puts_str(filename);
                puts_str("\r\n================\r\n");
                puts_str(buffer);
                puts_str("\r\n================\r\n");
                puts_str("Size: ");
                puts_dec(result);
                puts_str(" bytes\r\n");
            } else {
                puts_str("File not found: ");
                puts_str(filename);
                puts_str("\r\n");
            }
        } else {
            puts_str("Usage: cat <filename>\r\n");
        }
    }
    // create
    else if (strncmp(cmd, "create ", 7) == 0) {
        char filename[MAX_FILENAME];
        char* filename_ptr = (char*)cmd + 7;
        int i = 0;
        while (*filename_ptr != ' ' && *filename_ptr != '\0' && i < MAX_FILENAME - 1) {
            filename[i++] = *filename_ptr++;
        }
        filename[i] = '\0';
        
        if (strlen(filename) > 0) {
            puts_str("Enter file content (end with empty line):\r\n");
            char content[FILE_SIZE];
            content[0] = '\0';
            int content_len = 0;
            
            while (1) {
                char line[80];
                puts_str("> ");
                gets_line(line, sizeof(line));
                if (strlen(line) == 0) break;
                if (content_len + strlen(line) + 2 < FILE_SIZE) {
                    if (content_len > 0) {
			            content[content_len++] = '\r';
		    	        content[content_len++] = '\n';
		            }
                    strcpy(content + content_len, line);
                    content_len += strlen(line);
                } else {
                    puts_str("File too large! Maximum size is ");
                    puts_dec(FILE_SIZE);
                    puts_str(" bytes\r\n");
                    break;
                }
            }
            
            content[content_len] = '\0';
            int result = fs_create(filename, content);
            if (result == 0) puts_str("File created: ");
            else if (result == -2) puts_str("File already exists: ");
            else puts_str("Cannot create file. Filesystem may be full.\r\n");
            if (result == 0 || result == -2) {
                puts_str(filename);
                puts_str("\r\n");
            }
        } else {
            puts_str("Usage: create <filename>\r\n");
        }
    }
    // write
    else if (strncmp(cmd, "write ", 6) == 0) {
        char filename[MAX_FILENAME];
        char* filename_ptr = (char*)cmd + 6;
        int i = 0;
        while (*filename_ptr != ' ' && *filename_ptr != '\0' && i < MAX_FILENAME - 1) {
            filename[i++] = *filename_ptr++;
        }
        filename[i] = '\0';
        
        if (strlen(filename) > 0) {
            if (fs_exists(filename)) {
                puts_str("Enter new content for '");
                puts_str(filename);
                puts_str("' (end with empty line):\r\n");
                char content[FILE_SIZE];
                content[0] = '\0';
                int content_len = 0;
                
                while (1) {
                    char line[80];
                    puts_str("> ");
                    gets_line(line, sizeof(line));
                    if (strlen(line) == 0) break;
                    if (content_len + strlen(line) + 2 < FILE_SIZE) {
                        if (content_len > 0) {
                            content[content_len++] = '\r';
                            content[content_len++] = '\n';
                        }
                        strcpy(content + content_len, line);
                        content_len += strlen(line);
                    } else {
                        puts_str("File too large! Maximum size is ");
                        puts_dec(FILE_SIZE);
                        puts_str(" bytes\r\n");
                        break;
                    }
                }
                
                content[content_len] = '\0';
                fs_write(filename, content);
                puts_str("File updated: ");
                puts_str(filename);
                puts_str("\r\n");
            } else {
                puts_str("File not found: ");
                puts_str(filename);
                puts_str("\r\n");
            }
        } else {
            puts_str("Usage: write <filename>\r\n");
        }
    }
    // del
    else if (strncmp(cmd, "del ", 4) == 0 || strncmp(cmd, "rm ", 3) == 0) {
        char filename[MAX_FILENAME];
        char* filename_ptr;
        if (strncmp(cmd, "del ", 4) == 0) filename_ptr = (char*)cmd + 4;
        else filename_ptr = (char*)cmd + 3;
        
        int i = 0;
        while (*filename_ptr != ' ' && *filename_ptr != '\0' && i < MAX_FILENAME - 1) {
            filename[i++] = *filename_ptr++;
        }
        filename[i] = '\0';
        
        if (strlen(filename) > 0) {
            int result = fs_delete(filename);
            if (result == 0) {
                puts_str("File deleted: ");
                puts_str(filename);
                puts_str("\r\n");
            } else {
                puts_str("File not found: ");
                puts_str(filename);
                puts_str("\r\n");
            }
        } else {
            puts_str("Usage: del <filename>\r\n");
        }
    }
    // format
    else if (strcmp(cmd, "format") == 0) {
        puts_str("Are you sure you want to format the filesystem? (y/n): ");
        char c = gets();
        puts(c);
        puts('\r');
        puts('\n');
        if (c == 'y' || c == 'Y') fs_format();
        else puts_str("Format cancelled\r\n");
    }
    // info
    else if (strncmp(cmd, "info ", 5) == 0) {
        char filename[MAX_FILENAME];
        char* filename_ptr = (char*)cmd + 5;
        int i = 0;
        while (*filename_ptr != ' ' && *filename_ptr != '\0' && i < MAX_FILENAME - 1) {
            filename[i++] = *filename_ptr++;
        }
        filename[i] = '\0';
        
        if (strlen(filename) > 0) {
            if (fs_exists(filename)) {
                int size = fs_get_size(filename);
                puts_str("File: ");
                puts_str(filename);
                puts_str("\r\nSize: ");
                puts_dec(size);
                puts_str(" bytes\r\nUsed space: ");
                puts_dec(size);
                puts_str("/");
                puts_dec(FILE_SIZE);
                puts_str(" bytes\r\n");
            } else {
                puts_str("File not found: ");
                puts_str(filename);
                puts_str("\r\n");
            }
        } else {
            puts_str("Usage: info <filename>\r\n");
        }
    }
    //  cut
    else if (strncmp(cmd, "cut ", 4) == 0) {
        cut_command(cmd + 4);
    }
    else if (strcmp(cmd, "cut") == 0) {
        puts_str("Usage: cut <range> <filename>\r\n");
        puts_str("Examples:\r\n");
        puts_str("  cut 5 test.txt      - 5th character\r\n");
        puts_str("  cut 1-5 test.txt    - characters 1 to 5\r\n");
        puts_str("  cut 5- test.txt     - from 5th to end\r\n");
        puts_str("  cut -5 test.txt     - first 5 characters\r\n");
    }
    // Неизвестная команда
    else if (strlen(cmd) > 0) {
        puts_str("Unknown command: ");
        puts_str(cmd);
        puts_str("\r\n");
        puts_str("Type 'help' for available commands\r\n");
    }
}

// БАЗОВЫЕ ФУНКЦИИ 

void puts(char c) {
    __asm__ volatile (
        "mov $0x0E, %%ah\n"
        "mov %0, %%al\n"
        "mov $0x07, %%bx\n"
        "int $0x10\n"
        :
        : "r"(c)
        : "ax", "bx"
    );
}

char gets(void) {
    char res;
    __asm__ volatile (
        "mov $0x00, %%ah\n"
        "int $0x16\n"
        "mov %%al, %0\n"
        : "=r"(res)
        :
        : "ax"
    );
    return res;
}

void puts_str(const char* str) {
    while (*str) puts(*str++);
}

void clear_screen(void) {
    __asm__ volatile (
        "mov $0x06, %%ah\n"
        "xor %%al, %%al\n"
        "xor %%cx, %%cx\n"
        "mov $0x184F, %%dx\n"
        "mov $0x07, %%bh\n"
        "int $0x10\n"
        "mov $0x02, %%ah\n"
        "xor %%bh, %%bh\n"
        "xor %%dx, %%dx\n"
        "int $0x10\n"
        :
        :
        : "ax", "bx", "cx", "dx"
    );
}

void reboot(void) {
    __asm__ volatile (
        "int $0x19\n"     
        : : : "memory"
        );
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

int strncmp(const char* s1, const char* s2, int n) {
    for (int i = 0; i < n; i++) {
        if (s1[i] != s2[i]) return (unsigned char)s1[i] - (unsigned char)s2[i];
        if (s1[i] == '\0') return 0;
    }
    return 0;
}

int strlen(const char* str) {
    int len = 0;
    while (str[len] != '\0') len++;
    return len;
}

char* strcpy(char* dest, const char* src) {
    char* d = dest;
    while ((*d++ = *src++) != '\0');
    return dest;
}

//  strchr 
char* strchr(const char* str, int ch) {
    while (*str != '\0') {
        if (*str == ch) {
            return (char*)str;
        }
        str++;
    }
    return NULL;
}

void puts_dec(int num) {
    if (num < 0) { puts('-'); num = -num; }
    if (num == 0) { puts('0'); return; }
    
    char buffer[10];
    int i = 0;
    while (num > 0) { buffer[i++] = (num % 10) + '0'; num /= 10; }
    while (--i >= 0) puts(buffer[i]);
}

void gets_line(char* buffer, int max_len) {
    int index = 0;
    char c;
    
    while (1) {
        c = gets();
        if (c == '\r') {
            puts('\r');
            puts('\n');
            buffer[index] = '\0';
            break;
        }
        else if (c == 8 && index > 0) {
            puts(8);
            puts(' ');
            puts(8);
            index--;
        }
        else if (c >= 32 && c <= 126 && index < max_len - 1) {
            puts(c);
            buffer[index++] = c;
        }
    }
}

// Функция выключения
void poweroff(void) {
    
    // QEMU
    __asm__ volatile (
        "movw $0x2000, %%ax\n"
        "movw $0x604, %%dx\n"
        "outw %%ax, %%dx\n"
        ::: "ax", "dx"
    );
    
    // Дать время на обработку
    for (volatile int i = 0; i < 100000; i++);
}
