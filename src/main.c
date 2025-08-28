#include "globals.h"

const char* name = "hermes";
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
                read(STDIN_FILENO, &c, 1);
                switch (c) {
                    case '[':
                        read(STDIN_FILENO, &c, 1);
                        switch (c) {
                            case UP:
                                printf("\033[A");
                                fflush(stdout);
                                break;
                            case DOWN:
                                printf("\033[B");
                                fflush(stdout);
                                break;
                            case RIGHT:
                                printf("\033[C");
                                fflush(stdout);
                                break;
                            case LEFT:
                                printf("\033[D");
                                fflush(stdout);
                                break;
                        }
                }
                break;
            case CTRL_D:
                if (buffer.len == 0) {
                    printf("\n\r");
                    disableRawMode();
                    exit(SIGINT);
                }
                break;
            case BACKSPACE:
                if (buffer.len > 0) {
                    printf("\b \b");
                    fflush(stdout);
                    buffer.chars[--buffer.len] = '\0';
                }
                break;
            case TAB:
                // buffer = handle_tab(buffer);
                // printf("\n\r%s%s", PROMPT, buffer.chars);
                // fflush(stdout);
                break;
            default:
                buffer.chars[buffer.len++] = (char)c;
                printf("\r%s", buffer.chars);
                fflush(stdout);
                break;
        }
        if (buffer.len >= BUFFER_MAX_SIZE - 1) break;
    }
    buffer.chars[buffer.len] = '\0';
    return buffer;
}

int parse_line(String line, String **out) {
    String token = { .chars = strtok(line.chars, PARSE_TOKEN_DELIM), .len = strlen(token.chars) };
    String *buffer = malloc(sizeof(String) * BUFFER_MAX_SIZE);
    if (!buffer) {
        perror(name);
        exit(1);
    }
    int i = 0;

    while (token.chars != NULL) {
        switch (token.chars[0]) {
            case '$':
                token.chars = getenv(token.chars + 1);
                break;
        }
        buffer[i].chars = token.chars;
        buffer[i].len = token.len;
        token.chars = strtok(NULL, PARSE_TOKEN_DELIM);
        i++;
    }

    *out = buffer;
    return i;
}

int main(int argc, char** argv) {
    printf("\033[2J\033[H");
    name = argv[0];

    int status;
    while (true) {
        enableRawMode();

        String line = read_line();

        printf("\n\r");
        fflush(stdout);

        String *args;
        int argc = parse_line(line, &args);
        if (argc > 0) {
            disableRawMode();
            // status = execute(args, argc);
        }

        free(line.chars);
        free(args);
    }

    return 0;
}
