#include <cstdio>
#include <cstdlib>

int main() {
  char *pathvar = getenv("PATH");
  if (pathvar == nullptr) {
    return 1;
  } else {
    printf("pathvar = %s", pathvar);
    return 0;
  }
}
