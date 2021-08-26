#include <set>
#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <cassert>

// struct representing triplet of coordinates
struct position
{
    int x, y, z;

    // we need comparsion to store them in map/set
    bool operator<(const position pos) const
    {
        if (x == pos.x)
        {
            if (y == pos.y)
                return z < pos.z;
            return y < pos.y;
        }
        return x < pos.x;
    }

    // list of neighboring positions
    std::vector<position> neighbors()
    {
        return {
            {x - 1, y, z},
            {x + 1, y, z},
            {x, y - 1, z},
            {x, y + 1, z},
            {x, y, z - 1},
            {x, y, z + 1},
        };
    }
};

// struct representing one unit cube, contains position, index and some other tags
struct cube
{
    int index;
    position pos;
    bool visited;
};

// it might be useful to distinguish different types of squares
enum class square_type
{
    top_base,
    bottom_base,
    circumference,
    hole
};

// styles for different square types
std::map<square_type, std::string> square_style = 
{
    {square_type::top_base, "fill:green;stroke:black;stroke-width:5;fill-opacity:0.5"},
    {square_type::bottom_base, "stroke-alignment:inner;fill:blue;stroke:black;stroke-width:5;fill-opacity:0.5"},
    {square_type::circumference, "fill:red;stroke:black;stroke-width:5;fill-opacity:0.5"},
    {square_type::hole, ""},
};

// enum for directions in plane
namespace direction
{
    enum direction
    {
        up = 0,
        left = 1,
        down = 2,
        right = 3
    };

    // cyclic increment
    void operator++(direction &dir, int)
    {
        switch (dir)
        {
        case up:
            dir = left;
            break;
        case left:
            dir = down;
            break;
        case down:
            dir = right;
            break;
        case right:
            dir = up;
        }
    }
    void operator--(direction &dir, int)
    {
        dir++;
        dir++;
        dir++;
    }
}

// same as position, one for two coordinates
struct plane_position
{
    int x, y;

    bool operator<(const plane_position &pos) const
    {
        return x == pos.x ? y < pos.y : x < pos.x;
    }

    bool operator==(const plane_position &pos) const
    {
        return x == pos.x && y == pos.y;
    }

    bool operator!=(const plane_position &pos) const
    {
        return x != pos.x || y != pos.y;
    }

    plane_position up() const { return {x, y + 1}; }
    plane_position down() const { return {x, y - 1}; }
    plane_position left() const { return {x - 1, y}; }
    plane_position right() const { return {x + 1, y}; }

    // order matters
    std::vector<plane_position> neighbors() const
    {
        return {up(), left(), down(), right()};
    }
};

// struct representing square of unfolding
struct square
{
    plane_position pos;
    square_type type;
};

// unfolding of some polycube, consists of several squares
class unfolding
{
public:
    std::map<plane_position, square> squares;
};

//generates svg output
std::ostream &operator<<(std::ostream &os, unfolding &uf)
{
    int minx = 1e9, maxx = -1e9, miny = 1e9, maxy = -1e9;
    for (auto pair : uf.squares)
    {
        minx = std::min(minx, pair.first.x);
        miny = std::min(miny, pair.first.y);
        maxy = std::max(maxy, pair.first.y);
        maxx = std::max(maxx, pair.first.x);
    }
    int square_size = 100;
    int margin = 20;
    int height = (maxy - miny + 1) * square_size + 2*margin;
    int width = (maxx - minx + 1) * square_size + 2*margin;
    os << "<svg width=\"" << width << "\" height=\"" << height << "\">" << std::endl;
    for (auto pair : uf.squares)
    {
        int x = square_size * (pair.first.x - minx) + margin;
        int y = square_size * (pair.first.y - miny) + margin;
        os << "<rect x=\"" << x << "\" y=\"" << y << "\" width=\"" << square_size << "\" height=\"" << square_size << "\", style=\"" << square_style[pair.second.type] << "\"/>" << std::endl;
    }
    os << "</svg>" << std::endl;
    return os;
}

// cube for one-layer polycubes
struct plane_cube
{
    int index;
    plane_position pos;
    // in order: up, left, down, right
    std::vector<bool> circumefence = {false, false, false, false};
};

// class representing one-layer polycubes
class plane_polycube
{
public:
    int n;
    int h = 0;
    std::vector<std::set<plane_position>> holes;
    std::set<plane_position> hole_cubes;
    std::map<plane_position, plane_cube> cubes;
    std::vector<std::pair<plane_position, direction::direction>> circumference;
    void caluclate_circumference();
    void calculate_holes();
    unfolding unfold_no_holes();

private:
    void hole_dfs(plane_position pos, int hole_index);
};

// unfolds one-layer polycube without holes
unfolding plane_polycube::unfold_no_holes()
{
    assert(n > 0);
    assert(h == 0);
    unfolding uf;
    plane_position min_pos = {0, (int)1e9};
    for (auto pair : circumference)
    {
        if (pair.second == direction::down && pair.first.y < min_pos.y)
            min_pos = pair.first;
    }
    plane_position pos = min_pos.down();
    for (int i = 0; i < (int)circumference.size(); i++, pos = pos.right())
        uf.squares[pos] = {pos, square_type::circumference};
    for (auto pair : cubes)
    {
        uf.squares[pair.first] = {pair.first, square_type::top_base};
        pos = {pair.first.x, 2 * min_pos.y - pair.first.y - 2};
        uf.squares[pos] = {pos, square_type::bottom_base};
    }
    return uf;
}

// dfs searching for holes
void plane_polycube::hole_dfs(plane_position pos, int hole_index)
{
    if (holes[hole_index].count(pos) || cubes.count(pos))
        return;
    holes[hole_index].insert(pos);
    hole_cubes.insert(pos);
    for (auto neighbor : pos.neighbors())
        hole_dfs(neighbor, hole_index);
}

// finds all holes and saves them into appripriate sets
void plane_polycube::calculate_holes()
{
    for (auto &&pair : cubes)
    {
        direction::direction dir = direction::up;
        do
        {
            plane_position neighbor = pair.first.neighbors()[dir];
            if (!cubes.count(neighbor) && !pair.second.circumefence[dir] && !hole_cubes.count(neighbor))
            {
                holes.push_back({});
                hole_dfs(neighbor, h++);
            }
            dir++;
        } while (dir != direction::up);
    }
}

// calculates direction of circumference for every cube
void plane_polycube::caluclate_circumference()
{
    // start with the lowest cube of the leftmost column
    assert(n);
    circumference.clear();
    plane_position start = cubes.begin()->first;
    direction::direction dir = direction::left;
    plane_position pos = start;
    // annoying edge-case
    if (n == 1)
    {
        do
        {
            cubes[pos].circumefence[dir] = true;
            circumference.push_back({pos, dir});
            dir++;
        } while (dir != direction::left);
        return;
    }
    bool first = true;
    // continue untill we return to the starting point
    while (first || pos != start)
    {
        while (!cubes.count(pos.neighbors()[dir]))
        {
            cubes[pos].circumefence[dir] = true;
            circumference.push_back({pos, dir});
            dir++;
        }
        // find the next cube and direction
        pos = pos.neighbors()[dir];
        dir--;
        if (cubes.count(pos.neighbors()[dir]))
        {
            pos = pos.neighbors()[dir];
            dir--;
        }
        first = false;
    }
    // finish the rest of the first cube
    while (!cubes.count(pos.neighbors()[dir]))
    {
        if (dir == direction::left)
            break;
        circumference.push_back({pos, dir});
        cubes[pos].circumefence[dir] = true;
        dir++;
    }
}

// main class for polycube
class polycube
{
public:
    int n;
    std::map<position, cube> cubes;
    bool connected();
    bool polyhedron();
    int one_layer();
    plane_polycube to_one_layer();

private:
    int dfs(position pos);
};

// deletes one of coordinates from position
plane_position from_position(position pos, int axis)
{
    assert(axis >= 1 && axis <= 3);
    if (axis == 1)
        return {pos.y, pos.z};
    else if (axis == 2)
        return {pos.x, pos.z};
    return {pos.x, pos.y};
}

// creates one-layer polycube from polycube (basically just deletes one of the coordinates)
plane_polycube polycube::to_one_layer()
{
    plane_polycube pl_pc;
    pl_pc.n = n;
    int axis = one_layer();
    assert(axis);
    for (auto pair : cubes)
    {
        plane_cube pl_c;
        pl_c.index = pair.second.index;
        plane_position pos = from_position(pair.first, axis);
        pl_c.pos = pos;
        pl_pc.cubes[pos] = pl_c;
    }
    return pl_pc;
}

// returns 0 in case of multi-layer polycube, otherwise returns index representing the planar axis
int polycube::one_layer()
{
    bool x = true, y = true, z = true;
    if (!cubes.size())
        return 1;
    position first = cubes.begin()->first;
    for (auto pair : cubes)
    {
        x &= pair.first.x == first.x;
        y &= pair.first.y == first.y;
        z &= pair.first.z == first.z;
    }
    return x ? 1 : y ? 2
               : z   ? 3
                     : 0;
}

// auxiliary function for checking connectivity
int polycube::dfs(position pos)
{
    if (!cubes.count(pos) || cubes[pos].visited)
        return 0;
    cubes[pos].visited = true;
    int visited = 1;
    for (auto neighbor : pos.neighbors())
        visited += dfs(neighbor);
    return visited;
}

// returns true iff the polycube is connected
bool polycube::connected()
{
    for (auto pair : cubes)
        cubes[pair.first].visited = false;
    return !n || n == dfs(cubes.begin()->first);
}

// reads triplets of coordinates till the EOF
std::istream &operator>>(std::istream &is, polycube &pc)
{
    position pos;
    cube c;
    pc.n = 0;
    while (is >> pos.x >> pos.y >> pos.z)
    {
        c.pos = pos;
        c.index = pc.n++;
        pc.cubes[pos] = c;
    }
    return is;
}

// checks, whether the polycube is a polyhedron (no holes, simple surface)
bool polycube::polyhedron()
{
    //TODO
    return true;
}

int main()
{
    polycube pc;
    std::cin >> pc;
    std::cerr << "Loaded polycube consisting of " << pc.n << " cubes." << std::endl;
    if (pc.connected())
        std::cerr << "The polycube is connected." << std::endl;
    else
    {
        std::cerr << "The polycube is not connected, please enter a connceted polycube." << std::endl;
        return 0;
    }
    if (pc.one_layer())
    {
        std::cerr << "The polycube is one-layered." << std::endl;
        plane_polycube pl_pc = pc.to_one_layer();
        pl_pc.caluclate_circumference();
        std::cerr << "The circumference has lenght " << pl_pc.circumference.size() << "." << std::endl;
        pl_pc.calculate_holes();
        if (pl_pc.h == 0)
        {
            std::cerr << "The polycube contains no holes." << std::endl;
            unfolding uf = pl_pc.unfold_no_holes();
            std::cout << uf;
        }
        else if (pl_pc.h == (int)pl_pc.hole_cubes.size())
            std::cerr << "The polycube contains " << pl_pc.h << " holes, all of which are cubic." << std::endl;
        else
            std::cerr << "The polycube contains " << pl_pc.h << " holes." << std::endl;
    }
    else
    {
        std::cerr << "I can only unfold one-layer polycubes now, this may change in the future." << std::endl;
    }
    return 0;
}
