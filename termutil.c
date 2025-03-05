// INCLUDES
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdbool.h>


// DEFINES
#define CTRL_KEY(k) ((k) & 0x1f)
#ifndef thread_local
#define thread_local _Thread_local
#endif

#define TU_DUMB          0
#define TU_CDPGLEGACY    1
#define TU_UTF8LEGACY    2
#define TU_UTF8NOLIGS    3
#define TU_FULLUTF8APPLE 4
#define TU_FULLUTF8      5
#define TU_FULLUTF82027  6

static const char *TERMTYPES[] = {
    "Dumb Terminal",
    "Legacy Terminal w/ Code Page",
    "Legacy Terminal w/ UTF-8 Support",
    "Terminal w/ Basic UTF-8 Support (no grapheme clustering)",
    "Terminal w/ Full UTF-8 Support (Apple Terminal quirks)",
    "Terminal w/ Full UTF-8 Support (no Mode 2027 support)",
    "Terminal w/ Full UTF-8 Support and Mode 2027 support",
};

void TUApplyRawMode(bool enable) {
    static thread_local struct termios rawmode;
    static thread_local struct termios normalmode;
    
    // First run
    if (rawmode.c_iflag == 0) {
        tcgetattr(STDIN_FILENO, &normalmode);
        rawmode = normalmode;
        rawmode.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
        rawmode.c_oflag &= ~(OPOST);
        rawmode.c_cflag |= (CS8);
        rawmode.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
        rawmode.c_cc[VMIN] = 0;
        rawmode.c_cc[VTIME] = 1;
    }
    
    if (enable) tcsetattr(STDIN_FILENO, TCSAFLUSH, &rawmode);
    else        tcsetattr(STDIN_FILENO, TCSAFLUSH, &normalmode);
}

size_t TUGetTermTextLength(const char * const text) {
    static const char * const LINESTART = "\033[0G";
    static const char * const SAVECURSOR = "\033[s";
    static const char * const RESTORECURSOR = "\033[u";
    static const char * const MAKEHIDDEN = "\033[8m";
    static const char * const UNMAKEHIDDEN = "\033[28m";
    static const char * const GETCURSORPOS = "\033[6n";
    size_t row;
    size_t col;


    write(STDOUT_FILENO, LINESTART, strlen(LINESTART));
    write(STDOUT_FILENO, MAKEHIDDEN, strlen(MAKEHIDDEN));
    write(STDOUT_FILENO, text, strlen(text));
    write(STDOUT_FILENO, UNMAKEHIDDEN, strlen(UNMAKEHIDDEN));
    write(STDOUT_FILENO, GETCURSORPOS, strlen(GETCURSORPOS));
    write(STDOUT_FILENO, LINESTART, strlen(LINESTART));
    fflush(stdout);

    scanf("\033[%zu;%zuR", &row, &col);

    return col - 1;    
}


// INIT
int main() {
    unsigned char c = '\0';
    char   buffer[128];
    size_t length; 

    TUApplyRawMode(true);

    strcpy(buffer, u8"Hello, World!");
    length = TUGetTermTextLength(buffer);
    printf("Length of \'%s\' is %zu\r\n", buffer, length);

    strcpy(buffer, u8"﷽           ");
    length = TUGetTermTextLength(buffer);
    printf("Length of \'%s\' is %zu\r\n", buffer, length);

    strcpy(buffer, u8"🧑‍🌾");
    length = TUGetTermTextLength(buffer);
    printf("Length of \'%s\' is %zu\r\n", buffer, length);

    strcpy(buffer, u8"🧑");
    length = TUGetTermTextLength(buffer);
    printf("Length of \'%s\' is %zu\r\n", buffer, length);

    strcpy(buffer, u8"🌾");
    length = TUGetTermTextLength(buffer);
    printf("Length of \'%s\' is %zu\r\n", buffer, length);
    

    while (1) {
        if (read(STDIN_FILENO, &c, 1) == -1) {
            switch (errno) {
                case EAGAIN:
                    break;
                default:
                    perror("read");
                    exit(EXIT_FAILURE);
            }
        }
        if (c == 0) continue;
        if (c == CTRL_KEY('q')) break;
        if (iscntrl(c)) printf("%d\r\n", c);
        else printf("%d ('%c')\r\n", c, c);
        c = 0;
    }

    TUApplyRawMode(false);

    return 0;
}
