#include <stdio.h>
#include <stdlib.h>
#include "pam250.h"
#include "asci_map.h"


void read_pam250_file() {
	int i, j;
	FILE *pam250_file;
	if ((pam250_file = fopen("pam250.txt", "r")) == NULL) {
		fprintf(stderr, "Reading file PAM250.txt has failed.\n");
		exit(41);
	}
	for(i = 0; i < 20; i++) {
        for(j = 0; j < 20; j++) {
            if (!fscanf(pam250_file, "%d", &PAM250[i][j])) {
				fprintf(stderr, "File PAM250.txt is missing data (in (%d, %d)).\n", i, j);
				exit(42);
			}
        }
    } 
    fclose(pam250_file);
}

int pam250_function(char a, char b) {
	int i = ASCI_MAP[(int)(a) - 65];
	int j = ASCI_MAP[(int)(b) - 65];
	return PAM250[i][j];
}

ppam250_function get_pam250_function() {
	read_pam250_file();
	set_asci_map();
	return &pam250_function;
}




