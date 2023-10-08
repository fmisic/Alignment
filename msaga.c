#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "pam250.h"
#include "blosum62.h"


void *safe_malloc(size_t n) {
    void *p = malloc(n);
    if (p == NULL) {
        fprintf(stderr, "Failed to allocate %zu bytes.\n", n);
        exit(11);
    }
    return p;
}

typedef struct data {
	char** matrix;
	int row;
	int column;
	int* len;
} data;

void allocate_data(data* input, int row, int column) {
	int i;
	char *pfirst;
	int byte_size = row * sizeof(char*) + row * column * sizeof(char);
	input->matrix = safe_malloc(byte_size);
	pfirst = (char *)(input->matrix + row);
	for(i = 0; i < row; i++) {
        input->matrix[i] = pfirst + column * i;
	}
	input->len = safe_malloc(row * sizeof(int));
	input->row = row;
	input->column = column;
}

void free_data(data* input) {
	free(input->matrix);
	input->matrix = NULL;
	free(input->len);
	input->len = NULL;
}

typedef struct alignment {
	int** matrix;
	int row;
	int column;
	int score;
	int normalized_score;
} alignment;

void allocate_alignment(alignment* A, int row, int column) {
	int i, *pfirst;
	int byte_size = row * sizeof(int *) + row * column * sizeof(int);
	A->matrix = safe_malloc(byte_size);
	pfirst = (int *)(A->matrix + row);
	for(i = 0; i < row; i++) {
        A->matrix[i] = pfirst + column * i;
	}
	A->row = row;
	A->column = column;
	A->score = 0;
	A->normalized_score = 0;
}

void free_alignment(alignment* A) {
	free(A->matrix);
	A->matrix = NULL;
}

typedef struct alignment_list {
	alignment* list;
	int size;
	int position;
} alignment_list;

void allocate_alignment_list(alignment_list* alignments, int size, int row, int column) {
	int i;
	alignments->list = safe_malloc(size * sizeof(alignment));
	for(i = 0; i < size; i++) {
		allocate_alignment(&alignments->list[i], row, column);
	}
	alignments->size = size;
	alignments->position = 0;
}

void free_alignment_list(alignment_list* alignments) {
	int i;
	for(i = 0; i < alignments->size; i++) {
		free_alignment(&alignments->list[i]);
	}
	free(alignments->list);
	alignments->list = NULL;
}

typedef int (*pscoring_function)(char, char);

typedef struct constants_type constants_type;

typedef void (*pfitness_function)(alignment*, data*, constants_type*);

struct constants_type {
	int GENERATION_NUMBER;
	int POPULATION_NUMBER;
	int POPULATION_SIZE;
	int SOLUTION;
	double GAP_COEFFICIENT;
	double MUTATION_PARAMETER;
	double MUTATION1_PARAMETER;
	double MUTATION2_PARAMETER;
	double MUTATION3_PARAMETER;
	char* INPUT_FILENAME;
	pfitness_function FITNESS_FUNCTION;
	pscoring_function SCORING_FUNCTION;
};

void allocate_constants_type(constants_type* constants, int filename_size) {
	constants->INPUT_FILENAME = safe_malloc(filename_size * sizeof(char));
}

void free_constants_type(constants_type* constants) {
	free(constants->INPUT_FILENAME);
	constants->INPUT_FILENAME = NULL;
}

//---------------------------------

void print_alignment(alignment* A1,  data* input) {
	int i, j, zero;
	for (i = 0; i < A1->row; i++) {
		zero = 0;
		for (j = 0; j < A1->column; j++) {
			if (A1->matrix[i][j] == 0) {
				printf("%c", input->matrix[i][zero]);
				zero++;
			}
			else {
				printf("-");
			}
		}
		printf("\n");
	}
	printf("Score: %d\n", A1->score);
}

void print_alignment_binary(alignment* A1) {
	int i, j;
	int ones;
	for (i = 0; i < A1->row; i++) {
		ones = 0;
		for (j = 0; j < A1->column; j++) {
			printf("%d", A1->matrix[i][j]);
			if (A1->matrix[i][j]) ones++;
		}
		printf(" %d", ones);
		printf("\n");
	}
	printf("Score: %d\n", A1->score);
}

int print_loading_bar(int percentage, int add_percentage) {
	int i;
	if (add_percentage > 100 - percentage) {
		add_percentage = 100 - percentage;
	}
	if (add_percentage > 0) {
		if (percentage == 0 && add_percentage > 0) {
			
			for (i = 0; i < 22; i++) {
				printf("_");
			}
			printf("\n");		
			printf("[");
		}
		if (percentage % 5 > 0) {
			printf("\b");
		}
		add_percentage += percentage % 5;
		for (i = 0; i < add_percentage / 5; i++) {
			printf("#");
		}
		switch (add_percentage % 5) {
			case 1:
				printf(".");
				break;
			case 2:
				printf(":");
				break;
			case 3:
				printf("-");
				break;
			case 4:
				printf("=");
				break;
			default:
				break;
		}
		add_percentage -= percentage % 5;
		percentage += add_percentage;
		if (percentage == 100) {
			printf("]\n");
		}
	}
	fflush(stdout);
	return percentage;
}

void set_fitness1(alignment* A, data* input, constants_type* CONSTANTS) {
	int i, j, k, zero1, zero2, previous, matrix_score;
	int row = A->row;
	int column = A->column;
	int* ungapped = safe_malloc(column * sizeof(int));
	for (j = 0; j < column; j++) ungapped[j] = 0;
	int sum = 0;
	double gap_penalties = 0;

	for (j = 0; j < column; j++) {
		for (i = 0; i < row; i++){
			if (A->matrix[i][j] != 1) ungapped[j] = 1;
		}
	}

	for (i = 0; i < row; i++) {
		for (j = i + 1; j < row; j++) {
			zero1 = 0;
			zero2 = 0;
			for (k = 0; k < column; k++) {
				if (A->matrix[i][k] == 0 && A->matrix[j][k] == 0) {
					matrix_score = (*(CONSTANTS->SCORING_FUNCTION))(input->matrix[i][zero1], input->matrix[j][zero2]);
					if (input->matrix[i][k] == input->matrix[j][k]) {
						sum += 2 * matrix_score;
					} else {
						sum += matrix_score;
					}
					
				}
				if (A->matrix[i][k] == 0) zero1++;
				if (A->matrix[j][k] == 0) zero2++;				
			}
		}
	}
	
	for (i = 0; i < row; i++) {
		previous = -1;
		for (j = 0; j < column; j++) {
			if (ungapped[j] == 1) {
				if (A->matrix[i][j] == 1) {
					if (previous == 1) {
						gap_penalties += 0.05;
					}
					else {
						gap_penalties += 10;
					}
				}
				previous = A->matrix[i][j];
			}
		}
	}
	
	A->score = sum - gap_penalties;
	free(ungapped);
	return;
}

void set_fitness2(alignment* A, data* input, constants_type* CONSTANTS) {
	int i, j, k, zero1, zero2, previous_i, previous_j;
	int row = A->row;
	int column = A->column;
	int sum = 0;
	double gap_penalties = 0;
	
	for (i = 0; i < row; i++) {
		for (j = i + 1; j < row; j++) {
			zero1 = 0;
			zero2 = 0;
			previous_i = 0;
			previous_j = 0;
			for (k = 0; k < column; k++) {
				if (A->matrix[i][k] == 0 && A->matrix[j][k] == 1) {
					if (previous_j == 1) gap_penalties += 0.05;
					else gap_penalties += 10;
					previous_i = 0;
					previous_j = 1;
				}
				if (A->matrix[i][k] == 1 && A->matrix[j][k] == 0) {
					if (previous_i == 1) gap_penalties += 0.05;
					else gap_penalties += 10;
					previous_i = 1;
					previous_j = 0;
				}

				if (A->matrix[i][k] == 0 && A->matrix[j][k] == 0) {
					sum += (*(CONSTANTS->SCORING_FUNCTION))(input->matrix[i][zero1], input->matrix[j][zero2]);
				}
				if (A->matrix[i][k] == 0) zero1++;
				if (A->matrix[j][k] == 0) zero2++;
			}
		}
	}
	
	A->score = sum - gap_penalties;
	return;
}

void initialize_constants(constants_type* constants, int argc, char *argv[]) {
	int DEFAULT_GENERATION_NUMBER = 1000;
	int DEFAULT_POPULATION_NUMBER = 10;
	int DEFAULT_POPULATION_SIZE = 100;
	int DEFAULT_SOLUTION = 0;
	double DEFAULT_GAP_COEFFICIENT = 1.2;
	double DEFAULT_MUTATION_PARAMETER = 0.1;
	double DEFAULT_MUTATION1_PARAMETER = 0.3;
	double DEFAULT_MUTATION2_PARAMETER = 0.3;
	double DEFAULT_MUTATION3_PARAMETER = 0.3;
	pfitness_function DEFAULT_FITNESS_FUNCTION = &set_fitness1;
	// DEFAULT SCORING_FUNCTION is blosum62_function	
	
	constants->GENERATION_NUMBER = DEFAULT_GENERATION_NUMBER;
	constants->POPULATION_NUMBER = DEFAULT_POPULATION_NUMBER;
	constants->POPULATION_SIZE = DEFAULT_POPULATION_SIZE;
	constants->SOLUTION = DEFAULT_SOLUTION;
	constants->GAP_COEFFICIENT = DEFAULT_GAP_COEFFICIENT;
	constants->MUTATION_PARAMETER = DEFAULT_MUTATION_PARAMETER;
	constants->MUTATION1_PARAMETER = DEFAULT_MUTATION1_PARAMETER;
	constants->MUTATION2_PARAMETER = DEFAULT_MUTATION2_PARAMETER;
	constants->MUTATION3_PARAMETER = DEFAULT_MUTATION3_PARAMETER;
	constants->FITNESS_FUNCTION = DEFAULT_FITNESS_FUNCTION;
	constants->SCORING_FUNCTION = NULL;
	if (argc < 2) {
		fprintf(stderr, "File name of the input file is required. (example: %s input_file.txt)\n", argv[0]);
		exit(21);
	}
	allocate_constants_type(constants, (strlen(argv[1]) + 1));
	strcpy(constants->INPUT_FILENAME, argv[1]);
	int i = 2, fitness_option;
	while (i < argc) {
		if (argv[i][0] != '-') {
			fprintf(stderr, "Expected option instead of %s.\n", argv[i]);
			exit(21);
		} else {
			if (strcmp(argv[i], "-popnum") == 0) {
				constants->POPULATION_NUMBER = atoi(argv[++i]);
				if (constants->POPULATION_NUMBER == 0) {
					fprintf(stderr, "%s is not valid population number (-popnum) (it should be positive integer).\n", argv[i]);
					exit(22);
				}
			} else if (strcmp(argv[i], "-popsize") == 0) {
				constants->POPULATION_SIZE = atoi(argv[++i]);
				if (constants->POPULATION_SIZE == 0 || constants->POPULATION_SIZE % 2 == 1) {
					fprintf(stderr, "%s is not valid generation size (-popsize) (it should be even integer).\n", argv[i]);
					exit(22);
				}
			} else if (strcmp(argv[i], "-gennum") == 0) {
				constants->GENERATION_NUMBER = atoi(argv[++i]);
				if (constants->GENERATION_NUMBER == 0) {
					fprintf(stderr, "%s is not valid generation number (-gennum) (it should be positive integer).\n", argv[i]);
					exit(23);
				}
			} else if (strcmp(argv[i], "-gapcoeff") == 0) {
				constants->GAP_COEFFICIENT = atof(argv[++i]);
				if (constants->GAP_COEFFICIENT <= 1.0) {
					fprintf(stderr, "%s is not valid gap coefficient (-gapcoeff) (it should be number greater than 1.0).\n", argv[i]);
					exit(24);
				}
			} else if (strcmp(argv[i], "-mparams") == 0) {
				constants->MUTATION_PARAMETER = atof(argv[++i]);
				if (constants->MUTATION_PARAMETER <= 0.0 || constants->MUTATION_PARAMETER > 1.0) {
					fprintf(stderr, "%s is not valid mutation parameter (-mparams) (it should be positive number less than 1.0).\n", argv[i]);
					exit(25);
				}
				constants->MUTATION1_PARAMETER = atof(argv[++i]);
				if (constants->MUTATION1_PARAMETER <= 0.0 || constants->MUTATION1_PARAMETER > 1.0) {
					fprintf(stderr, "%s is not valid mutation1 parameter (-mparams) (it should be positive number less than 1.0).\n", argv[i]);
					exit(26);
				}
				constants->MUTATION2_PARAMETER = atof(argv[++i]);
				if (constants->MUTATION2_PARAMETER <= 0.0 || constants->MUTATION2_PARAMETER > 1.0) {
					fprintf(stderr, "%s is not valid mutation1 parameter (-mparams) (it should be positive number less than 1.0).\n", argv[i]);
					exit(27);
				}
				constants->MUTATION3_PARAMETER = atof(argv[++i]);
				if (constants->MUTATION3_PARAMETER <= 0.0 || constants->MUTATION3_PARAMETER > 1.0) {
					fprintf(stderr, "%s is not valid mutation1 parameter (-mparams) (it should be positive number less than 1.0).\n", argv[i]);
					exit(28);
				}
			} else if (strcmp(argv[i], "-fitness") == 0) {
				fitness_option = atoi(argv[++i]);
				if (fitness_option == 1) {
					constants->FITNESS_FUNCTION = &set_fitness1;
				} else if (fitness_option == 2) {
					constants->FITNESS_FUNCTION = &set_fitness2;
				} else {
					fprintf(stderr, "%s is not valid option for fintess function (-fitness) (it should be either 1 or 2).\n", argv[i]);
					exit(29);
				}
			} else if (strcmp(argv[i], "-score") == 0) {
				i++;
				if (strcmp(argv[i], "blosum62") == 0) {
					constants->SCORING_FUNCTION = get_blosum62_function();
				} else if (strcmp(argv[i], "pam250") == 0) {
					constants->SCORING_FUNCTION = get_pam250_function();
				} else {
					fprintf(stderr, "%s is not valid option for scoring matrix (-score) (it should be either 'blosum62' or 'pam250').\n", argv[i]);
					exit(29);
				}
			} else if (strcmp(argv[i], "-solution") == 0) {
				constants->SOLUTION = 1;
			} else {
				fprintf(stderr, "Unsupported option %s.\nAvailable options are: -popnum, -popsize, -gennum, -gapcoeff, -mparams, -fitness, -score, -solution.\n", argv[i]);
				exit(29);
			}
		}
		i++;
	}
	if (constants->SCORING_FUNCTION == NULL) {
		constants->SCORING_FUNCTION = get_blosum62_function();
	}
}

void read_input_file(data* input, constants_type* CONSTANTS) {
	FILE* input_file;
	char c;
	int row = 0;
	int column = 0;
	int current_column = 0;
	int i = 0;
	int j = 0;
	if ((input_file = fopen(CONSTANTS->INPUT_FILENAME, "r")) == NULL) {		
		fprintf(stderr, "Reading file %s has failed.\n", CONSTANTS->INPUT_FILENAME);
		exit(31);
	}
	while (fscanf(input_file, "%c", &c) > 0) {
		if (c != '\r') {
			if (c == '\n') {
				if (current_column > column) {
					column = current_column;
				}
				if (current_column > 0) {
					row++;
					current_column = 0;
				}				
			} else {
				current_column++;	
			}
		}
	}
	if (current_column > 0) {
		if (current_column > column) {
			column = current_column;
		}
		row++;
	}
	allocate_data(input, row, column);
	rewind(input_file);
	while (fscanf(input_file, "%c", &c) > 0) {
		if (c != '\r') {
			if (c == '\n') {
				if (j > 0) {
					input->len[i] = j;
					i++;
					input->len[i] = 0;
				}
				j = 0;
			} else {
				input->matrix[i][j] = c;
				j++;
			}
		}
	}
	if (j > 0) {
		input->len[i] = j;
	}
	fclose(input_file);
}

void read_alignment_file(alignment* A1, data* input, constants_type* CONSTANTS) {
	FILE* alignment_file;
	char c;
	int row = 0;
	int input_column = 0;
	int current_input_column = 0;
	int alignment_column = 0;
	int current_alignment_column = 0;
	int i = 0;
	int ij = 0;
	int	aj = 0;
	if ((alignment_file = fopen(CONSTANTS->INPUT_FILENAME, "r")) == NULL) {		
		fprintf(stderr, "Reading file %s has failed.\n", CONSTANTS->INPUT_FILENAME);
		exit(31);
	}
	while (fscanf(alignment_file, "%c", &c) > 0) {
		if (c != '\r') {
			if (c == '\n') {
				if (current_input_column > 0) {
					if (current_alignment_column > alignment_column) {
						alignment_column = current_alignment_column;
					}
					if (current_input_column > input_column) {
						input_column = current_input_column;
					}
					row++;
					current_input_column = 0;
					current_alignment_column = 0;
				}
			} else {
				current_alignment_column++;	
				if (c != '-') {
					current_input_column++;
				}
			}
		}
	}
	if (current_input_column > 0) {
		if (current_alignment_column > alignment_column) {
			alignment_column = current_alignment_column;
		}
		if (current_input_column > input_column) {
			input_column = current_input_column;
		}
		row++;
	}
	allocate_data(input, row, input_column);
	allocate_alignment(A1, row, alignment_column);
	rewind(alignment_file);
	while (fscanf(alignment_file, "%c", &c) > 0) {
		if (c != '\r') {
			if (c == '\n') {
				if (ij > 0) {
					while (aj < alignment_column) {
						A1->matrix[i][aj] = 1;
						aj++;
					}
					input->len[i] = ij;
					i++;
					input->len[i] = 0;
				}
				ij = 0;
				aj = 0;
			} else {
				if (c != '-') {
					input->matrix[i][ij] = c;
					ij++;
					A1->matrix[i][aj] = 0;
				} else {
					A1->matrix[i][aj] = 1;
				}
				aj++;
			}
		}
	}
	if (ij > 0) {
		while (aj < alignment_column) {
			A1->matrix[i][aj] = 1;
			aj++;
		}
		input->len[i] = ij;
	}
	
	fclose(alignment_file);
}

void copy(alignment* A1, alignment* A2) {
	int i, j;
	for (i = 0; i < A1->row; i++) {
		for (j = 0; j < A1->column; j++) {
			A2->matrix[i][j] = A1->matrix[i][j];
		}
	}
	A2->score = A1->score;
	A2->normalized_score = A1->normalized_score;
}

void swap(alignment* A1, alignment* A2) {
	int i, j, temp;
	for (i = 0; i <  A1->row; i++) {
		for (j = 0; j < A1->column; j++) {
			temp = A1->matrix[i][j];
			A1->matrix[i][j] = A2->matrix[i][j];
			A2->matrix[i][j] = temp;
		}
	}
	temp = A1->score;
	A1->score = A2->score;
	A2->score = temp;
	temp = A1->normalized_score;
	A1->normalized_score = A2->normalized_score;
	A2->normalized_score = temp;
}

int partition(alignment* arr, int low, int high) {
	int i, j, pivot, pivot_index;
	pivot_index = low + rand()%(high - low + 1);
	swap(&arr[pivot_index], &arr[high]);
	pivot = arr[high].score;
	i = (low - 1);
	for (j = low; j <= high - 1; j++) {
		if (arr[j].score < pivot) {
			i++; 
			swap(&arr[i], &arr[j]);
		}
	}
	swap(&arr[i + 1], &arr[high]);
	return (i + 1);
}

void quick_sort(alignment* arr, int low, int high) {
	int pi;
	if (low < high) {
		pi = partition(arr, low, high);
		quick_sort(arr, low, pi - 1);
		quick_sort(arr, pi + 1, high);
	}
}

void sort_alignment_list(alignment_list* alignments) {
	quick_sort(alignments->list, 0, alignments->size - 1);
}

void normalize_fitness(alignment_list* alignments) {
	int absolute_min, change;
	int i = 1;
	int j = alignments->size - 2;
	for (; i < alignments->size && j > 0; i++, j--) {
		if (alignments->list[i].score > 0 || alignments->list[j].score < 0) break;
	}	
	if (alignments->list[i].score > 0) {
		if (abs(alignments->list[i - 1].score) < abs(alignments->list[i].score)) {
			absolute_min = abs(alignments->list[i - 1].score);
		} else {
			absolute_min = abs(alignments->list[i].score);
		}
	}
	else if (alignments->list[j].score < 0) {
		if (abs(alignments->list[j].score) < abs(alignments->list[j + 1].score)) {
			absolute_min = abs(alignments->list[j].score);
		}
		else {
			absolute_min = abs(alignments->list[j + 1].score);
		}
	}
	change = absolute_min - alignments->list[0].score;
	for (i = 0; i < alignments->size; i++) {
		alignments->list[i].normalized_score = alignments->list[i].score + change;
	}
}

void initialization(alignment_list* parents, data* input, alignment* solution, constants_type* CONSTANTS) {
	int j, k, l;
	int gap_number, num1, num2, column, repeat;
	int i = 0, end = 0;
	if (CONSTANTS->SOLUTION) {
		end = 0.1 * parents->size + 1;
		for (; i < end; i++) {
			copy(solution, &parents->list[i]);
		}
	}
	end += 2;
	if (end > parents->size) {
		end = parents->size;
	}
	for (; i < end; i++) {
		for (j = 0; j < parents->list[i].row; j++) {
			for (k = 0; k < input->len[j]; k++) {
				parents->list[i].matrix[j][k] = 0;
			}
			for (k = input->len[j]; k < parents->list[i].column; k++) {
				parents->list[i].matrix[j][k] = 1;
			}
		}
		(*(CONSTANTS->FITNESS_FUNCTION))(&parents->list[i], input, CONSTANTS);
	}
	for (; i < parents->size; i++) {
		for (j = 0; j < parents->list[i].row; j++) {
			column = parents->list[i].column;
			for (k = 0; k < column; k++) {
				parents->list[i].matrix[j][k] = 0;
			}
			gap_number = column - input->len[j];
			num2 = -1;
			for (k = 1; k <= gap_number; k++) {
				num1 = (rand() % (3));
				if (num1 == 0 || num2 == -1) {
					repeat = 1;
					num2 = (rand() % (column));
					while (parents->list[i].matrix[j][num2] == 1 && repeat <= 5) {
						num2 = (rand() % (column));
						repeat++;
					}
					if (parents->list[i].matrix[j][num2] == 1) {
						l = (num2 + 1) % column;
						while (parents->list[i].matrix[j][l] == 1) {
							l = (l + 1) % column;
						}
						num2 = l;
					}
					parents->list[i].matrix[j][num2] = 1;
				} else if (num1 == 1) {
					l = (num2 - 1 + column) % column;
					while (parents->list[i].matrix[j][l] == 1) {
						l = (l + 1) % column;
					}
					num2 = l;
					parents->list[i].matrix[j][num2] = 1;
				} else if (num1 == 2) {
					l = (num2 + 1) % column;
					while (parents->list[i].matrix[j][l] == 1) {
						l = (l + 1) % column;
					}
					num2 = l;
					parents->list[i].matrix[j][num2] = 1;
				}
			}
		}
		(*(CONSTANTS->FITNESS_FUNCTION))(&parents->list[i], input, CONSTANTS);
	}
	parents->position = parents->size - 1;
	sort_alignment_list(parents);
	normalize_fitness(parents);
	return;
}

int selection(int select_index, alignment_list* alignments) {
	int i, j, k, num, sum;
	int sum_fitness = 0;
	for (i = 0; i < alignments->size; i++) {
		sum_fitness += alignments->list[i].normalized_score;
	}
	for (k = 0; k < 5; k++) {
		num = (rand() % (sum_fitness + 1));
		sum = 0;
		for (i = 0; i < alignments->size; i++) {
			if (alignments->list[i].normalized_score > 0) {
				sum += alignments->list[i].normalized_score;
			}
			if (num <= sum) {
				if (i != select_index) {
					return i;
				} else {
					j = i;
					break;
				}
			}
		}
	}
	return (j + 1) % alignments->size;
}

void crossover1(alignment* A1, alignment* A2, alignment_list* children) {
	int i, j, diff, diff_copy;
	int num = rand() % (A1->column);
	int pos = children->position;
	for (i = 0; i < A1->row; i++) {
		diff = 0;
		for (j = 0; j < num; j++) {
			if (A1->matrix[i][j] == 1) diff++;
			if (A2->matrix[i][j] == 1) diff--;
			children->list[pos].matrix[i][j] = A1->matrix[i][j];
			children->list[pos + 1].matrix[i][j] = A2->matrix[i][j];
		}
		j = num - 1;
		diff_copy = diff;
		
		while (diff > 0) {
			if (children->list[pos].matrix[i][j] == 1) {
				children->list[pos].matrix[i][j] = 0;
				diff--;
			}
			j--;
		}
		j = num - 1;
		diff = diff_copy;
		while (diff > 0) {
			if (children->list[pos + 1].matrix[i][j] == 0) {
				children->list[pos + 1].matrix[i][j] = 1;
				diff--;
			}
			j--;
		}
		
		while (diff < 0) {
			if (children->list[pos].matrix[i][j] == 0) {
				children->list[pos].matrix[i][j] = 1;
				diff++;
			}
			j--;
		}
		j = num - 1;
		diff = diff_copy;
		while (diff < 0) {
			if (children->list[pos + 1].matrix[i][j] == 1) {
				children->list[pos + 1].matrix[i][j] = 0;
				diff++;
			}
			j--;
		}
	}
	
	for (i = 0; i <  A1->row; i++) {
		for (j = num; j <  A1->column; j++) {
			children->list[pos].matrix[i][j] = A2->matrix[i][j];
			children->list[pos + 1].matrix[i][j] = A1->matrix[i][j];
		}
	}
	
	children->position += 2;
	return;
}

void crossover2(alignment* A1, alignment* A2, alignment_list* children) {
	int i, j;
	int pos = children->position;
	int num = rand() % A1->row;
	for (i = 0; i < num; i++) {
		for (j = 0; j < A1->column; j++) {
			children->list[pos].matrix[i][j] = A1->matrix[i][j];
			children->list[pos + 1].matrix[i][j] = A2->matrix[i][j];
		}
	}
	for (i = num; i < A1->row; i++) {
		for (j = 0; j < A1->column; j++) {
			children->list[pos].matrix[i][j] = A2->matrix[i][j];
			children->list[pos + 1].matrix[i][j] = A1->matrix[i][j];
		}
	}
	children->position += 2;
	return;
}

void mutation1(alignment* A, constants_type* CONSTANTS) {
	int i, j1, j2, temp;
	double rnum;
	for (i = 0; i < A->row; i++) {
		rnum = rand() / RAND_MAX;
		if (rnum < CONSTANTS->MUTATION1_PARAMETER) {
			j1 = rand() % A->column;
			j2 = rand() % A->column;
			temp = A->matrix[i][j1];
			A->matrix[i][j1] = A->matrix[i][j2];
			A->matrix[i][j2] = temp;
		}
	}
}

void mutation2(alignment* A, constants_type* CONSTANTS, int* bit_array) {
	int i, j, j1, j2, temp, num;
	double rnum;
	for (i = 0; i < A->row; i++) {
		rnum = rand() / RAND_MAX;
		if (rnum < CONSTANTS->MUTATION2_PARAMETER) {
			j1 = rand() % A->column;
			j2 = rand() % A->column;
			if (j2 < j1) {
				temp = j2;
				j2 = j1;
				j1 = j2;
			}
			if (j2 != j1) {
				num = rand() % (j2 - j1) + 1;
				for (j = j1; j < j2; j++) {
					bit_array[j - j1] = A->matrix[i][j];
				}
				for (j = j1; j < j2; j++) {
					A->matrix[i][j] = bit_array[(j - j1 + num) % (j2 - j1)];
				}
			}
		}
	}
}

void mutation3(alignment* A, constants_type* CONSTANTS) {
	int i, j, j1, j2, temp;
	double rnum;
	for (i = 0; i < A->row; i++) {
		rnum = rand() / RAND_MAX;
		if (rnum < CONSTANTS->MUTATION3_PARAMETER) {
			j1 = rand() % A->column;
			j2 = rand() % A->column;
			if (j2 < j1) {
				temp = j2;
				j2 = j1;
				j1 = j2;
			}
			for (j = j1; j < j1 + (j2 - j1 + 1) / 2; j++) {
				temp = A->matrix[i][j];
				A->matrix[i][j] = A->matrix[i][j2 - j + j1];
				A->matrix[i][j2 - j + j1] = temp;
			}
		}
	}
}

int main(int argc, char** argv) {
	constants_type CONSTANTS;
	data input;
	alignment solution;
	alignment_list parents, children;
	int alignment_length, i, j, k, p1, p2, num, i1, i2, l;
	int *bit_array;
	double rnum;
	int gen_percent, sec;
	clock_t start, diff;
	start = clock();
	
	srand(time(0));
	
	initialize_constants(&CONSTANTS, argc, argv);
	if (CONSTANTS.SOLUTION) {
		read_alignment_file(&solution, &input, &CONSTANTS);
		(*(CONSTANTS.FITNESS_FUNCTION))(&solution, &input, &CONSTANTS);
		alignment_length = solution.column;
	} else {
		read_input_file(&input, &CONSTANTS);
		alignment_length = input.column * CONSTANTS.GAP_COEFFICIENT;
	}
	
	allocate_alignment_list(&parents, CONSTANTS.POPULATION_SIZE, input.row, alignment_length);
	allocate_alignment_list(&children, CONSTANTS.POPULATION_SIZE, input.row, alignment_length);
	bit_array = safe_malloc(alignment_length * sizeof(int));
	
	for (k = 0; k < CONSTANTS.POPULATION_NUMBER; k++) {
		initialization(&parents, &input, &solution, &CONSTANTS);
		
		printf("Population %d:\n\n", k + 1);
		printf("Score range of initialized alignments: [%d, %d]\n", parents.list[0].score, parents.list[parents.size - 1].score);
		printf("Alignment with highest score:\n");
		print_alignment(&parents.list[parents.size - 1], &input);
		fflush(stdout);
		gen_percent = 0;
		
		for (i = 0; i < CONSTANTS.GENERATION_NUMBER; i++) {
			children.position = 0;
			
			for (j = 0; j < CONSTANTS.POPULATION_SIZE / 2; j++) {
				p1 = selection(-1, &parents);
				p2 = selection(p1, &parents);
				num = rand() % 2;
				if (num == 0) crossover1(&parents.list[p1], &parents.list[p2], &children);
				if (num == 1) crossover2(&parents.list[p1], &parents.list[p2], &children);				
			}

			for (j = 0; j < CONSTANTS.POPULATION_SIZE; j++) {
				rnum = rand() / RAND_MAX;
				if (rnum < CONSTANTS.MUTATION_PARAMETER) {
					num = rand() % 3;
					if (num == 0) mutation1(&children.list[j], &CONSTANTS);
					if (num == 1) mutation2(&children.list[j], &CONSTANTS, bit_array);
					if (num == 2) mutation3(&children.list[j], &CONSTANTS);
				}
				(*(CONSTANTS.FITNESS_FUNCTION))(&children.list[j], &input, &CONSTANTS);
			}
			
			sort_alignment_list(&children);
			normalize_fitness(&children);
			
			for (j = CONSTANTS.POPULATION_SIZE / 2; j < CONSTANTS.POPULATION_SIZE; j++) {
				copy(&parents.list[j], &children.list[j - CONSTANTS.POPULATION_SIZE / 2]);
			}
			
			i1 = 0;
			i2 = CONSTANTS.POPULATION_SIZE / 2;
			j = 0;
			while (i1 < CONSTANTS.POPULATION_SIZE / 2 && i2 < CONSTANTS.POPULATION_SIZE) {
				if (children.list[i1].score < children.list[i2].score) {
					copy(&children.list[i1], &parents.list[j]);
					i1++;
				}
				else {
					copy(&children.list[i2], &parents.list[j]);
					i2++;
				}
				j++;
			}
			while (i1 < CONSTANTS.POPULATION_SIZE / 2) {
				copy(&children.list[i1], &parents.list[j]);
				i1++;
				j++;
			}
			while (i2 < CONSTANTS.POPULATION_SIZE) {
				copy(&children.list[i2], &parents.list[j]);
				i2++;
				j++;
			}
			gen_percent = print_loading_bar(gen_percent, 100 * (i + 1) / CONSTANTS.GENERATION_NUMBER - gen_percent);
			fflush(stdout);
		}
		parents.position = 0;
		
		printf("\n");
		printf("Score range of alignments in last generation: [%d, %d]\n", parents.list[0].score, parents.list[parents.size - 1].score);
		printf("Alignment with highest score:\n");
		print_alignment(&parents.list[parents.size - 1], &input);
		printf("\n");
		if (k < CONSTANTS.POPULATION_NUMBER - 1) {
			for (l = 0; l < alignment_length; l++) {
				printf("-");
			}
			printf("\n\n");
		}
		fflush(stdout);
	}
	diff = clock() - start;
	sec = diff / CLOCKS_PER_SEC;
	printf("Execution time: %d seconds", sec);
	
	free_constants_type(&CONSTANTS);
	free_data(&input);
	if (CONSTANTS.SOLUTION) {
		free_alignment(&solution);
	}
	free_alignment_list(&parents);
	free_alignment_list(&children);
	free(bit_array);
	return 0;
}

