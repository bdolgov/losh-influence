#include <vector>
#include <utility>
#include <map>
#include <cstdlib>

using namespace std;

typedef long long ll;
typedef long double ld;
typedef pair<int,int> pi;
typedef vector<int> vi;


#define all(c) (c).begin(),(c).end()
#define sz(c) (int)(c).size()

#define pb push_back
#define mp make_pair
#define fs first
#define sc second
#define x first
#define y second
#define y1 y1_gdssdfjsdgf
#define y0 y0_fsdjfsdogfs
#define ws ws_fdfsdfsdgfs
#define image(a) {sort(all(a)),a.resize(unique(all(a))-a.begin());}
#define eprintf(...) {fprintf(stderr,__VA_ARGS__),fflush(stderr);}

#define forn(i,n) for( int i = 0 ; i < (n) ; i++ )
#define forit(it,c) for( __typeof((c).begin()) it = (c).begin() ; it != (c).end() ; it++ )
#define problem_name "a"
int n, p;
vector<pair<pair<int, int>, pair<int, int> > > ls;
map<pair<int, int>, vector<pair<int, int> > > G;
map<pair<int, int>, int> was;
void add(pair<int, int> a, pair<int, int> b) {
  ls.pb(mp(a, b));
}
void add2(pair<pair<int, int>, pair<int, int> > a) {
  G[a.x].pb(a.y);
  G[a.y].pb(a.x);
}
bool dfs(pair<int, int> v1, pair<int, int> v2) {
  if (v1 == v2) return true;
  was[v1] = 1;
  for (auto x : G[v1]) {
    if (was[x] == 0) {
      if (dfs(x, v2)) return true;
    }
  }
  return false;
}
bool united(pair<int, int> v1, pair<int, int> v2) {
  was.clear();
  return dfs(v1, v2);
}
int main(){  
  srand(time(0));
  scanf("%d %d", &n, &p);
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      if (i > 0) {
        add(mp(i - 1, j), mp(i, j));
      }
      if (j > 0) {
        add(mp(i, j - 1), mp(i, j));
      }
      if (i % 2 == 0 && j > 0 && i + 1 < n) {
        add(mp(i, j), mp(i + 1, j - 1));
      }
      if (i % 2 == 1 && j + 1 < n && i + 1 < n) {
        add(mp(i, j), mp(i + 1, j + 1));
      }
    }
  }
  random_shuffle(all(ls));
  int k = sz(ls) * p / 100;
  for (int i = 0; i < k; i++) {
    add2(ls[i]);
  }
  for (int i = k; i < sz(ls); i++) {
    if (!united(ls[i].x, ls[i].y)) {
      add2(ls[i]);
    }
  }
  printf("%d\n", n * n);
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      printf("%d %d %d %d", i, j, rand() % (21 - 8) + 8, sz(G[mp(i, j)]));
      for (auto x : G[mp(i, j)]) {
        printf(" %d %d", x.x, x.y);
      }
      printf("\n");
    }
  }
  for (int i = 0; i < 4; i++) {    
    int x = rand() % (n / 4);
    if (i & 1) x += 3 * n / 4;
    int y = rand() % (n / 4);
    if ((i >> 1) & 1) y += 3 * n / 4;      
    printf("%d %d ", x, y);
  }
  return 0;
}
