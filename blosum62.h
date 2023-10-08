#include <stdio.h>


int BLOSUM62[20][20];

void read_blosum62_file();
int blosum62_function(char, char);
typedef int (*pblosum62_function)(char, char);
pblosum62_function get_blosum62_function();