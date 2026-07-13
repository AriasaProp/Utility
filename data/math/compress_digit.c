#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#define BUFF_LEN 512

int main (int argc, char **argv) {
  if (argc < 3) {
    printf("Usage: compress_digit <input> <output>\n");
    return 1;
  }
  printf("Compress with Input: %s, Output: %s\n", argv[1], argv[2]);
  FILE *input = fopen(argv[1],"rb");
  if (!input) {
    printf("File input: %s\n", strerror(errno));
    return 1;
  }
  FILE *output = fopen(argv[2],"wb");
  if (!output) {
    printf("File output: %s\n", strerror(errno));
    fclose(input);
    return 1;
  }
  size_t read = 0, write, i, j;
  size_t writen = 0;
  char loadin[BUFF_LEN];
  do {
    for (read = 0;
      !feof(input) && (read < BUFF_LEN);
      read += fread(loadin + read, BUFF_LEN - read, 1, input)
    ) ;
    for (i = 0, j = 0; i < read; i += 2, ++j) {
      loadin[j] = ((loadin[  i  ] - '0') & 0xf)
                | (((loadin[i + 1] - '0') & 0xf) << 4);
      printf("%02hhx ", loadin[j]); 
    }
    for (write = 0;
      (write < j);
      write += fwrite(loadin + write, j - write, 1, output)
    ) ;
    writen += j;
    fflush(stdout);
    printf("Writen: %08zu", writen);
  } while (!feof(input));
  printf("\n");
  fclose(input);
  fclose(output);
  return 0;
}
