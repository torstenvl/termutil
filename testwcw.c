#include <stdio.h>
#include <locale.h>
#include <wchar.h>
 
int main(void) {
 
    wchar_t arabic[] = { 0xFDC4, 0x0000 };
    wchar_t emoji1[] = { 0x1F9D1, 0x0000 }; 
    wchar_t emoji2[] = { 0x1F33E, 0x0000 }; 
    wchar_t combin[] = { 0x1F9D1, 0x200D, 0x1F33E, 0x0000 }; 
 
    setlocale(LC_ALL, "");
 
    printf("Locale is %s\n", setlocale(LC_ALL, NULL));
    //printf("Arabic:   \"%ls\"     Cnt: %d\n", arabic, wcwidth(arabic[0]));
    printf("Emoji 1:  \"%ls\"    Cnt: %d\n", emoji1, wcwidth(emoji1[0]));
    printf("Emoji 2:  \"%ls\"    Cnt: %d\n", emoji2, wcwidth(emoji2[0]));
    printf("Combined: \"%ls\"    Cnt: %d + %d + %d = %d\n", combin,
        wcwidth(combin[0]), wcwidth(combin[1]), wcwidth(combin[2]),
        wcwidth(combin[0]) + wcwidth(combin[1]) + wcwidth(combin[2]) );
 
    return 0;
}
