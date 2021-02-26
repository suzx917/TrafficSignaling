#include <bits/stdc++.h>

using namespace std;

template <typename T>
inline void prop(T& t) {
  cerr << "[" << t << "]\n";
}

struct Street {
  int start;
  int end;
  string name;
  int cost;

  list<pair<int,int>> cars; // ( car_id, dist to end )

  Street() = default;

  Street(int B, int E, string nm, int C)
  : start{B}, end{E}, name{nm}, cost{C} {
  }

  void take_car(int cid) {
    cars.push_back(make_pair(cid, cost));
  }
};

inline ostream& operator<<(ostream& os, Street const& st) {
    os << "Street: " << st.start << "->" << st.end
      << " cost=" << st.cost << ", " << st.name ;

    return os;
}

struct Intersection {
  vector<int> in; // in street ids
  vector<int> out; // out street ids

  inline int indeg() {return in.size();}
  inline int outdeg() {return out.size();}
};

class Traffic {
public:
  int D, I, S, V, F, B, E, C;
  string stname;
  map<string, int> stid;
  vector<Street> street;
  vector<vector<int>> cpath;

  vector<Intersection> inter; // adj list of arcs (in/out street ids)

  vector<int> lights; // for each intersection indexing to current phase pair (src_id, duration)
                      // ans[i][ lights[i] ].first = green light src id

  vector<int> clocks; // cycling timer

  vector<vector<pair<int,int>>> ans; // ans[inter_id] = list of pairs < src_street_id, duration >

  void output(ostream& os) {
    os << I << '\n';
    for (int i = 0; i < I; ++i) {
      os << i << '\n';
      auto& row = ans[i];
      os << row.size() << '\n';
      for (auto const& p: row) {
          os << street[p.first].name << ' ' << p.second << '\n';
      }
    }
  }

  void init() {
    lights = vector<int>(I,-1);
    clocks = vector<int>(I,-1);
    // set lights
    for (int i = 0; i < I; ++i) {
      if (!ans[i].empty()) {
        lights[i] = 0;
        clocks[i] = 0;
      }
    }
  }

  long simulate() {
    init();
    long scr = 0;

    // D total epochs
    for (int t = 0; t <= D; ++t) {
      // cerr << ">> Epoch " << t << '\n';

      static vector<int> progress(V, 0); // mark car pos = index of path
      static vector<bool> updated_cars(V, false);

      // Moving cars by intersections (green lights)
      for (int j = 0; j < I; ++j) {
        if (ans[j].empty())
          continue;
        // get green light street
        int src = ans[j][lights[j]].first;

        if (!street[src].cars.empty()) { // have car
          auto [car, dist] = street[src].cars.front(); // get head car
          if ( !updated_cars[car] ) { // not updated
            // special case: reaching final dest
            if (dist == 1 && progress[car] == static_cast<int>(cpath[car].size()-1)) {
              const long reward = F + D - t;
              scr += reward;
              // destroy
              street[src].cars.pop_front();
              // cerr << "Inter-recycle: [score = " << scr << "] (+" << reward << ")\n"; 
              continue;
            }
            // else: was queueing or just arriving
            else if (dist == 1 || dist == 0) {
              ++progress[car];
              int dest = cpath[car][progress[car]];

              // cerr << "Moving #" << car << ' ' << street[src].name << " -> " << street[dest].name << '\n';

              street[src].cars.pop_front();
              street[dest].take_car(car);
              updated_cars[car] = true;
            }
          }
        }
      }

      // Moving by streets
      for (int j = 0; j < S; ++j) {
        auto& cars = street[j].cars;
        if (cars.empty()) continue;

        auto iter = cars.begin();
        while(iter != cars.end()) {
          auto [car, dist] = *iter;
          // skip updated car
          if (updated_cars[car]) {
            ++iter;
            continue;
          }

          if (dist != 0) {
            --dist;
          }
          if (progress[car] == static_cast<int>(cpath[car].size()-1) && dist == 0) {
            const long reward = F + D - t;
            scr += reward;
            iter = cars.erase(iter);
            // cerr << "[score = " << scr << "] (+" << reward << ")\n"; 
          }
          else {
            *iter = make_pair(car,dist);
            ++iter;
            // cerr << "Car #" << car << " on " << street[j].name << " (" << dist << ")\n";
          }
        }
      }

      // Reset update list
      for (int j = 0; j < V; ++j) {
        updated_cars[j] = false;
      }

      // Update lights and timers
      for (int j = 0; j < I; ++j) {
        if (!ans[j].empty()) {
          ++clocks[j];
          if (clocks[j] == ans[j][lights[j]].second) {
            // increment index and wrap around 
            ++lights[j];
            if (lights[j] == static_cast<int>(ans[j].size()))
              lights[j] = 0;
            // reset clock
            clocks[j] = 0;
          }
        }
      }

    } // End epoch

    return scr;
  }

  void setup1() { // only green even intersections
    for (int i = 0; i < I; i+=2) {
      auto& row = inter[i].in;
      for (int j = 0; j < (int) row.size(); ++j) {
        ans[i].push_back(make_pair(row[j], 1));
      }
    }
  }

  void randomize(int seed) {
    srand(seed);
    for (int i = 0; i < I; ++i) {
      auto& row = inter[i].in;
      for (int j = 0; j < (int) row.size(); ++j) {
        if (rand() ^ (i*j) % 3)
          ans[i].push_back(make_pair(row[j], rand() % row.size() + 1));
        else
          ans[i].push_back(make_pair(row[j], 2));
      }
    }
  }

  void naive(int k) {
    for (int i = 0; i < I; ++i) {
      auto& row = inter[i].in;
      for (int j = 0; j < (int) row.size(); ++j) {
        ans[i].push_back(make_pair(row[j], k));
      }
    }
  }

  void improve() {
    naive(2);
    long max = simulate();
    prop(max);
    const int rep = 3000;
    for (int i = 1; i <= rep; ++i) {
      randomize(0);
      const long scr = simulate();
      if (scr > max) {
        max = scr;
        prop(scr);
        ofstream ofs("test.out");
        output(ofs);
      }
    }
  }

  Traffic() {
    // Read input
    cin >> D >> I >> S >> V >> F; // [D]uration, [I]ntersections,
                                  // [S]treets, [V]ehicles, [F]cking bonus

    street= vector<Street>(S);
    cpath = vector<vector<int>>(V, vector<int>());

    for (int i = 0; i < S; ++i) {
      cin >> B >> E >> stname >> C;
      stid[stname] = i;
      street[i] = Street(B,E,stname,C);
    }

    int P;
    for (int i = 0; i < V; ++i) {
      cin >> P;
      for (int j = 0; j < P; ++j) {
        cin >> stname;
        const int sid = stid[stname];
        cpath[i].push_back(stid[stname]);
        if (j == 0) { // starting street
          street[sid].cars.push_back(make_pair(i, 0));
        }
      }
    }
    // Initialize answer
    ans = vector<vector<pair<int,int>>>(I, vector<pair<int,int>>());

    // Preprocess
    inter = vector<Intersection>(I);

    for (int i = 0; i < S; ++i) {
      Street& st = street[i];
      inter[st.start].out.push_back(i);
      inter[st.end].in.push_back(i);
      //cerr << st << '\n';
    }

    // cout << I << '\n';
    // for (int i = 0; i < I; ++i) {
    //   cout << i << '\n';
    //   vector<int>& row = inter[i].in;
    //   cout << row.size() << '\n';
    //   for (int i: row) {
    //       cout << street[i].name << ' ' << 1 << '\n';
    //   }
    // }

    improve();

  }
};

int main() {
  // ios_base::sync_with_stdio(false);
  // cin.tie(NULL);
  Traffic t = Traffic();
  return 0;
}