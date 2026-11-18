#include <ios>
#include <vector>
#include <fstream>
#include <string>
#include <utility>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

using std::cout;
using std::endl;
using std::vector;

template <typename... Args>
std::size_t hash_all(const Args&... args) noexcept {
    std::size_t seed = 0;
    ((seed ^= std::hash<Args>{}(args) + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2)), ...);
    return seed;
}

struct Point {
    int x;
    int y;

    auto operator<=>(const Point&) const = default;
    friend std::ostream& operator<<(std::ostream& os, const Point& p) {
        return os << "(" << p.x << ", " << p.y << ")";
    }

    Point neg() const {
        return {x * -1, y * -1};
    }

    Point& operator+=(const Point& p) {
        this->x += p.x;
        this->y += p.y;
        return *this;
    }

    Point& operator-=(const Point& p) {
        return *this += p.neg();
    }

    friend Point operator-(Point lp, const Point& rp) {
        lp -= rp;
        return lp;
    }

    friend Point operator+(Point lp, const Point& rp) {
        lp += rp;
        return lp;
    }

    Point rotate_60_ccw() const {
        return {y * -1, x + y};
    }

    Point rotate_60() const {
        return {x + y, -1 * x};
    }

    // degrees = k*60
    Point rotate(int k) const {
        Point p = {x, y};

        k = (k % 6);
        int k_ = abs(6-k);

        if (k_ < k) {
            for (int i = 0; i < k_; ++i) p = p.rotate_60();
        }
        else {
            for (int i = 0; i < k; ++i) p = p.rotate_60_ccw();
        }
        return p;
    }

    Point symmetry_ox() const {
        return {x + y, y * -1};
    }
};

struct PointHash {
    std::size_t operator()(const Point& p) const noexcept {
        return hash_all(p.x, p.y);
    }
};

struct Edge {
    Point a, w;
    bool strong;

    Edge(Point a, Point b, bool strong = true)
        : a(a), w(b.x - a.x, b.y - a.y), strong(strong)
    {
        if (w.x != 0 && w.y != 0 && abs(w.x) != abs(w.y)) {
            cout << a << " " << b << endl;
            throw std::invalid_argument("Zla krawedz");
        }
    }

    static int sgn(int x) {
        return (0 < x) - (x < 0);
    }

    Point get_base_vector() const {
        return {sgn(w.x), sgn(w.y)};
    }
};

struct Triangle {
    vector<Point> points;

    Triangle(Point a, Point b, Point c)
        : points(vector<Point>{a, b, c})
    {
        std::sort(points.begin(), points.end());
    }

    Triangle(vector<Point>&& points)
        : points(std::move(points))
    {
        std::sort(this->points.begin(), this->points.end());
    }

    Triangle& operator=(const Triangle& t) = default;
    Triangle& operator=(Triangle&& t) = default;

    Triangle(const Triangle& t) noexcept = default;
    Triangle(Triangle&& t) noexcept = default;

    auto operator<=>(const Triangle&) const = default;
    friend std::ostream& operator<<(std::ostream& os, const Triangle& t) {
        return os << "[" << t.points.at(0) << ", " << t.points.at(1) << ", " << t.points.at(2) << "]";
    }

    // degrees = k*60
    Triangle rotate(int k) const {
        vector<Point> r_points;

        for (auto& p : points) {
            r_points.push_back(p.rotate(k));
        };

        return Triangle(std::move(r_points));
    }

    Triangle symmetry_ox() const {
        return Triangle(points.at(0).symmetry_ox(), points.at(1).symmetry_ox(), points.at(2).symmetry_ox());
    }

    Triangle move(const Point& p) const {
        return Triangle(points.at(0) + p, points.at(1) + p, points.at(2) + p);
    }
};

struct TriangleHash {
    std::size_t operator()(const Triangle& t) const noexcept {
        return hash_all(
            t.points.at(0).x, t.points.at(0).y,
            t.points.at(1).x, t.points.at(2).y,
            t.points.at(2).x, t.points.at(2).y
        );
    }
};

class Block {
    inline static int block_counter = 0;

    int id;
    vector<Triangle> shape;

    int get_new_id() {
        return ++block_counter;
    }

public:
    Block() {}
    Block(vector<Triangle>&& shape)
        : id(get_new_id()), shape(std::move(shape)) {}

    Block(vector<Triangle>&& shape, int id)
        : id(id), shape(std::move(shape)) {}

    Block(const vector<Triangle>& shape)
        : id(get_new_id()), shape(shape) {}
    
    Block(const Block& b) = default;
    Block(Block&& b) = default;

    Block& operator=(const Block& b) = default;
    Block& operator=(Block&& b) = default;

    const vector<Triangle>& get_shape() const {
        return shape;
    }

    int get_id() const {
        return id;
    }

    static int get_block_counter() {
        return block_counter;
    }

    Block flip_block() {
        vector<Triangle> f_shape;

        for (auto& t : shape) {
            f_shape.push_back(t.symmetry_ox());
        }

        return Block(std::move(f_shape), id);
    }

    Block rotate_block(int k) {
        vector<Triangle> r_shape;

        for (auto& t : shape) {
            r_shape.push_back(t.rotate(k));
        }

        return Block(std::move(r_shape), id);
    }

    vector<Block> generate_every_combination() const {
        vector<Block> combinations;
        Block candidate = *this;

        for (int j = 0; j < 2; ++j) {
            for (int i = 0; i < 6; ++i) {
                combinations.push_back(candidate);

                candidate = candidate.rotate_block(1);
            }

            candidate = candidate.flip_block();
        }

        return combinations;
    }
};

class Plane {
    std::unordered_map<Triangle, int, TriangleHash> surface;

    std::unordered_set<Point, PointHash> starting_points;
    std::unordered_set<Point, PointHash> ending_points;
    inline static constexpr Point NEXT_POINT = {1, 0};
    inline static constexpr int BORDER_ID = 0;
    inline static constexpr int MAX_GENERATE_SOLUTIONS = 40;

    inline static int solutions_counter = 0;

    bool find_solution_rec(int i, const vector<Block>& blocks) {
        Block result;

        for (auto p : starting_points) {
            p -= NEXT_POINT;
            do {
                p += NEXT_POINT;

                for (auto& b : blocks.at(i).generate_every_combination()) {

                    if (try_to_fit_block(b, p)) {
                        fit_block(b, p);
                        // we succeeded at inserting our block

                        if (i == 0) {
                            // we are last piece, so every bigger block is already inserted
                            ++solutions_counter;
                            if (solutions_counter <= MAX_GENERATE_SOLUTIONS) {
                                generate_map_file(
                                    "solve_for_a_" + std::to_string(solutions_counter)
                                );
                            }
                        }
                        else {
                            find_solution_rec(i - 1, blocks);
                        }

                        clear(b, p);
                    }
                }
            }
            while (!ending_points.contains(p));
        }

        return false;
    }

public:
    // give points in counter clock wise manner
    Plane(const vector<Edge>& boundaries) {
        for (auto& e : boundaries) {
            Point base_vector = e.get_base_vector();
            Point end = e.a + e.w;

            // for (Point p = e.a; p != end;) {
            Point p = e.a;
            Point next_point = p;
            do {
                p = next_point;
                next_point = p + base_vector;

                if (!e.strong && p == end) break;

                if (base_vector.y < 0) starting_points.emplace(p);
                else if (base_vector.y > 0) ending_points.emplace(p);

                surface.emplace(
                    Triangle(p, next_point, p + base_vector.rotate_60()),
                    BORDER_ID
                );

                if (e.strong) {
                    surface.emplace(
                        Triangle(p, p + base_vector.rotate_60(), p + base_vector.rotate(4)),
                        BORDER_ID
                    );
                }
            }
            while (p != end);

            if (base_vector.y < 0) starting_points.emplace(end);
            else if (base_vector.y > 0) ending_points.emplace(end);
        }
    }

    bool try_to_fit_triangle(const Triangle& t) const {
        return !surface.contains(t);
    }

    bool try_to_fit_block(const Block& b, const Point& p) const {
        for (auto& t : b.get_shape()) {
            if (!try_to_fit_triangle(t.move(p))) return false;
        }

        return true;
    }

    void fit_block(const Block& b, const Point& p) {
        for (auto& t : b.get_shape()) {
            surface.emplace(t.move(p), b.get_id());
        }
    }

    void clear() {
        for (auto it = surface.begin(); it != surface.end();) {
            if (it->second != 0) {
                it = surface.erase(it);
            }
            else ++it;
        }
    }

    void clear(int block_id) {
        for (auto it = surface.begin(); it != surface.end();) {
            if (it->second == block_id) {
                it = surface.erase(it);
            }
            else ++it;
        }
    }

    void clear(const Block& b, const Point& p) {
        for (auto t : b.get_shape()) {
            t = t.move(p);
            auto it = surface.find(t);
            if (it != surface.end() && it->second == b.get_id()) {
                surface.erase(t);
            }
        }
    }

    void remove_starting_point(const Point& p) {
        starting_points.erase(p);
    }

    void remove_ending_point(const Point& p) {
        ending_points.erase(p);
    }

    void generate_map_file(const std::string& filename) const {
        {
            std::ofstream file;
            file.open(filename);
            if (!file) {
                std::cerr << "Błąd otwarcia pliku" << endl;
                return;
            }

            for (auto& it : surface) {
                file << it.first << ", " << it.second << '\n';
            }

            file << "startings\n";
            for (auto& it : starting_points) {
                file << it << ", ";
            }
            file << "\nendings\n";
            for (auto& it : ending_points) {
                file << it << ", ";
            }
            file << '\n';
        }
    }

    void add_border(const Triangle& t) {
        surface.emplace(t, BORDER_ID);
    }

    int find_solution(const vector<Block>& blocks) {
        solutions_counter = 0;
        find_solution_rec(blocks.size() - 1, blocks);
        return solutions_counter;
    }
};

void triangle_test() {
    cout << "triangle_test" << '\n';
    Triangle t = {{1, 0}, {1, 1}, {2, 0}};

    for (int i = 0; i < 7; ++i) {
        cout << t << ", " << i << " -> " << t.rotate(i) << '\n';
    }
    cout << t.symmetry_ox() << '\n';
    cout << "------------------" << '\n';
}

void plane_test() {
    cout << "plane_test" << '\n';
    Plane p = {{
        {{0, 0}, {5, 0}},
        {{7, 0}, {7, 5}},
    }};

    p.generate_map_file("test");
    cout << "------------------" << '\n';
}

void create_and_delete() {
    cout << "small block" << '\n';
    Block A = {{
        {{0, 0}, {0, 1}, {1, 0}},
        {{0, 1}, {1, 0}, {1, 1}}
    }};
    Block B = {{
        {{0, 0}, {0, 1}, {1, 0}},
        {{0, 1}, {1, 0}, {1, 1}}
    }};

    Plane p = {{
        {{0, 1}, {2, 1}},
        {{2, 1}, {2, 3}},
        {{2, 3}, {0, 3}},
        {{0, 3}, {0, 1}},
    }};


    vector<Point> a = {
        {0, 1},
        {0, 2},
        {1, 1},
        {0, 3},
        {1, 2},
        {1, 3}
    };

    int i = 0;

    for (auto& po : a) {

        if (p.try_to_fit_block(A, po)) {
            p.fit_block(A, po);

            p.generate_map_file("solve_for_cad_" + std::to_string(i));

            p.clear(A, po);

            p.generate_map_file("solve_for_cad_" + std::to_string(++i));
        }
        else {
            cout << po << " nie da sie" << '\n';
        }
        i ++;
    }

    cout << "------------------" << '\n';
}

void small_blocks() {
    cout << "small block" << '\n';
    Block A = {{
        {{0, 0}, {0, 1}, {1, 0}},
        {{0, 1}, {1, 0}, {1, 1}}
    }};
    Block B = {{
        {{0, 0}, {0, 1}, {1, 0}},
        {{0, 1}, {1, 0}, {1, 1}}
    }};

    Plane p = {{
        {{0, 1}, {2, 1}},
        {{2, 1}, {2, 2}},
        {{2, 2}, {0, 2}},
        {{0, 2}, {0, 1}},
    }};

    cout << "soulutions found: " << p.find_solution({
            A,
            B
    }) << '\n';

    p.generate_map_file("test");

    cout << "------------------" << '\n';
}

void first_serious_puzzle() {
    cout << "first_serious_puzzle" << '\n';
    Block A1 = {{
        {{0, 0}, {0, 1}, {1, 0}},
        {{0, 1}, {1, 0}, {1, 1}},
        {{1, 0}, {1, 1}, {2, 0}},
        {{1, 0}, {2, -1}, {2, 0}}
    }};
    Block B1 = {{
        {{0, 0}, {1, 0}, {0, 1}},
        {{0, 1}, {1, 0}, {1, 1}},
        {{0, 1}, {1, 1}, {0, 2}},
        {{1, 0}, {2, 0}, {1, 1}},
        {{0, 2}, {1, 1}, {1, 2}},
        {{1, 1}, {2, 1}, {1, 2}}
    }};
    Block C1 = {{
        {{0, 0}, {1, 0}, {1, -1}},
        {{1, -1}, {2, -1}, {1, 0}},
        {{1, 0}, {2, -1}, {2, 0}},
        {{1, 0}, {2, 0}, {1, 1}},
        {{1, 1}, {2, 0}, {2, 1}},
        {{1, 1}, {2, 1}, {1, 2}}
    }};

    Plane p = {{
        {{1, 1}, {4, 1}},
        {{4, 1}, {4, 4}},
        {{4, 4}, {3, 4}},
        {{3, 4}, {3, 3}, false},
        {{3, 3}, {2, 4}, false},
        {{2, 4}, {1, 4}},
        {{1, 4}, {1, 1}}
    }};

    p.remove_ending_point({3, 3});
    p.remove_starting_point({3, 3});
    // plane created

    cout << "soulutions found: " << p.find_solution({
            A1,
            B1,
            C1
    }) << '\n';

    cout << "------------\n";
}

static const Block A = {{
    {{0, 0}, {1, 0}, {0, 1}},
    {{0, 0}, {0, 1}, {-1, 1}},
    {{-1, 1}, {0, 1}, {-1, 2}},
    {{0, 1}, {0, 2}, {-1, 2}},
    {{-1, 3}, {0, 2}, {-1, 2}},
    {{-1, 3}, {0, 2}, {0, 3}},
    {{1, 2}, {0, 2}, {0, 3}},
    {{1, 2}, {1, 3}, {0, 3}},
    {{1, 2}, {1, 3}, {2, 2}},
    {{2, 3}, {1, 3}, {2, 2}},
    {{2, 3}, {3, 2}, {2, 2}},
    {{3, 1}, {3, 2}, {2, 2}},
    {{3, 1}, {3, 2}, {4, 1}},
}};
static const Block B = {{
    {{0, 0}, {1, 0}, {0, 1}},
    {{1, 1}, {1, 0}, {0, 1}},
    {{1, 1}, {1, 0}, {2, 0}},
    {{1, 1}, {2, 1}, {2, 0}},
    {{3, 0}, {2, 1}, {2, 0}},
    {{3, 0}, {3, -1}, {2, 0}},
    {{3, 0}, {3, -1}, {4, -1}},
    {{3, 0}, {4, 0}, {4, -1}},
    {{4, -2}, {3, -1}, {4, -1}},
    {{4, -2}, {5, -2}, {4, -1}},
    {{4, -2}, {5, -2}, {5, -3}},
    {{4, -2}, {4, -3}, {5, -3}},
    {{4, -2}, {4, -3}, {3, -2}}
}};
static const Block C = {{
    {{0, 0}, {0, 1}, {-1, 1}},
    {{-1, 2}, {0, 1}, {-1, 1}},
    {{-1, 2}, {0, 1}, {0, 2}},
    {{1, 1}, {0, 1}, {0, 2}},
    {{1, 1}, {0, 1}, {1, 0}},
    {{1, 1}, {1, 2}, {0, 2}},
    {{1, 1}, {1, 2}, {2, 1}},
    {{2, 2}, {1, 2}, {2, 1}},
    {{2, 2}, {1, 2}, {1, 3}},
    {{2, 2}, {2, 3}, {1, 3}},
    {{1, 4}, {2, 3}, {1, 3}}
}};
static const Block D = {{
    {{0, 0}, {1, 0}, {1, -1}},
    {{2, -1}, {1, 0}, {1, -1}},
    {{2, -1}, {1, 0}, {2, 0}},
    {{2, -1}, {3, -1}, {2, 0}},
    {{2, -1}, {3, -1}, {3, -2}},
    {{4, -2}, {3, -1}, {3, -2}},
    {{4, -2}, {4, -3}, {3, -2}},
    {{4, -2}, {4, -3}, {5, -3}},
    {{4, -2}, {5, -2}, {5, -3}},
    {{6, -3}, {5, -2}, {5, -3}},
    {{6, -3}, {5, -2}, {6, -2}},
    {{5, -1}, {5, -2}, {6, -2}}
}};

void original_puzzle_hard() {
    cout << "original_puzzle_hard" << '\n';

    Plane p = {{
        {{0, 0}, {3, 0}},
        {{3, 0}, {3, 4}},
        {{3, 4}, {1, 6}},
        {{1, 6}, {-2, 6}},
        {{-2, 6}, {-2, 5}, false},
        {{-2, 5}, {-3, 6}, false},
        {{-3, 6}, {-4, 6}},
        {{-4, 6}, {-4, 4}},
        {{-4, 4}, {0, 0}}
    }};

    p.remove_ending_point({-2, 5});
    p.remove_starting_point({-2, 5});
    // plane created

    cout << "soulutions found: " << p.find_solution({
            A,
            B,
            C,
    }) << '\n';

    cout << "------------\n";
}

void original_puzzle_easy() {
    cout << "original_puzzle_easy" << '\n';

    Plane p = {{
        {{0, 0}, {4, 0}},
        {{4, 0}, {4, 1}},
        {{4, 1}, {3, 2}, false},
        {{3, 2}, {3, 3}},
        {{3, 3}, {0, 6}},
        {{0, 6}, {-1, 6}},
        {{-1, 6}, {-1, 5}, false},
        {{-1, 5}, {-4, 5}},
        {{-4, 5}, {-4, 3}},
        {{-4, 3}, {-2, 1}},
        {{-2, 1}, {-1, 1}, false},
        {{-1, 1}, {0, 0}},
    }};

    p.remove_ending_point({-1, 1});
    p.remove_starting_point({-1, 1});

    p.remove_starting_point({3, 2});

    p.remove_ending_point({-1, 5});
    p.remove_starting_point({-1, 5});
    // plane created

    // p.fit_block()

    cout << "soulutions found: " << p.find_solution({
            A,
            B,
            C,
            D
    }) << '\n';

    cout << "------------\n";
}

void flip_test() {
    cout << "flip test\n";
    Block b = {{
        {{0, 1}, {1, 1}, {0, 2}},
        {{1, 2}, {0, 2}, {1, 1}}
    }};

    Block bf = b.flip_block();
    for (int i = 0; i < 2; ++i) {
        cout << b.get_shape().at(i) << " -> " << bf.get_shape().at(i) << '\n';
    }
    cout << "------------\n";
}

int main() {
    //plane_test();

    // triangle_test();

    // small_blocks();

    // create_and_delete();

    // first_serious_puzzle();

    // flip_test();

    // original_puzzle_easy();

    original_puzzle_hard();

    return 0;
}
