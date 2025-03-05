#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// NEED TO KNOW
// Character set/code page (incl. UTF-8, UTF-16, etc.)
// If Unicode, what Unicode version supported
// Length of various Arabic ligatures

struct TUInfo {
    short int ucver;
    short 
}



// 	/* Adjust output channel */
// 	tcgetattr(filedes, &ostate);		/* save old state */
// 	nstate = ostate;			/* get base of new state */
// #ifndef cfmakeraw
// 	nstate.c_iflag &= ~(IMAXBEL|IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
// 	nstate.c_oflag &= ~OPOST;
// 	nstate.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
// 	nstate.c_cflag &= ~(CSIZE|PARENB);
// 	nstate.c_cflag |= CS8;
// #else
// 	cfmakeraw(&nstate);
// #endif
// 	tcsetattr(filedes, TCSADRAIN, &nstate);	/* set mode */

// 	/* Query size of terminal by first trying to position cursor */
// 	if (write(filedes, query, sizeof(query)) != -1 && poll(&fd, 1, 300) > 0) {
// 		int row = 0, col = 0;

// 		/* Terminal responds with \e[row;posR */
// 		if (scanf("\e[%d;%dR", &row, &col) == 2) {
// 			t.t_nrow = row;
// 			t.t_ncol = col;
// 		}
// 	}


#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif

static int TU_INTERNAL_SysIsUTF8(void) {
    char *syslang = getenv("LANG");
    size_t len = strlen(syslang);
    if (len >= 5) return ( !strcmp("UTF-8", &syslang[len-5]) ) ? true : false;
    return false;
}



static int TU_INTERNAL_SSHIsActive(void) {
    return (getenv("SSH_CLIENT") || getenv("SSH_CONNECTION")) ? true : false;
}



static int TU_INTERNAL_TerminalIsLegacy(void) {
    char *term = getenv("TERM");
    if (term) {
        #define strequals(a, b) (!strcmp(a, b))
        return (strequals(term, "vt100")) ? true :
            (strequals(term, "vt220"))    ? true :
            (strequals(term, "cons25"))   ? true :
            (strequals(term, "rxvt"))     ? true :
            (strequals(term, "cons25-c")) ? true :
            (strequals(term, "ansi"))     ? true :
            (strequals(term, "Eterm"))    ? true :
            (strequals(term, "dumb"))     ? true :
            (strequals(term, "kterm"))    ? true :
            (strequals(term, "cygwin"))   ? true : false;
        #undef strequals
    }
    return false;
}



static int TU_UTF8(void) {
    static _Thread_local int retval = -1;
    static _Thread_local unsigned long lastcheck = 0;
    static const int checkinterval = CLOCKS_PER_SEC * 60 * 5;

    if (retval == -1 || clock() > lastcheck + checkinterval) {
        lastcheck = clock();
        if (TU_INTERNAL_SSHIsActive())
            retval = !TU_INTERNAL_TerminalIsLegacy();
        else
            retval = TU_INTERNAL_SysIsUTF8();
    }

    return retval;
}



// Some info is at
//   /etc/terminfo
//   /usr/share/terminfo
//   /lib/terminfo
//   /etc/termcap

// enum TUTermType {
//     ASCII,
//     ALACRITTY,
//     APPLETERM,
//     GHOSTTY,
//     GNOME,
//     ITERM,
//     KITTY,
//     VTE,
//     WEZTERM,
//     XFCE,
//     XTERM,
// }
// struct TUInfo {
//     short ucver;
//     short
// }
// int TUInternalCheckAcceptEnv(const char * const var) {
//     static thread_local int firsttime = 1;
//     static thread_local char **env = NULL;
//     FILE *sshd_config;
//
//     if (firsttime) {
//
//         sshd_config = fopen("/etc/ssh/sshd_config", "rb");
//         if (!sshd_config) return 0;
//
//     }
// }
// struct TUInfo TUGetInfo(void) {
//     bool ssh = (getenv("SSH_CLIENT") || getenv("SSH_CONNECTION")) ? true : false;
// }



// int main(void) {
//     TUInfo = TUGetInfo()
// }

#ifdef TU_TESTDRIVER
int main(void) {
    if (TU_UTF8()) printf("Your system locale is UTF-8.\n");
    else printf("Your system local is not UTF-8.\n");
    
    return 0;
}
#endif
