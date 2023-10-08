#include <stdio.h>
#include <stdlib.h>
#include "blosum62.h"
#include "asci_map.h"


void read_blosum62_file() {
	int i, j;
	FILE *blosum62_file;
	if ((blosum62_file = fopen("blosum62.txt", "r")) == NULL) {
		fprintf(stderr, "Reading file BLOSUM62.txt has failed.\n");
		exit(51);
	}
	for(i = 0; i < 20; i++) {
        for(j = 0; j < 20; j++) {
            if (!fscanf(blosum62_file, "%d", &BLOSUM62[i][j])) {
				fprintf(stderr, "File BLOSUM62.txt is missing data (in (%d, %d)).\n", i, j);
				exit(52);
			}
        }
    } 
    fclose(blosum62_file);
}


int blosum62_function(char a, char b) {
	int i = ASCI_MAP[(int)(a) - 65];
	int j = ASCI_MAP[(int)(b) - 65];
	return BLOSUM62[i][j];
}


pblosum62_function get_blosum62_function() {
	read_blosum62_file();
	set_asci_map();
	return &blosum62_function;
}




