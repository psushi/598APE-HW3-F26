#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

static void die(const char *s) {
  fprintf(stderr, "%s\n", s);
  exit(1);
}

int **grid;

// you can also use this function to debug print your sandpile
static void render(char *filename, const int N) {
  int chan = 3;
  unsigned char *pixels = (unsigned char *)malloc(N * N * chan * sizeof(char));
  int stride = N * chan;
  if (!pixels)
    die("malloc failed for PNG pixel buffer");
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      unsigned char r = 0, g = 0, b = 0;
      switch (grid[i][j]) {
      case 0: // obsidian
        r = 15;
        g = 15;
        b = 25;
        break;
      case 1: // electric cyan
        r = 0;
        g = 210;
        b = 255;
        break;
      case 2: // vibrant magenta
        r = 230;
        g = 30;
        b = 130;
        break;
      case 3: // bright gold
        r = 255;
        g = 200;
        b = 0;
        break;
      default: // white
        r = 0;
        g = 0;
        b = 0;
      }

      int pid = (i * N + j) * chan;
      pixels[pid] = r;
      pixels[pid + 1] = g;
      pixels[pid + 2] = b;
    }
  }
  if (stbi_write_png(filename, N, N, chan, pixels, stride)) {
    printf("Successfully saved sandpile identity to: %s\n", filename);
  } else {
    die("Error: stb_image_write failed to save png file\n");
  }
  free(pixels);
}

static void find_ranges(const int N, int *first, int *last) {
  const int64_t radius = N / 2;
  for (int i = 0; i < N; ++i) {
    first[i] = N;
    last[i] = -1;
    const int64_t y = i - radius;
    for (int j = 0; j <= MAX(i, N - 1 - i); ++j) {
      const int64_t x = j - radius;
      if (x * x + y * y <= radius * radius) {
        if (first[i] == N)
          first[i] = j;
        last[i] = j;
      }
    }
  }
}

static void stabilize(const int N, const int *first, const int *last) {
  const int folded = N % 2;
  const int last_row = folded ? N / 2 : N - 1;
  const size_t stride = (size_t)N + 2;
  unsigned char *topples = calloc(((size_t)last_row + 3) * stride, 1);
  if (!topples)
    die("malloc failed for toppling flags");
  int changed = 0;
#pragma omp parallel
  {
    for (;;) {
#pragma omp single
      changed = 0;
#pragma omp for schedule(static) reduction(| : changed)
      for (int i = 0; i <= last_row; ++i) {
        unsigned char *sent = topples + (size_t)(i + 1) * stride + 1;
        for (int j = first[i]; j <= last[i]; ++j) {
          sent[j] = grid[i][j] >= 4;
          changed |= sent[j];
        }
      }
      if (!changed)
        break;
#pragma omp for schedule(static)
      for (int i = 0; i <= last_row; ++i) {
        int *row = grid[i];
        const unsigned char *sent = topples + (size_t)(i + 1) * stride + 1;
        const unsigned char *above = sent - stride;
        const unsigned char *below = sent + stride;
        const int above_grains = folded && i == last_row ? 2 : 1;
        for (int j = 0; j < N; ++j)
          row[j] += above_grains * above[j] + below[j] + sent[j - 1]
                    + sent[j + 1] - 4 * sent[j];
      }
    }
  }
  free(topples);
  if (folded) {
    for (int i = 0; i < last_row; ++i)
      memcpy(grid[N - 1 - i], grid[i], N * sizeof(int));
  }
}

void find_identity(const int N) {
  int *first = malloc(N * sizeof(int));
  int *last = malloc(N * sizeof(int));
  if (!first || !last)
    die("malloc failed for row ranges");
  find_ranges(N, first, last);
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      grid[i][j] = 6;
    }
  }
  stabilize(N, first, last);
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      grid[i][j] = 6 - grid[i][j];
    }
  }
  stabilize(N, first, last);
  free(first);
  free(last);
}

int main(int argc, char **argv) {
  if (argc < 2 || argc > 2) {
    fprintf(stderr, "Usage: %s N\n", argv[0]);
    return EXIT_FAILURE;
  }
  char *end;
  uintmax_t parsed = strtoumax(argv[1], &end, 10);
  if (errno || argv[1][0] < '0' || argv[1][0] > '9' || *end || !parsed ||
      parsed > INT32_MAX)
    die("grid size must be a positive int");
  const int N = (int)parsed;

  grid = (int **)malloc(N * sizeof(int *));
  for (int i = 0; i < N; ++i) {
    grid[i] = (int *)malloc(N * sizeof(int));
  }

  find_identity(N);

  render("identity.png", N);
  for (int i = 0; i < N; ++i) {
    free(grid[i]);
  }
  free(grid);
  return EXIT_SUCCESS;
}
