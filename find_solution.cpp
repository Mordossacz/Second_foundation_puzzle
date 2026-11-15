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

    Point rotate_60_ccw() {
        return {y * -1, x + y};
    }

    Point rotate_60() {
        return {x + y, -1 * x};
    }

    // degrees = k*60
    Point rotate(int k) {
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

    Point symmetry_oy() {
        return {-1 * x, y};
    }
};

struct PointHash {
    std::size_t operator()(const Point& p) const noexcept {
        return hash_all(p.x, p.y);
    }
};

struct Edge {
    Point a, w;

    Edge(Point a, Point b)
        : a(a), w(b.x - a.x, b.y - a.y)
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

    Triangle(const Triangle& t) noexcept = default;
    Triangle(Triangle&& t) noexcept = default;

    auto operator<=>(const Triangle&) const = default;
    friend std::ostream& operator<<(std::ostream& os, const Triangle& t) {
        return os << "[" << t.points.at(0) << ", " << t.points.at(1) << ", " << t.points.at(2) << "]";
    }

    // degrees = k*60
    Triangle rotate(int k) {
        vector<Point> r_points;

        for (auto& p : points) {
            r_points.push_back(p.rotate(k));
        };

        return Triangle(std::move(r_points));
    }

    Triangle symmetry_oy() {
        return Triangle(points.at(0), points.at(1), points.at(2));
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
    std::string name;
    vector<Triangle> shape;

    int get_new_id() {
        return ++block_counter;
    }

public:
    Block(const std::string& name, vector<Triangle>&& shape)
        : id(get_new_id()), name(name), shape(std::move(shape)) {}

    Block(const std::string& name, const vector<Triangle>& shape)
        : id(get_new_id()), name(name), shape(shape) {}
    
    const vector<Triangle>& get_shape() const {
        return shape;
    }

    Block flip_block() {
        vector<Triangle> f_shape;

        return Block(name + "'", std::move(f_shape));
    }

    Block rotate_block(int angle) {
        vector<Triangle> r_shape;

        return Block(name + std::to_string(angle), std::move(r_shape));
    }
};

class Plane {
    std::unordered_map<Triangle, int, TriangleHash> surface;
    std::unordered_set<Point, PointHash> starting_points;
    std::unordered_set<Point, PointHash> ending_points;

public:
    // give points in counter clock wise manner
    Plane(const vector<Edge>& boundaries) {
        for (auto& e : boundaries) {
            Point base_vector = e.get_base_vector();
            Point end = e.a + e.w;

            for (Point p = e.a; p != end;) {
                if (base_vector.y < 0) starting_points.emplace(p);
                else if (base_vector.y > 0) ending_points.emplace(p);

                Point next_point = p + base_vector;
                surface.emplace(
                    Triangle(p, next_point, p + base_vector.rotate_60()),
                    0
                );

                p = next_point;
            }

            if (base_vector.y < 0) starting_points.emplace(end);
            else if (base_vector.y > 0) ending_points.emplace(end);
        }

    }

    // void try_to_fit(Plane& plane) {

    // }

    void generate_map_file(const std::string& filename) {
        {
            std::ofstream file(filename);
            if (!file) {
                std::cerr << "Błąd otwarcia pliku" << endl;
                return;
            }

            for (auto& it : surface) {
                file << it.first << " " << it.second << '\n';
            }
        }
    }
};

void triangle_test() {
    cout << "triangle_test" << '\n';
    Triangle t = {{1, 0}, {1, 1}, {2, 0}};

    for (int i = 0; i < 7; ++i) {
        cout << t << ", " << i << " -> " << t.rotate(i) << '\n';
    }
    cout << t.symmetry_oy() << '\n';
    cout << "------------------" << '\n';
}

void plane_test() {
    cout << "plane_test" << '\n';
    Plane p = {{
        {{0, 0}, {1, 0}},
        {{1, 0}, {0, 1}},
        {{0, 1}, {0, 0}},
    }};

    p.generate_map_file("test");
    cout << "------------------" << '\n';
}

int main() {
    plane_test();

    triangle_test();

    return 0;
}