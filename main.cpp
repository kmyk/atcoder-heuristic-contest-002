#include <bits/stdc++.h>
#define REP(i, n) for (int i = 0; (i) < (int)(n); ++ (i))
#define REP3(i, m, n) for (int i = (m); (i) < (int)(n); ++ (i))
#define REP_R(i, n) for (int i = (int)(n) - 1; (i) >= 0; -- (i))
#define REP3R(i, m, n) for (int i = (int)(n) - 1; (i) >= (int)(m); -- (i))
#define ALL(x) std::begin(x), std::end(x)
using namespace std;

class xor_shift_128 {
public:
    typedef uint32_t result_type;
    xor_shift_128(uint32_t seed = 42) {
        set_seed(seed);
    }
    void set_seed(uint32_t seed) {
        a = seed = 1812433253u * (seed ^ (seed >> 30));
        b = seed = 1812433253u * (seed ^ (seed >> 30)) + 1;
        c = seed = 1812433253u * (seed ^ (seed >> 30)) + 2;
        d = seed = 1812433253u * (seed ^ (seed >> 30)) + 3;
    }
    uint32_t operator() () {
        uint32_t t = (a ^ (a << 11));
        a = b; b = c; c = d;
        return d = (d ^ (d >> 19)) ^ (t ^ (t >> 8));
    }
    static constexpr uint32_t max() { return numeric_limits<result_type>::max(); }
    static constexpr uint32_t min() { return numeric_limits<result_type>::min(); }
private:
    uint32_t a, b, c, d;
};

constexpr int N = 50;

constexpr array<char, 4> DIR_LETTER = {'U', 'D', 'R', 'L'};
constexpr array<int, 4> DIR_Y = {-1, 1, 0, 0};
constexpr array<int, 4> DIR_X = {0, 0, 1, -1};
inline bool is_on_tiles(int y, int x) {
    return 0 <= y and y < N and 0 <= x and x < N;
}

template <class RandomEngine>
string solve(const int sy, const int sx, const array<array<int, N>, N>& tile, const array<array<int, N>, N>& point, RandomEngine& gen) {
    int M = 0;
    REP (y, N) {
        REP (x, N) {
            M = max(M, tile[y][x] + 1);
        }
    }

    // random walk
    int y = sy;
    int x = sx;
    vector<bool> used(M);
    used[tile[y][x]] = true;
    int score = point[y][x];
    string ans;
    while (true) {
        array<int, 4> dirs = {{0, 1, 2, 3}};
        shuffle(ALL(dirs), gen);
        bool found = false;
        for (int dir : dirs) {
            int ny = y + DIR_Y[dir];
            int nx = x + DIR_X[dir];
            if (is_on_tiles(ny, nx) and not used[tile[ny][nx]]) {
                found = true;
                ans.push_back(DIR_LETTER[dir]);
                y = ny;
                x = nx;
                used[tile[y][x]] = true;
                score += point[y][x];
                break;
            }
        }
        if (not found) {
            break;
        }
    }

    cerr << "ans = " << ans << endl;
    cerr << "score = " << score << endl;
    return ans;
}

int main() {
    int sy, sx; cin >> sy >> sx;
    array<array<int, N>, N> tile;
    REP (y, N) {
        REP (x, N) {
            cin >> tile[y][x];
        }
    }
    array<array<int, N>, N> point;
    REP (y, N) {
        REP (x, N) {
            cin >> point[y][x];
        }
    }
    xor_shift_128 gen;
    string ans = solve(sy, sx, tile, point, gen);
    cout << ans << endl;
    return 0;
}
