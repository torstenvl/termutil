#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>


// We use anonymous internal unions and structs to make an efficient TUInfo 
// structure.  As a result, we need C11 or higher or GNU/Clang extensions.
#if !(__STDC_VERSION__ >= 201112L)                      && \
    !(__GNUC__ >= 4 && !defined(__STRICT_ANSI__))       && \
    !(__clang_major__ >= 2 && !defined(__STRICT_ANSI__)) 
#error 
This library requires C11 or higher or GNU/Clang extensions to support anony-\n\
mous internal unions and structs. Please use a newer compiler or check your\n\
compiler settings to ensure extensions are enabled.
#endif


struct TUInfo {
    int errorcode; // Cf. errno
    struct {
        struct termios origattr;
        struct termios rawattr;
        unsigned int init : 1; // Whether our structure is initialized
        unsigned int raw  : 1; // Whether we're currently in raw mode
        unsigned int ssh  : 1; // Whether we're working over a SSH connection
        unsigned int utf8 : 1; // Whether UTF-8 is supported (otherwise ASCII)
        unsigned int cjk  : 1; // Whether doublewidth CJK chars are supported
        unsigned int arw  : 1; // Whether arabic ligatures are wide
        unsigned int em   : 1; // Whether emoji are supported
        unsigned int emu  : 1; // Whether emoji are unaligned (need forced)
        unsigned int emj  : 1; // Whether emoji can be joined
    };
} ;


static ssize_t TU_INTERNAL_GetCursorMoveLength(const char * const text);

struct TUInfo TUInitialize(int enablerawmode) {
    static struct TUInfo T = {};

    #define env(a)      (getenv(a))
    #define streq(a,b)  (!strcmp(a,b))
    #define strhas(a,b) (!!strstr(a,b))
    
    //-------------------------------------------------------------------------
    //                           TERMINAL MODESETTING
    //-------------------------------------------------------------------------
    if (!T.init) {
        // If we haven't stored the original terminal settings or derived a
        // raw mode setting yet, let's do that. But first let's ensure we're
        // in an interactive terminal. 
        if (!isatty(STDIN_FILENO)) return -ENOTTY;
        if (tcgetattr(STDIN_FILENO, &T.origattr) == -1) return -ENOTTY;
        T.rawattr = T.origattr;
        T.rawattr.c_lflag &= ~(ECHO|ICANON|IEXTEN|ISIG);
        T.rawattr.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
        T.rawattr.c_oflag &= ~(OPOST);
        T.rawattr.c_cflag &= ~(CSIZE|PARENB);
        T.rawattr.c_cflag |=  (CS8);
        T.rawattr.c_cc[VMIN]  = 0;
        T.rawattr.c_cc[VTIME] = 1;
        // Now, use some heuristics to figure out what our text output looks
        // like (UTF-8? Emoji behavior? Etc.). 
        if (!env("SSH_CLIENT")) {
            if (strhas(env("TERM"), 
        }
        T.init = true;
    }

    // Toggle raw mode. This code cannot be run unless the terminal settings
    // have already been checked and we know STDIN_FILENO is a valid file
    // descriptor and the terminal is interactive. However, tcsetattr() can
    // still fail with errno as EINTR or EINVAL. 
    if (T.init && enablerawmode) {
        if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &T.rawattr) == -1) {
            return -errno;
        }
        T.raw = true;
    } else if (T.init && !enablerawmode) {
        if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &T.origattr) == -1) {
            return -errno;
        }
        T.raw = false;
    }


    //-------------------------------------------------------------------------
    //                           TERMINAL MODESETTING
    // Ensure we're in an interactive terminal, get current settings, modify
    // them to derive a raw mode, and try to set the raw mode.
    //-------------------------------------------------------------------------

    // Try to get cursor position
    write(STDOUT_FILENO, "\e[6n", 4); 
    if (read(STDIN_FILENO, &c, 1) < 1 || 
        c != '\e' || 
        (scanf("[%zd;%zdR", &row, &col) != 2)) {
            // Can't get cursor position :(
    }



    return 0;
    #undef streq(a,b) (strcmp(a,b) == 0)
    #undef strhas(a,b) (strstr(a,b) != NULL)
}



static ssize_t TU_INTERNAL_GetCursorMoveLength(const char * const text) {
    size_t row = 0;
    size_t col = 0;
    ssize_t chk;
    unsigned char c = 0;

    if (!TUTERM.rawmode) {
        fprintf(stderr, "Invalid use of internal API without Raw Mode enabled.\n");
        exit(EXIT_FAILURE);
    }

    write(STDOUT_FILENO, "\e[0G", 4);          // Start of line
    write(STDOUT_FILENO, "\e[8m", 4);          // Make hidden
    write(STDOUT_FILENO, text, strlen(text));  // Write text
    write(STDOUT_FILENO, "\e[28m", 5);         // Undo hidden
    write(STDOUT_FILENO, "\e[6n", 4);          // Get cursor pos'n
    write(STDOUT_FILENO, "\e[0G", 4);          // Back to start

    chk = read(STDIN_FILENO, &c, 1);
    if (chk < 1 || c != '\e') {
        fprintf(stderr, "Terminal response did not start with ESC: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    chk = scanf("[%zu;%zuR", &row, &col);
    if (chk != 2) {
        fprintf(stderr, "Terminal response did not contain cursor position: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return (col) ? col - 1 : 0;
}




int main(void) {
    ssize_t chk;
    int retval = 0;

    TUInit();
    TURawMode(true);

    if ((chk = TUGetCursorMoveLength(u8"丟")) < 0) goto ERRORHANDLING;
    printf("Length of \'%s\' is %zu\r\n", u8"丟", chk);
    if ((chk = TUGetCursorMoveLength(u8"🧑‍🌾")) < 0) goto ERRORHANDLING;
    printf("Length of \'%s\' is %zu\r\n", u8"🧑‍🌾", chk);
    TURawMode(false);
    return 0;

    ERRORHANDLING:
    printf("Failed to get cursor move length: %s\r\n", strerror(-chk));
    return chk;
}
