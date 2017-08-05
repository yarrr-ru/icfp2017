#include <vector>
#include <numeric>

class Dsu {
public:
  Dsu(size_t n): color(n) {
    std::iota(color.begin(), color.end(), 0);
  }

  size_t get(size_t u) {
    if (color[u] == u) {
      return u;
    }
    return color[u] = get(color[u]);
  }

  bool join(size_t u, size_t v) {
    u = get(u);
    v = get(v);
    if (u == v) {
      return false;
    }
    color[u] = v;
    return true;
  }

private:
  std::vector<size_t> color;
};

