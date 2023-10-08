#include <stdio.h>


int PAM250[20][20]; 

void read_pam250_file();
int pam250_function(char, char);
typedef int (*ppam250_function)(char, char);
ppam250_function get_pam250_function();
