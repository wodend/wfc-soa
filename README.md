# WFC SoA

WFC SoA is an implementation of the
[Wave Function Collapse](https://github.com/mxgmn/WaveFunctionCollapse)
algorithm's overlapping model variant in C using
[SoAs](https://en.wikipedia.org/wiki/AoS_and_SoA).

## Installation

WFC SoA is in under active development. None of the APIs should be considered
stable at this time.  Once complete, WFC SoA will be available as a library. To
use WFC SoA in your project, follow the instructions for Linux/Mac OS below.
WFC SoA has only been tested with Linux using GCC on x86 archtitecture.

1. Clone the respository.

    ```bash
    git clone https://github.com/wodend/wfc-soa.git
    ```

2. Copy the files to your project directory.

    ```bash
    cp wfc-soa/* my-project/
    ```

3. Include the appropriate headers in your project source files.

    ```c
    #include "matrix2.h"
    ```

4. Add the WFC SoA files to your build, see an example build below.

    ```c
    gcc matrix2.c wfc.c util.c my_project.c matrix2.h util.h my_project.h
    ```

## Usage

The Bash commands in this section are for Linux/Mac OS, and may have to be
adapted to work on Windows.

To view the current debugging output, compile with `make` and run the
executable.

```bash
make
chmod +x wfc
./wfc
```

This implementation of the Wave Function Collapse algorithm operates on
matricies rather than bitmaps. So far, only two dimentional matricies are
supported. A `Matrix2` input can be created with the `matrix2_create` function
given a length, width, and a flat data array.

```c
size_t sample_length = 3;
size_t sample_width = 3;
uint16_t sample_data[] = {
	7, 7, 7,
	7, 1, 1,
	7, 1, 0,
};
Matrix2 *sample;
sample = matrix2_create(sample_length, sample_width, sample_data);
```

The `run` function runs the algorithm given a sample matrix input, a tile size,
output length, and output width. It returns a `Matrix2` Wave Function Collapse
result, or `NULL` if the alorithm reached a contradiction.

```c
size_t n = 2;
size_t output_length = 3;
size_t output_width = 3;
Matrix2 *output;
output = run(sample, n, output_length, output_width);
```

The return values of both `matrix2_create` and `run` must be freed by the
caller.

```c
matrix2_free(sample);
matrix2_free(output);
```

## Contributing
Pull requests are welcome. For major changes, please open an issue first to
discuss what you would like to change.

## License
[MIT](https://choosealicense.com/licenses/mit/)
