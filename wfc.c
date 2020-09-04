#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "matrix2.h"
#include "util.h"

#define PATTERN(n, i)		((i) * (n) * (n))

typedef struct {
	size_t length;
	int *x_offsets;
	int *y_offsets;
	size_t *lengths;
	size_t **data;
} Constraints;

typedef struct {
	size_t length;
	size_t n;
	uint16_t *data;
	uint16_t *weights;
	Constraints **constraints;
} Patterns;

/* function declarations */
Matrix2 *run(Matrix2 *sample, size_t n, size_t length, size_t width);
Patterns *patterns_from_sample(Matrix2 *sample, size_t n);
void add_symmetries(uint16_t *data, Patterns *patterns);
uint16_t *get_symmetries(size_t n, uint16_t *pattern);
void pattern_reflect(size_t n, uint16_t *data);
void pattern_rotate(size_t n, uint16_t *data);
bool pattern_equal(size_t n, uint16_t *p1, uint16_t *p2);
void patterns_free(Patterns *patterns);
void constraints_create(Patterns *patterns);
bool compatible(size_t n, uint16_t *a, uint16_t *b, size_t x, size_t y);
uint8_t *wave_create(size_t length, size_t width, size_t patterns_length);
int observe(uint8_t *wave, Patterns *patterns);
void propogate(uint8_t *wave, Patterns *patterns);
Matrix2 *wave_to_matrix2(uint8_t *wave, Patterns *patterns);

/* debugging helpers */
void patterns_print(Patterns *patterns);
void pattern_print(size_t n, uint16_t *pattern);
void constraints_print(size_t i, Patterns *patterns);

/* function implementations */
Matrix2 *
run(Matrix2 *sample, size_t n, size_t length, size_t width)
{
	Patterns *patterns;
	int observed;
	uint8_t *wave;
	Matrix2 *output;

	patterns = patterns_from_sample(sample, n);
	patterns_print(patterns); /* DEBUG */
	constraints_create(patterns);
	constraints_print(0, patterns); /* DEBUG */
	wave = wave_create(length, width, patterns->length);
	do {
		observed = observe(wave, patterns);
		propogate(wave, patterns);
	} while(observed == 1);
	if (observed == -1) {
		return NULL;
	}
	output = wave_to_matrix2(wave, patterns);
	free(wave);
	patterns_free(patterns);
	return output;
}

void
patterns_free(Patterns *patterns)
{
	free(patterns->data);
	free(patterns->weights);
	free(patterns);
	/* TODO: Free constraints */
}

Patterns *
patterns_from_sample(Matrix2 *sample, size_t n)
{
	size_t slices_length, slices_width;
	size_t i, j;
	size_t patterns_length_max;
	uint16_t *slice;
	Patterns *patterns;

	patterns = ecalloc(1, sizeof(Patterns));
	slices_length = sample->length - n + 1;
	slices_width = sample->width - n + 1;
	patterns_length_max = slices_length * slices_width * 8;
	patterns->data = ecalloc(patterns_length_max, n * n * sizeof(uint16_t));
	patterns->weights = ecalloc(patterns_length_max, sizeof(uint16_t));
	patterns->n = n;
	patterns->length = 0;
	for (i = 0; i < slices_length; i++) {
		for (j = 0; j < slices_width; j++) {
			slice = matrix2_slice(sample, n, i, j);
			add_symmetries(slice, patterns);
			free(slice);
		}
	}
	return patterns;
}

void
add_symmetries(uint16_t *data, Patterns *patterns)
{
	bool found;
	size_t i, j;
	size_t symmetries_length;
	uint16_t *symmetries;
	symmetries_length = 8;
	symmetries = get_symmetries(patterns->n, data);
	for (i = 0; i < symmetries_length; i++) {
		/* TODO: pattern hash function for more efficient duplicate check */
		found = false;
		for (j = 0; j < patterns->length; j++) {
			if (pattern_equal(patterns->n,
			                  &patterns->data[PATTERN(patterns->n, j)],
			                  &symmetries[PATTERN(patterns->n, i)])) {
				found = true;
				break;
			}
		}
		if (found) {
			patterns->weights[j]++;
		} else { /* if not found add symmetry to pattern data array */
			memcpy(&patterns->data[PATTERN(patterns->n, patterns->length)],
				   &symmetries[PATTERN(patterns->n, i)],
				   patterns->n * patterns->n * sizeof(uint16_t));
			patterns->weights[patterns->length] = 1;
			patterns->length++;
		}
	}
	free(symmetries);
}

uint16_t *
get_symmetries(size_t n, uint16_t *pattern)
{
	size_t i;
	size_t pattern_data_size;
	uint16_t *symmetries;

	pattern_data_size = n * n * sizeof(uint16_t);
	symmetries = ecalloc(8, pattern_data_size);
	memcpy(&symmetries[PATTERN(n, 0)], pattern, pattern_data_size);
	for (i = 0; i < 5; i += 2) {
		/* add reflection of current pattern */
		memcpy(&symmetries[PATTERN(n, i+1)],
		       &symmetries[PATTERN(n, i)],
		       pattern_data_size);
		pattern_reflect(n, &symmetries[PATTERN(n, i+1)]);
		/* add rotation of current pattern */
		memcpy(&symmetries[PATTERN(n, i+2)],
		       &symmetries[PATTERN(n, i)],
		       pattern_data_size);
		pattern_rotate(n, &symmetries[PATTERN(n, i+2)]);
	}
	memcpy(&symmetries[PATTERN(n, i+1)],
		   &symmetries[PATTERN(n, i)],
		   pattern_data_size);
	pattern_reflect(n, &symmetries[PATTERN(n, i+1)]);
	return symmetries;
}

void
pattern_reflect(size_t n, uint16_t *data)
{
	size_t i, j;
	uint16_t temp;
	for (i = 0; i < n; i++) {
		for (j = 0; j < n / 2; j++) {
			temp = data[GET_2D(n, i, j)];
			data[GET_2D(n, i, j)] = data[GET_2D(n, i, n-j-1)];
			data[GET_2D(n, i, n-j-1)] = temp;
		}
	}
}

void
pattern_rotate(size_t n, uint16_t *data)
{
	size_t i, j;
	uint16_t temp;
	/* Rotate from the outer edges in */
	for (i = 0; i < n / 2; i++) {
		for (j = i; j < n - i - 1; j++) {
			temp = data[GET_2D(n, i, j)];
			data[GET_2D(n, i, j)] = data[GET_2D(n, j, n-i-1)];
			data[GET_2D(n, j, n-i-1)] = data[GET_2D(n, n-i-1, n-j-1)];
			data[GET_2D(n, n-i-1, n-j-1)] = data[GET_2D(n, n-j-1, i)];
			data[GET_2D(n, n-j-1, i)] = temp;
		}
	}
}

bool
pattern_equal(size_t n, uint16_t *p1, uint16_t *p2)
{
	size_t i;

	for (i = 0; i < n * n; i++) {
		if (p1[i] != p2[i]) {
			return false;
		}
	}
	return true;
}

/* Work in progress*/
void
constraints_create(Patterns *patterns)
{
	size_t i, j;
	size_t n;
	size_t x_offset, y_offset;
	size_t constraints_length_max;
	size_t pattern_size;
	Constraints **constraints;
	Constraints *current;

	n = patterns->n;
	constraints_length_max = (size_t) pow(2 * n - 2, 2);
	pattern_size = n * n * sizeof(uint16_t);
	constraints = ecalloc(patterns->length, sizeof(Constraints *));
	for (i = 0; i < patterns->length; i++) {
		current = ecalloc(1, sizeof(Constraints));
		current->length = constraints_length_max;
		current->x_offsets = ecalloc(constraints_length_max, sizeof(int));
		current->y_offsets = ecalloc(constraints_length_max, sizeof(int));
		current->lengths = ecalloc(constraints_length_max, sizeof(size_t));
		current->data = ecalloc(constraints_length_max, sizeof(size_t *));
		for (x_offset = -n-1; x_offset < n-1; x_offset++) {
			for (y_offset = -n-1; y_offset < n-1; y_offset++) {
				current->x_offsets[i] = x_offset;
				current->y_offsets[i] = y_offset;
				current->lengths[i] = 0;
				current->data[i] = ecalloc(patterns->length, pattern_size);
				for (j = 0; j < patterns->length; j++) {
					if (compatible(n,
					               &patterns->data[PATTERN(n, i)],
					               &patterns->data[PATTERN(n, j)],
					               x_offset,
					               y_offset)) {
						memcpy(&current->data[i][PATTERN(n, current->lengths[i])],
						       &patterns->data[PATTERN(n, j)],
						       pattern_size);
						current->lengths[i]++;
					}
				}
			}
		}
		constraints[i] = current;
	}
	patterns->constraints = constraints;
}

bool
compatible(size_t n, uint16_t *a, uint16_t *b, size_t x, size_t y)
{
	size_t i, j, a_i, a_j;
	size_t i0, j0, in, jn;

	i0 = MAX(0, -x);
	j0 = MAX(0, -y);
	in = MIN(n, n-x);
	jn = MIN(n, n-y);
	for (i = i0; i < in; i++) {
		for (j = j0; j < jn; j++) {
			a_i = i + x;
			a_j = j + y;
			if (a[GET_2D(n, a_i, a_j)] != b[GET_2D(n, i, j)]) {
				return false;
			}
		}
	}
	return true;
}

uint8_t *wave_create(size_t length, size_t width, size_t patterns_length)
{
	printf("wave_create() is not yet implemented.\n");
	return NULL;
}

int observe(uint8_t *wave, Patterns *patterns)
{
	printf("observe() is not yet implemented.\n");
	return -1;
}

void propogate(uint8_t *wave, Patterns *patterns)
{
	printf("propogate() is not yet implemented.\n");
}

Matrix2 *wave_to_matrix2(uint8_t *wave, Patterns *patterns)
{
	printf("wave_to_matrix2() is not yet implemented.\n");
	return NULL;
}

void
pattern_print(size_t n, uint16_t *pattern)
{
	size_t i, j;

	for (i = 0; i < n; i++) {
		for (j = 0; j < n; j++) {
			printf("%d ", pattern[GET_2D(n, i, j)]);
		}
		printf("\n");
	}
	printf("\n");
}

void
patterns_print(Patterns *patterns)
{
	size_t i;

	for (i = 0; i < patterns->length; i++) {
		printf("id: %lu\nweight: %d\npattern:\n", i, patterns->weights[i]);
		pattern_print(patterns->n, &patterns->data[PATTERN(patterns->n, i)]);
	}
}

void
constraints_print(size_t id, Patterns *patterns)
{
	size_t i, j, length, pattern_id;

	for (i = 0; i < patterns->constraints[id]->length; i++) {
		length = patterns->constraints[id]->lengths[i];
		printf("x_offset: %d y_offset: %d length: %lu\npatterns:\n",
		       patterns->constraints[id]->x_offsets[i],
		       patterns->constraints[id]->y_offsets[i],
		       length);
		for (j = 0; j < length; j++) {
			pattern_id = patterns->constraints[id]->data[i][j];
			pattern_print(patterns->n, &patterns->data[PATTERN(patterns->n, pattern_id)]);
		}
	}
}

int
main(int argc, char *argv[])
{
	size_t sample_length = 3;
	size_t sample_width = 3;
	uint16_t sample_data[] = {
		7, 7, 7,
		7, 1, 1,
		7, 1, 0,
	};
	size_t n = 2;
	//size_t sample_length = 4;
	//size_t sample_width = 5;
	//uint16_t sample_data[] = {
	//	7, 7, 7, 7, 1,
	//	7, 0, 0, 0, 2,
	//	7, 0, 1, 0, 3,
	//	7, 0, 0, 0, 4
	//};
	//size_t n = 3;
	size_t output_length = 16;
	size_t output_width = 16;
	Matrix2 *sample, *output;

	sample = matrix2_create(sample_length, sample_width, sample_data);
	output = run(sample, n, output_length, output_width);
	matrix2_free(sample);
	if (!output) {
		die("model reached contradiction.");
	}
	matrix2_print(output);
	matrix2_free(output);
	return EXIT_SUCCESS;
}
