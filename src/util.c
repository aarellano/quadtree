#include <stdio.h>


int atoi(char *string) {

  int negative = 0;
  int value = 0;
  char *start;

if (string == NULL) /* if data passed in is null */
    return 0;

start = string;

while(*start == ' ') /* ignore pre-whitespaces */
    start++;

if (*start == '-') {
    negative = 1;
    start++;
}

while (*start != '\0') {
    if (*start >= '0' && *start <= '9') { /* numeric characters */
      if (negative == 0 && value * 10 + ((int)*start - '0') < value) { /* check for + buffer overflow */
        printf("buffer overflow \n");
        return 0;
      }
      if (negative == 1 && (((-value) * 10) - ((int)*start - '0') > -value)) { /* check for - buffer overflow */
        printf("buffer overflow \n");
        return 0;
      }
      value = value * 10 + ((int)*start - '0');
    }
    start++;
 }

if (negative)
    value = -value;

return value;
}
