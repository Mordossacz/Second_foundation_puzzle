#include <vector>
#include <string>
#include <utility>
#include <cmath>
#include <iostream>

using std::cout;
using std::endl;
using std::vector;

#define PI 3.1415

struct point {
    int x;
    int y;

    bool operator==(const point& p) const = default;
    bool operator<=>(const point& p) const = default;
    friend std::ostream& operator<<(std::ostream& os, const point& p) {
        return os << "(" << p.x << ", " << p.y << ")";
    }
};

class Block {
    inline static int block_counter = 0;

    int id;
    std::string name;
    vector<point> points;

    int get_new_id() {
        return ++block_counter;
    }


public:
    static point rotate_point(const point& p, double a) {
        point new_point = {
            (int) lround(p.x * cos(a) - p.y * sin(a)),
            (int) lround(p.x * sin(a) + p.y * cos(a))
        };
        return new_point;
    }
    Block(const std::string& name, vector<point>&& points)
        : id(get_new_id()), name(name), points(std::move(points)) {}

    Block(const std::string& name, const vector<point>& points)
        : id(get_new_id()), name(name), points(points) {}
    
    const vector<point>& get_points() const {
        return points;
    }

    static Block flip_block(Block block) {
        vector<point> f_points;

        return Block(block.name + "'", std::move(f_points));
    }

    static Block rotate_block(Block block, int angle) {
        vector<point> r_points;

        return Block(block.name + std::to_string(angle), std::move(r_points));
    }
};

class Plane {
    vector<vector<int>> points;

public:
    Plane(int x, int y, const vector<std::pair<point, point>>& boundry_points)
        : points(vector<vector<int>>(y, vector<int>(x, 0)))
    {
        for (auto p : boundry_points) {
            point& left = p.first;
            point& right = p.second;

            point start = left;
            while (start != right);

        }

    }

    void try_to_fit(Plane& plane) {

    }
};

int main() {
    vector<point> points = {
        {2, 0},
        {3, 0},
        {2, 1}
    };

    for (auto p : points) {
        cout << p << " -> " << Block::rotate_point(p, PI* 2/3) << '\n';
    }

    return 0;
}