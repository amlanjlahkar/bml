#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>

int main(int argc, char **argv) {

  puts("Toosty version 0.0.1");

  while (true) {
    char *ibuffer = readline("toosty> ");
    add_history(ibuffer);
    printf("%s\n", ibuffer);
    free(ibuffer);
  }
  return EXIT_SUCCESS;
}
