#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>



#define TURAW_L  ~(ECHO|ICANON|IEXTEN|ISIG)
#define TURAW_I  ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON)
#define TURAW_O  ~(OPOST)
#define TURAW_C  ~(CSIZE|PARENB)
#define TURAW_CC  (CS8)



struct TUTerminal {
    struct termios origattr; // Original mode
    struct termios rawattr;  // Raw mode
    int initialized;
    bool rawmode;
    bool cangetcurpos;
};

struct TUTerminal TUTERM = {};



int TUInit(void) {
    if (!isatty(STDIN_FILENO)) {
        if (errno) return -errno;
        else return -ENOTTY;
    }

    if (tcgetattr(STDIN_FILENO, &TUTERM.origattr) == -1) return -errno;
    TUTERM.rawattr = TUTERM.origattr;
    TUTERM.rawattr.c_lflag &= TURAW_L;
    TUTERM.rawattr.c_iflag &= TURAW_I;
    TUTERM.rawattr.c_oflag &= TURAW_O;
    TUTERM.rawattr.c_cflag &= TURAW_C;
    TUTERM.rawattr.c_cflag |= TURAW_CC;
    TUTERM.rawattr.c_cc[VMIN]  = 0;
    TUTERM.rawattr.c_cc[VTIME] = 1;

    TUTERM.initialized = 1;

    return 0;
}



int TURawMode(bool enable) {
    int chk;
    struct termios *attr;

    if (!TUTERM.initialized)
        if ((chk = TUInit()) < 0) goto catch;
    attr = (enable) ? &TUTERM.rawattr : &TUTERM.origattr;
    chk = tcsetattr(STDIN_FILENO, TCSAFLUSH, attr);
    if (chk < 0) goto catch;
    TUTERM.rawmode = (enable) ? true : false;
    return 0;

    catch:
    return chk;
}



ssize_t TUGetCursorMoveLength(const char * const text) {
    size_t row = 0;
    size_t col = 0;
    ssize_t chk;
    unsigned char c = 0;

    if (!TUTERM.rawmode) return -EIO;

    write(STDOUT_FILENO, "\e[0G", 4);          // Start of line
    write(STDOUT_FILENO, "\e[8m", 4);          // Make hidden
    write(STDOUT_FILENO, text, strlen(text));  // Write text
    write(STDOUT_FILENO, "\e[28m", 5);         // Undo hidden
    write(STDOUT_FILENO, "\e[6n", 4);          // Get cursor pos'n
    write(STDOUT_FILENO, "\e[0G", 4);          // Back to start

    chk = read(STDIN_FILENO, &c, 1);
    if (chk < 1 || c != '\e') goto catch;

    chk = scanf("[%zu;%zuR", &row, &col);
    if (chk != 2) goto catch;
    return (col) ? col - 1 : 0;

    catch:
    TUTERM.cangetcurpos = false;
    return -ENOTTY;
}



int main(void) {
    ssize_t chk;
    int retval = 0;

    TUInit();
    TURawMode(true);

    if ((chk = TUGetCursorMoveLength(u8"丟")) < 0) goto catch;
    printf("Length of \'%s\' is %zu\r\n", u8"丟", chk);
    if ((chk = TUGetCursorMoveLength(u8"🧑‍🌾")) < 0) goto catch;
    printf("Length of \'%s\' is %zu\r\n", u8"🧑‍🌾", chk);
    TURawMode(false);
    return 0;

    catch:
    printf("Failed to get cursor move length: %s\r\n", strerror(-chk));
    return chk;
}
