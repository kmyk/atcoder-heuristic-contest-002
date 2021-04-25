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

constexpr array<int, 4> DIRS = {{0, 1, 2, 3}};
constexpr array<int, 4> DIR_Y = {-1, 1, 0, 0};
constexpr array<int, 4> DIR_X = {0, 0, 1, -1};
inline bool is_on_tiles(int y, int x) {
    return 0 <= y and y < N and 0 <= x and x < N;
}

inline uint16_t pack_point(int y, int x) {
    return (y << 8) + x;
}

inline pair<int, int> unpack_point(int packed) {
    return {packed >> 8, packed & ((1 << 8) - 1)};
}

template <class RandomEngine>
string solve(const int sy, const int sx, const array<array<int, N>, N>& tile, const array<array<int, N>, N>& point, RandomEngine& gen, chrono::high_resolution_clock::time_point clock_end) {
    chrono::high_resolution_clock::time_point clock_begin = chrono::high_resolution_clock::now();

    int M = 0;
    REP (y, N) {
        REP (x, N) {
            M = max(M, tile[y][x] + 1);
        }
    }

    vector<uint16_t> path_prev;
    path_prev.push_back(pack_point(sy, sx));
    vector<char> used_tile_prev(M);
    used_tile_prev[tile[sy][sx]] = true;
    array<array<bool, N>, N> used_pos_prev = {};
    used_pos_prev[sy][sx] = true;
    int score_prev = point[sy][sx];

    vector<uint16_t> result = path_prev;
    int highscore = score_prev;

    // simulated annealing
    int64_t iteration = 0;
    double temperature = 1.0;
    for (; ; ++ iteration) {
        if (iteration % 64 == 0) {
            chrono::high_resolution_clock::time_point clock_now = chrono::high_resolution_clock::now();
            temperature = static_cast<long double>((clock_end - clock_now).count()) / (clock_end - clock_begin).count();
            if (temperature <= 0.0) {
                cerr << "done  (iteration = " << iteration << ")" << endl;
                break;
            }
        }

        int start = uniform_int_distribution<int>(1, path_prev.size())(gen);
        int score_next = 0;
        vector<char> used_tile_next(M);
        vector<uint16_t> diff;
        REP (i, start) {
            auto [y, x] = unpack_point(path_prev[i]);
            score_next += point[y][x];
            used_tile_next[tile[y][x]] = true;
        }
        auto [y, x] = unpack_point(path_prev[start - 1]);
        while (true) {
            array<int, 4> dirs = {{0, 1, 2, 3}};
            shuffle(ALL(dirs), gen);
            bool found = false;
            for (int dir : dirs) {
                int ny = y + DIR_Y[dir];
                int nx = x + DIR_X[dir];
                if (not is_on_tiles(ny, nx)) {
                    continue;
                }
                if (diff.empty() and start < path_prev.size() and path_prev[start] == pack_point(ny, nx)) {
                    continue;
                }
                if (not used_tile_next[tile[ny][nx]]) {
                    found = true;
                    diff.push_back(pack_point(ny, nx));
                    y = ny;
                    x = nx;
                    used_tile_next[tile[y][x]] = true;
                    score_next += point[y][x];
                    break;
                }
            }
            if (not found) {
                break;
            }
            if (used_pos_prev[y][x]) {
                break;
            }
        }
        if (diff.empty()) {
            continue;
        }
        if (used_pos_prev[y][x]) {
            int end = start;
            while (end < path_prev.size() and path_prev[end] != pack_point(y, x)) {
                ++ end;
            }
            assert (end < path_prev.size());
            REP3 (i, end + 1, path_prev.size()) {
                auto [y, x] = unpack_point(path_prev[i]);
                if (used_tile_next[tile[y][x]]) {
                    break;
                }
                diff.push_back(path_prev[i]);
                score_next += point[y][x];
                used_tile_next[tile[y][x]] = true;
            }
        }

        int delta = score_next - score_prev;
        auto probability = [&]() {
            constexpr long double boltzmann = 0.01;
            return exp(boltzmann * delta) * temperature;
        };
        if (delta >= 0 or bernoulli_distribution(probability())(gen)) {
            // accept
            if (delta < 0) {
#ifdef LOCAL
                cerr << "decreasing move  (delta = " << delta << ", iteration = " << iteration << ")" << endl;
#endif  // LOCAL
            }

            path_prev.resize(start);
            path_prev.insert(path_prev.end(), ALL(diff));
            used_tile_prev = used_tile_next;
            used_pos_prev = {};
            for (uint16_t packed : path_prev) {
                auto [y, x] = unpack_point(packed);
                used_pos_prev[y][x] = true;
            }
            score_prev = score_next;

            if (highscore < score_prev) {
                highscore = score_prev;
                result = path_prev;
#ifdef LOCAL
                cerr << "highscore = " << highscore << "  (iteration = " << iteration << ")" << endl;
#endif  // LOCAL
            }
        }
    }

    string ans;
    assert (not result.empty());
    REP (i, (int)result.size() - 1) {
        auto [ay, ax] = unpack_point(result[i]);
        auto [by, bx] = unpack_point(result[i + 1]);
        if (by == ay - 1 and bx == ax) {
            ans.push_back('U');
        } else if (by == ay + 1 and bx == ax) {
            ans.push_back('D');
        } else if (by == ay and bx == ax + 1) {
            ans.push_back('R');
        } else if (by == ay and bx == ax - 1) {
            ans.push_back('L');
        } else {
            assert (false);
        }
    }
    cerr << "ans = " << ans << endl;
    cerr << "score = " << highscore << endl;
    return ans;
}

int main() {
    constexpr auto TIME_LIMIT = chrono::milliseconds(2000);
    chrono::high_resolution_clock::time_point clock_begin = chrono::high_resolution_clock::now();
    xor_shift_128 gen(20210425);

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
    string ans = solve(sy, sx, tile, point, gen, clock_begin + chrono::duration_cast<chrono::milliseconds>(TIME_LIMIT * 0.95));
    cout << ans << endl;
    return 0;
}
