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
constexpr array<char, 4> DIR_LETTER = {'U', 'D', 'R', 'L'};
constexpr array<int, 4> DIR_Y = {-1, 1, 0, 0};
constexpr array<int, 4> DIR_X = {0, 0, 1, -1};
inline bool is_on_tiles(int y, int x) {
    return 0 <= y and y < N and 0 <= x and x < N;
}

struct beam_state {
    string command;
    vector<bool> used;
    int score;
    int8_t y;
    int8_t x;
};

template <class RandomEngine>
string solve(const int sy, const int sx, const array<array<int, N>, N>& tile, const array<array<int, N>, N>& point, RandomEngine& gen, chrono::high_resolution_clock::time_point clock_end) {
    chrono::high_resolution_clock::time_point clock_begin = chrono::high_resolution_clock::now();

    int M = 0;
    REP (y, N) {
        REP (x, N) {
            M = max(M, tile[y][x] + 1);
        }
    }

    vector<shared_ptr<beam_state> > cur;
    {
        shared_ptr<beam_state> initial = make_shared<beam_state>();
        initial->y = sy;
        initial->x = sx;
        initial->used.resize(M);
        initial->used[tile[sy][sx]] = true;
        initial->score = point[sy][sx];
        cur.push_back(initial);
    }
    shared_ptr<beam_state> result = cur.back();

    while (not cur.empty()) {
        chrono::high_resolution_clock::time_point clock_now = chrono::high_resolution_clock::now();
        if (clock_end <= clock_now) {
            break;
        }

        vector<shared_ptr<beam_state> > prv;
        cur.swap(prv);
        for (auto& s : prv) {
            for (int dir : DIRS) {
                int ny = s->y + DIR_Y[dir];
                int nx = s->x + DIR_X[dir];
                if (is_on_tiles(ny, nx) and not s->used[tile[ny][nx]]) {
                    shared_ptr<beam_state> t = make_shared<beam_state>(*s);
                    t->y = ny;
                    t->x = nx;
                    t->command.push_back(DIR_LETTER[dir]);
                    t->used[tile[ny][nx]] = true;
                    t->score += point[ny][nx];
                    cur.push_back(t);
                }
            }
        }
        constexpr int WIDTH = 1000;
        if (cur.size() > WIDTH) {
            partial_sort(cur.begin(), cur.begin() + WIDTH, cur.end(), [&](const shared_ptr<beam_state>& a, const shared_ptr<beam_state>& b) {
                return a->score > b->score;
            });
            cur.resize(WIDTH);
        }
        if (not cur.empty() and result->score < cur.front()->score) {
            result = cur.front();
        }
    }

    cerr << "ans = " << result->command << endl;
    cerr << "score = " << result->score << endl;
    return result->command;
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
