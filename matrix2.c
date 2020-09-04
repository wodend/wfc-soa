#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "matrix2.h"
#include "util.h"

Matrix2 *
matrix2_create(size_t length, size_t width, uint16_t *data)
{
	Matrix2 *matrix2;
	matrix2 = ecalloc(1, sizeof(matrix2));
	matrix2->length = length;
	matrix2->width = width;
	matrix2->data = ecalloc(length * width, sizeof(uint16_t));
	memcpy(matrix2->data, data, length * width * sizeof(uint16_t));
	return matrix2;
}

uint16_t *
matrix2_slice(Matrix2 *matrix2, size_t n, size_t i0, size_t j0)
{
	uint16_t *slice;
	size_t slice_i, i, j;

	slice = ecalloc(n * n, sizeof(uint16_t));
	slice_i = 0;
	for (i = i0; i < i0+n; i++) {
		for (j = j0; j < j0+n; j++) {
			slice[slice_i] = matrix2->data[GET_2D(matrix2->width, i, j)];
			slice_i++;
		}
	}
	return slice;
}

void
matrix2_print(Matrix2 *matrix2)
{
	size_t i, j;

	for (i = 0; i < matrix2->length; i++) {
		for (j = 0; j < matrix2->width; j++) {
			printf("%d ", matrix2->data[GET_2D(matrix2->width, i, j)]);
		}
		printf("\n");
	}
	printf("\n");
}

void
matrix2_free(Matrix2 *matrix2)
{
	free(matrix2->data);
	free(matrix2);
}
