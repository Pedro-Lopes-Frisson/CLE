#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#define BUFFER_SIZE 50

int main () {
   size_t ret;
   char *MB = (char *)malloc( BUFFER_SIZE );
   wchar_t *WC = L"à";

   /* converting wide-character string */
   ret = wcsrtombs(MB, WC, BUFFER_SIZE, NULL);
   
   printf("Characters converted = %u\n", ret);
   printf("Multibyte character = %s\n\n", MB);
   
   return(0);
}
