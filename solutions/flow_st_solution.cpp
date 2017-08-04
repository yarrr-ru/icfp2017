#include "launcher.h"
#include <algorithm>
#include <vector>

#define all(v) (v).begin(), (v).end()
#define forn(i, n) for (int i = 0; i < (int)(n); ++i)

using namespace std;

typedef pair<int, int> pii;
typedef vector<pii> vpi;
typedef vector<vpi> vvpi;
typedef vector<int> vi;
typedef vector<vi> vvi;

const int inf = 1e9;

struct edge {
	int a, b, cap, flow;
};
 
struct MaxFlow {
  int s, t, n;
  vi d, ptr, q;
  vector<edge> e;
  vvi g;

  MaxFlow(int n_) {
    n = n_;
    d.resize(n);
    ptr.resize(n);
    q.resize(n);
    e.clear();
    g.assign(n, vi());
  }

  void add_edge (int a, int b, int cap) {
    edge e1 = { a, b, cap, 0 };
    edge e2 = { b, a, cap, 0 };
    g[a].push_back ((int) e.size());
    e.push_back (e1);
    g[b].push_back ((int) e.size());
    e.push_back (e2);
  }

  void clear() {
    for(auto& edge: e) edge.flow = 0;
  }

  int go(int s_, int t_) {
    s = s_; t = t_;
    return dinic();
  }

  bool bfs() {
    int qh=0, qt=0;
    q[qt++] = s;
    fill(all(d), -1);
    d[s] = 0;
    while (qh < qt && d[t] == -1) {
      int v = q[qh++];
      for (size_t i=0; i<g[v].size(); ++i) {
        int id = g[v][i],
          to = e[id].b;
        if (d[to] == -1 && e[id].flow < e[id].cap) {
          q[qt++] = to;
          d[to] = d[v] + 1;
        }
      }
    }
    return d[t] != -1;
  }
 
  int dfs (int v, int flow) {
    if (!flow)  return 0;
    if (v == t)  return flow;
    for (; ptr[v]<(int)g[v].size(); ++ptr[v]) {
      int id = g[v][ptr[v]],
        to = e[id].b;
      if (d[to] != d[v] + 1)  continue;
      int pushed = dfs (to, min (flow, e[id].cap - e[id].flow));
      if (pushed) {
        e[id].flow += pushed;
        e[id^1].flow -= pushed;
        return pushed;
      }
    }
    return 0;
  }
   
  int dinic() {
    int flow = 0;
    for (int iter = 0; iter < 1000; ++iter) {
      // cerr << "dinic " << flow << '\n';
      if (!bfs())  break;
      fill(all(ptr), 0);
      int p_iter = 0;
      while (int pushed = dfs (s, inf)) {
        // cerr << "pushed " << pushed << '\n';
        flow += pushed;
        if (++p_iter >= 1000) break;
      }
    }
    return flow;
  }
};

struct Bfs {
  int n;

  Bfs(int n_) :n(n_) {
    g.assign(n, vpi());
  }

  void add_edge(int v, int u, int c) {
    g[v].eb(u, c);
    g[u].eb(v, c);
  }

  vi get(int v) {
    vi d(n, n);
    d[v] = 0;
    deque<int> q;
    q.push_back(v);
    while (!q.empty()) {
      int v = q.front(); q.pop_front();
      for (auto& e: g[v]) {
        int u = e.fi;
        int w = d[v] + e.se;
        if (d[u] > d[v] + w) {
          d[u] = d[v] + w;
          if (w) q.push_back(u);
          else q.push_front(u);
        }
      }
    }
    return d;
  }

  vvpi g;
};

River make_move_flow(const Map& map) {
  int n = map.graph.size();
  int inf_flow = 10000;
  MaxFlow mf(n);
  Bfs bfs(n);

  for(auto& g: map.graph) {
    for(auto& e: g) {
      if (e.from >= e.to) continue;
      int c;
      if (e.owner == map.punter) {
        bf.add_edge(e.from, e.to, 0);
        c = inf_flow;
      } else if (e.owner == kNoOwner) {
        bf.add_edge(e.from, e.to, 1);
        c = 1;
      } else {
        c = 0;
      }
      // cerr << "add_edge " << e.from << ' ' << e.to << ' ' << c << '\n';\
      mf.add_edge(e.from, e.to, c);
    }
  }
  int k = map.lambda_vertices.size();
  vvi lf(k, vi(k, 0));
  vvi bf(k, vi(k, 0));

  // forn(i, k) {
  //   int v = map.lambda_vertices[i];
  //   vi d = bfs.get(v);
  // }

  vector<tuple<int, int, pii>> bf;
  forn(i, k) forn(j, i) {
    mf.clear();
    int v = map.lambda_vertices[i];
    int u = map.lambda_vertices[j];
    int f = lf[i][j] = lf[j][i] = mf.go(v, u);
    if (f < inf_flow) {
      bf.emplace(-f, bf[i][j], pii(v, u));
    }
  }
  sort(all(bf));
  for(auto& e: bf) {
    cerr << "cand " << get<0>(e) << ' ' << get<1>(e) << ' '
      << get<2>(e).fi << ' ' << get<2>(e).se << '\n';
  }

  if (!bf.empty()) {
    int v = get<2>(bf[0]).fi;
    int u = get<2>(bf[0]).se;
  }

  auto river_owners = map.river_owners;

  int64_t max_score = -1;
  River best_river;

  for (size_t river_index = 0; river_index < river_owners.size(); ++river_index) {
    if (river_owners[river_index] != kNoOwner) {
      continue;
    }

    river_owners[river_index] = map.punter;
    auto score = map.get_score_by_river_owners(river_owners, map.punter);
    river_owners[river_index] = kNoOwner;
    if (score > max_score) {
      max_score = score;
      best_river = map.get_river(river_index);
    }
  }
  assert(max_score >= 0);
  return best_river;
}

int main() {
  return main_launcher(make_move_flow);
}
