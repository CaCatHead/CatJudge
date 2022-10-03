#include <cstdio>

char buf[1024];

int main() {
  freopen("./out.out", "r", stdin);
  while (scanf("%[^\n]", buf) != -1) {
    printf("%s\n", buf);
  }
  return 0;
}