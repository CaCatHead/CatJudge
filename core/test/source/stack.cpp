#include <iostream>

using namespace std;

const int mod = 1e9 + 7;

int dfs(int u, int v) {
  if (u == v) return u;
  return (dfs(u, v - 1) + 1 + u + v) % mod;
}

int main() {
  int a = 1, b = 1;
  cin >> a >> b;
  cout << dfs(min(a, b), max(a, b)) << '\n';
  return 0;
}