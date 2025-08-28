#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <dirent.h>
#include <stdbool.h>
#include <errno.h>

#define PARSE_TOKEN_DELIM " \t"
#define HERMES_SUCCESS 1
#define HERMES_FAILURE 0

typedef const enum sizes {
    BUFFER_MAX_SIZE = 1024,
    MAX_LINE = 512,
} sizes_t;

typedef enum chars {
    CTRL_D = 4,
    TAB = 9,
    ENTER = 13,
    ESCAPE = 27,
    UP = 65,
    DOWN = 66,
    RIGHT = 67,
    LEFT = 68,
    BACKSPACE = 127,
} chars_t;

typedef struct String {
    char* chars;
    int len;
} String;

const char* name = "calc";
struct termios orig_termios;

void disableRawMode(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode(void) {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG);
    raw.c_iflag &= ~(IXON | ICRNL);        
    raw.c_oflag &= ~(OPOST);               

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

String read_line(void) {
    chars_t c;
    String buffer = { .chars = calloc(BUFFER_MAX_SIZE, sizeof(char)), .len = 0};
    if (!buffer.chars) {
        perror(name);
        exit(1);
    }

    while (read(STDIN_FILENO, &c, 1) == 1 && c != ENTER) {
        switch (c) {
            case ESCAPE:
                exit(EXIT_SUCCESS);
            case BACKSPACE:
                printf("\b \b");
                fflush(stdout);
                break;
            default:
                putc(c, stdout);
                fflush(stdout);
                break;
        }
        if (buffer.len >= BUFFER_MAX_SIZE - 1) break;
    }
    buffer.chars[buffer.len] = '\0';
    return buffer;
}

int main(int argc, char** argv) {
    name = argv[0];

    int status;
    while (true) {

        enableRawMode();
        String line = read_line();
        disableRawMode();

        free(line.chars);
    }

    return 0;
}
