#include "launcher.h"
#include "greed.h"
#include <algorithm>
#include <vector>
#include <deque>

#define all(v) (v).begin(), (v).end()
#define forn(i, n) for (int i = 0; i < (int)(n); ++i)
#define eb emplace_back
#define pb push_back
#define fi first
#define se second

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

  std::pair<vi, vi> get(int v) {
    vi d(n, n), p(n, -1);
    d[v] = 0;
    deque<int> q;
    q.push_back(v);
    while (!q.empty()) {
      int v = q.front(); q.pop_front();
      for (auto& e: g[v]) {
        int u = e.fi;
        int w = d[v] + e.se;
        if (d[u] > w) {
          d[u] = w;
          p[u] = v;
          if (w) q.push_back(u);
          else q.push_front(u);
        }
      }
    }
    return {d, p};
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
        bfs.add_edge(e.from, e.to, 0);
        c = inf_flow;
      } else if (e.owner == kNoOwner) {
        bfs.add_edge(e.from, e.to, 1);
        c = 1;
      } else {
        c = 0;
      }
      if (c > 0) {
        // cerr << "add_edge " << e.from << ' ' << e.to << ' ' << c << '\n';
        mf.add_edge(e.from, e.to, c);
      }
    }
  }
  int k = map.lambda_vertices.size();
  vi lambda_max_score(k, 0);
  vi lambda_flow_score(k, 0);
  vvi lf(k, vi(k, 0));
  vvi bf(k, vi(k, 0));
  vvi bf_all(k, vi(n, 0));

  forn(i, k) {
    int v = map.lambda_vertices[i];
    lambda_max_score[i] = map.get_lambda_max_score(v);
    auto dp = bfs.get(v);
    bf_all[i] = dp.fi;
    forn(j, k) {
      int u = map.lambda_vertices[j];
      bf[i][j] = dp.fi[u];
      // cerr << "bf " << v << ' ' << u << ' ' << i << ' ' << j << ' ' << bf[i][j] << '\n';
    }
  }

  vector<tuple<int, int, int, int, pii>> best_connection;
  forn(i, k) forn(j, i) {
    int v = map.lambda_vertices[i];
    int u = map.lambda_vertices[j];
    mf.clear();
    int f = lf[i][j] = lf[j][i] = mf.go(v, u);
    lambda_flow_score[i] += f;
    lambda_flow_score[j] += f;
    if (0 < f && f < inf_flow) {
      best_connection.emplace_back(-f, bf[i][j], 
          -lambda_flow_score[i]-lambda_flow_score[j],
          -lambda_max_score[i]-lambda_max_score[j], pii(v, u));
    }
  }
  sort(all(best_connection));
  for(auto& e: best_connection) {
    cerr << "cand " << std::get<0>(e) << 
      ' ' << std::get<1>(e) << 
      ' ' << std::get<2>(e) << 
      ' ' << std::get<3>(e) << 
      ' ' << std::get<4>(e).fi << 
      ' ' << std::get<4>(e).se << '\n';
  }

  if (count(all(map.river_owners), map.punter) >= 1) {
    ;
  } else if (!best_connection.empty()) {
    int v = std::get<4>(best_connection[0]).fi;
    int u = std::get<4>(best_connection[0]).se;
    int d = std::get<1>(best_connection[0]);
    auto e = get_worst_edge(v, u, map);
    if (e.from == 0 && e.to == 0) {
      cerr << "omg\n";
    } else {
      cerr << "use worst_edge " << v << " " << u << " edge: " << e.from << ' ' << e.to << '\n';
      forn(i, k) {
        if (bf_all[i][v] + bf_all[i][u] == d) {
          cerr << "use lambda " << i << " " << map.lambda_vertices[i] << '\n';
          Edge best_z{};
          for(auto& e: map.graph[map.lambda_vertices[i]]) {
            if (e.owner != kNoOwner)
              continue;
            int z = e.to;
            auto dp = bfs.get(z);
            int dd = dp.fi[v] + dp.fi[u];
            cerr << "cand " << z << " dd: " << dd << " d: " << d << '\n';
            best_z = e;
          }
          if (best_z.from == 0 && best_z.to == 0) {
            cerr << "omg best_z\n";
          } else {
            return map.get_river(best_z);
          }
        }
      }
      return map.get_river(e);
    }
  }
  River r = make_move_greed_only_st(map);
  if (r.first != r.second) {
    return r;
  }
  return make_move_greed(map);
}

int main() {
  return main_launcher(make_move_flow);
}
