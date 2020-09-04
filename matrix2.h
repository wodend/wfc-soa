#define GET_2D(w, i, j)		((w) * (i) + (j))

typedef struct {
	size_t length;
	size_t width;
	uint16_t *data;
} Matrix2;

Matrix2 *matrix2_create(size_t length, size_t width, uint16_t *data);
uint16_t *matrix2_slice(Matrix2 *matrix2, size_t n, size_t i0, size_t j0);
void matrix2_print(Matrix2 *matrix2);
void matrix2_free(Matrix2 *matrix2);
