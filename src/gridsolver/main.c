#include <assert.h>
#include <err.h>
#include <fcntl.h>
#include <solver.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

static void print_help(void) {
  printf("Solver - Find the location of a word in a grid\n"
         "Usage: solver path word\n");
}

static void to_upper(char str[static 1]) {
  for (; *str; ++str)
    if (*str >= 'a' && *str <= 'z') {
      *str += 'A' - 'a';
    }
}

static int test_solver(const char *grid[static 1], const char *word, int rows,
                       int cols) {
  const char *list[] = {word};

  resolve(list, grid, sizeof(list) / sizeof(*list), rows, cols);
  printf("\n");
  return 1;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Error while trying to read the passed arguments\n");
    print_help();
    return 1;
  }

  struct stat st;

  int fd = open(argv[1], O_RDONLY);
  if (fd < 0) {
    errx(EXIT_FAILURE, "Error opening file %s", argv[1]);
  }
  if (fstat(fd, &st) == -1) {
    goto error;
  }

  char *data = mmap(NULL, (size_t)st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (data == NULL) {
    goto error2;
  }

  int cols = 0;
  while (data[cols] != '\n') {
    cols += 1;
  }
  int rows = (int)(st.st_size /
                   (cols + 1)); // cols+1 because we include the newline too

  const char **grid = (const char **)calloc((size_t)rows, sizeof(char *));

  for (int i = 0; i < rows; ++i) {
    grid[i] = &data[(ptrdiff_t)((cols + 1) * i)];
  }

  printf("R: %d, C: %d\n", rows, cols);
  char *word = argv[2];
  to_upper(word);
  test_solver(grid, word, rows, cols);

  free((void *)grid);
  munmap(data, (size_t)st.st_size);
  close(fd);
  return 0;

error2:
  if (data != NULL) {
    munmap(data, (size_t)st.st_size);
  }
error:
  close(fd);
  return EXIT_FAILURE;
}
