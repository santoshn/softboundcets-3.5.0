#include <stdlib.h>

int main() {
  void *x = malloc(20);
  free(x);
  free(x);
}
