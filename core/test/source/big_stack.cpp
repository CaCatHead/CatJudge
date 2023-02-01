#include <stdio.h>

const int max_size = 1024 * 1024 * 1024;

int main() {
  int big_stack[max_size] = {0};
  scanf("%d%d", &big_stack[0], &big_stack[max_size - 1]);
  printf("%d\n", big_stack[0] + big_stack[max_size - 1]);
  return 0;
}
