#include <stdio.h>
#include <locale.h>
#include <string.h>

int sysisutf8(void) {
    static int retval = 0;
    static int firstrun = 1;
    char *syscharset = NULL;

    if (firstrun) {
        syscharset = (setlocale(LC_ALL, ""), setlocale(LC_CTYPE, NULL));
        retval = 
            (syscharset && strlen(syscharset) >= strlen("UTF-8"))       ?\
            (!!!strcmp("UTF-8", syscharset + strlen(syscharset) - 5))   :\
            0;
        firstrun = 0;
    }
    
    return retval; 
}

int main(void) {
    if (sysisutf8()) printf("Your system locale is UTF-8.\n");
    else printf("Your system local is not UTF-8.\n");
    
    return 0;
}
