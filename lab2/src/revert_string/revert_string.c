#include "revert_string.h"
#include <stdlib.h>
#include <string.h>

void RevertString(char *str)
{
    if (str == NULL) { 
        return; 
    } 
  
    // получаем указатель на конец последнего символа в строке 
    char* end_ptr = str + (strlen(str) - 1); 
  
    // начинаем менять местами символы с обоих концов строки 
    while (end_ptr > str) 
    { 
        char ch = *str; 
        *str = *end_ptr; 
        *end_ptr = ch; 
  
        // увеличить строку и уменьшить end_ptr 
        ++str, --end_ptr; 
    }
}