#include <set>
#include <map>
#include <array>
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

    bool operator==(const position pos) const
    {
        return x == pos.x && y == pos.y && z == pos.z;
    }

    bool operator!=(const position pos) const
    {
        return x != pos.x || y != pos.y || z != pos.z;
    }

    position left() const { return {x - 1, y, z}; }
    position right() const { return {x + 1, y, z}; }
    position down() const { return {x, y - 1, z}; }
    position up() const { return {x, y + 1, z}; }
    position front() const { return {x, y, z - 1}; }
    position back() const { return {x, y, z + 1}; }

    // list of neighboring positions
    std::vector<position> neighbors() const
    {
        return {
            left(),
            right(),
            down(),
            up(),
            front(),
            back()};
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
        {square_type::top_base, "fill:blue;stroke:black;stroke-width:5;fill-opacity:0.5"},
        {square_type::bottom_base, "stroke-alignment:inner;fill:blue;stroke:black;stroke-width:5;fill-opacity:0.5"},
        {square_type::circumference, "fill:red;stroke:black;stroke-width:5;fill-opacity:0.5"},
        {square_type::hole, "fill:purple;stroke:black;stroke-width:5;fill-opacity:0.7"},
};

// enum for directions in space
namespace direction_3d
{
    enum direction
    {
        left,
        right,
        down,
        up,
        front,
        back
    };
}

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

    direction operator+(const direction dir, const int x)
    {
        return static_cast<direction>(((static_cast<int>(dir) + x) % 4 + 4) % 4);
    }

    direction operator-(const direction dir, const int x)
    {
        return static_cast<direction>(((static_cast<int>(dir) - x) % 4 + 4) % 4);
    }

    // cyclic increment
    direction operator++(direction &dir, int)
    {
        dir = dir + 1;
        return dir - 1;
    }
    direction operator--(direction &dir, int)
    {
        dir = dir - 1;
        return dir + 1;
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

struct face
{
    position pos;
    direction_3d::direction dir;

    bool operator< (const face &f) const
    {
        if (pos == f.pos)
            return dir < f.dir;
        return pos < f.pos;
    }
};

class surface
{
public:
    std::map<face, std::array<face, 4>> graph;
    void connect(face f1, direction::direction d1, face f2, direction::direction d2);

private:
};

void surface::connect(face f1, direction::direction d1, face f2, direction::direction d2)
{
    graph[f1][d1] = f2;
    graph[f2][d2] = f1;
}

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
    int height = (maxy - miny + 1) * square_size + 2 * margin;
    int width = (maxx - minx + 1) * square_size + 2 * margin;
    os << "<svg width=\"" << width << "\" height=\"" << height << "\">" << std::endl;
    for (auto pair : uf.squares)
    {
        int x = square_size * (pair.first.x - minx) + margin;
        int y = square_size * (maxy - pair.first.y) + margin;
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
    bool big_holes();
    unfolding unfold_no_holes();
    unfolding unfold_big_holes();
    unfolding unfold_1x1();
    unfolding unfold_orthotree();

private:
    void hole_dfs(plane_position pos, int hole_index);
    void stripe(plane_position head, plane_position uf_head, direction::direction dir, direction::direction uf_dir, unfolding &uf);
    int degree(plane_position pos);
};

// returns number of neighboring cubes
int plane_polycube::degree(plane_position pos)
{
    int deg = 0;
    for (auto neighbor : pos.neighbors())
        deg += cubes.count(neighbor);
    return deg;
}

// unfolds one-layer orthotree to a unfolding of height 3
unfolding plane_polycube::unfold_orthotree()
{
    unfolding uf;
    plane_position uf_pos = {0, 0};
    // maping from cube and direction to position in unfolding
    std::map<std::pair<plane_position, direction::direction>, plane_position> to_uf;
    for (int i = 0; i < (int)circumference.size(); i++, uf_pos = uf_pos.right())
    {
        uf.squares[uf_pos] = {uf_pos, square_type::circumference};
        to_uf[circumference[i]] = uf_pos;
    }
    for (auto pair : cubes)
    {
        // we start to the left, this is important
        direction::direction dir = direction::left;
        for (int i = 0; i < 4; i++, dir++)
        {
            if (pair.second.circumefence[dir])
            {
                uf.squares[to_uf[{pair.first, dir}].up()] = {to_uf[{pair.first, dir}].up(), square_type::top_base};
                uf.squares[to_uf[{pair.first, dir}].down()] = {to_uf[{pair.first, dir}].down(), square_type::bottom_base};
                // if the left neighbor has degree 4, we unfold it together with this square
                if (cubes.count(pair.first.left()) && degree(pair.first.left()) == 4)
                {
                    uf.squares[to_uf[{pair.first, dir}].up().left()] = {to_uf[{pair.first, dir}].up().left(), square_type::top_base};
                    uf.squares[to_uf[{pair.first, dir}].down().left()] = {to_uf[{pair.first, dir}].down().left(), square_type::bottom_base};
                }
                break;
            }
        }
    }
    return uf;
}

// unfolds stripe of width from given position and direction
void plane_polycube::stripe(plane_position head, plane_position uf_head, direction::direction dir, direction::direction uf_dir, unfolding &uf)
{
    int orientation = uf_dir == direction::up ? -1 : 1;
    square_type st = uf_dir == direction::up ? square_type::top_base : square_type::bottom_base;
    plane_position other_head = head.neighbors()[dir - 1];
    plane_position other_uf_head = uf_head.neighbors()[uf_dir + orientation];

    if (cubes.count(head.neighbors()[dir + 2]) || cubes.count(other_head.neighbors()[dir + 2]))
        return;

    while (cubes.count(head) || cubes.count(other_head))
    {

        // the shifts and rotaions are a bit confusing
        // TODO: check this
        if (cubes.count(head))
            uf.squares[uf_head] = {uf_head, st};
        else if (hole_cubes.count(head))
            uf.squares[uf_head] = {uf_head, square_type::hole};

        if (cubes.count(other_head))
            uf.squares[other_uf_head] = {other_uf_head, st};
        else if (hole_cubes.count(other_head))
            uf.squares[other_uf_head] = {other_uf_head, square_type::hole};

        if (hole_cubes.count(head.neighbors()[dir + 1]))
            uf.squares[uf_head.neighbors()[uf_dir - orientation]] = {uf_head.neighbors()[uf_dir - orientation], square_type::hole};
        if (hole_cubes.count(other_head.neighbors()[dir - 1]))
            uf.squares[other_uf_head.neighbors()[uf_dir + orientation]] = {other_uf_head.neighbors()[uf_dir + orientation], square_type::hole};

        head = head.neighbors()[dir];
        other_head = other_head.neighbors()[dir];
        uf_head = uf_head.neighbors()[uf_dir];
        other_uf_head = other_uf_head.neighbors()[uf_dir];
    }
}

// unfolds one-layer polyhedra with 1x1 holes
unfolding plane_polycube::unfold_1x1()
{
    assert(h == (int)hole_cubes.size());
    unfolding uf;
    plane_position uf_pos = {0, 0};
    // we go around the circumcircle and start stripes in correct places and directions
    for (int i = 0; i < (int)circumference.size(); i++, uf_pos = uf_pos.right())
    {
        uf.squares[uf_pos] = {uf_pos, square_type::circumference};
        plane_position pos = circumference[i].first;
        direction::direction dir = circumference[i].second;

        // the places and directions are quite messy, there might be some mistake
        if (dir == direction::left && (pos.y % 4 + 4) % 4 == 0)
            stripe(pos.up(), uf_pos.up().left(), direction::right, direction::up, uf);
        if (dir == direction::left && (pos.y % 4 + 4) % 4 == 1 && !cubes.count(pos.down()))
            stripe(pos, uf_pos.up(), direction::right, direction::up, uf);

        if (dir == direction::right && (pos.y % 4 + 4) % 4 == 2)
            stripe(pos, uf_pos.up(), direction::left, direction::up, uf);
        if (dir == direction::right && (pos.y % 4 + 4) % 4 == 3 && !cubes.count(pos.down()))
            stripe(pos.down(), uf_pos.up().left(), direction::left, direction::up, uf);

        if (dir == direction::up && (pos.x % 4 + 4) % 4 == 1)
            stripe(pos, uf_pos.down(), direction::down, direction::down, uf);
        if (dir == direction::up && (pos.x % 4 + 4) % 4 == 0 && !cubes.count(pos.right()))
            stripe(pos.right(), uf_pos.down().left(), direction::down, direction::down, uf);

        if (dir == direction::down && (pos.x % 4 + 4) % 4 == 2)
            stripe(pos, uf_pos.down(), direction::up, direction::down, uf);
        if (dir == direction::down && (pos.x % 4 + 4) % 4 == 3 && !cubes.count(pos.left()))
            stripe(pos.left(), uf_pos.down().left(), direction::up, direction::down, uf);
    }
    return uf;
}

std::ostream &operator<<(std::ostream &os, plane_polycube &pl_pc)
{
    unfolding uf;
    for (auto pair : pl_pc.cubes)
        uf.squares[pair.first] = {pair.first, square_type::top_base};
    os << uf;
    return os;
}

// checkes, whether there is one-wide gap in some of the holes
bool plane_polycube::big_holes()
{
    for (auto pos : hole_cubes)
    {
        if (!hole_cubes.count(pos.left()) && !hole_cubes.count(pos.right()))
            return false;
        if (!hole_cubes.count(pos.up()) && !hole_cubes.count(pos.down()))
            return false;
    }
    return true;
}

// unfolds one-layer polycube with big holes
unfolding plane_polycube::unfold_big_holes()
{
    assert(n > 0);
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
        // unfold left and right parts of hole
        if (!cubes[pair.first].circumefence[direction::left] && !cubes.count(pair.first.left()))
            uf.squares[pair.first.left()] = {pair.first.left(), square_type::hole};
        if (!cubes[pair.first].circumefence[direction::right] && !cubes.count(pair.first.right()))
            uf.squares[pair.first.right()] = {pair.first.right(), square_type::hole};

        pos = {pair.first.x, 2 * min_pos.y - pair.first.y - 2};
        // unfold top and bottom parts of hole
        uf.squares[pos] = {pos, square_type::bottom_base};
        if (!cubes[pair.first].circumefence[direction::up] && !cubes.count(pair.first.up()))
            uf.squares[pos.down()] = {pos.down(), square_type::hole};
        if (!cubes[pair.first].circumefence[direction::down] && !cubes.count(pair.first.down()))
            uf.squares[pos.up()] = {pos.up(), square_type::hole};
    }
    return uf;
}

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
            circumference.push_back({pos, dir++});
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
            circumference.push_back({pos, dir++});
        }
        // find the next cube and direction
        pos = pos.neighbors()[dir--];
        if (cubes.count(pos.neighbors()[dir]))
            pos = pos.neighbors()[dir--];
        first = false;
    }
    // finish the rest of the first cube
    while (!cubes.count(pos.neighbors()[dir]))
    {
        if (dir == direction::left)
            break;
        circumference.push_back({pos, dir});
        cubes[pos].circumefence[dir++] = true;
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
    bool orthotree();
    surface get_surface();

private:
    int dfs(position pos);
    bool ortho_dfs(position pos, position from);
    void connect_on_cube(position pos, surface &surf);
    void connect_face_neighbors(position pos, surface &surf);
    void connect_edge_neighbors(position pos, surface &surf);
};

void polycube::connect_on_cube(position pos, surface &surf)
{
    if (!cubes.count(pos.left()) && !cubes.count(pos.front()))
        surf.connect({pos, direction_3d::left}, direction::right, {pos, direction_3d::front}, direction::left);
    if (!cubes.count(pos.front()) && !cubes.count(pos.right()))
        surf.connect({pos, direction_3d::front}, direction::right, {pos, direction_3d::right}, direction::left);
    if (!cubes.count(pos.right()) && !cubes.count(pos.back()))
        surf.connect({pos, direction_3d::right}, direction::right, {pos, direction_3d::back}, direction::left);
    if (!cubes.count(pos.back()) && !cubes.count(pos.left()))
        surf.connect({pos, direction_3d::back}, direction::right, {pos, direction_3d::left}, direction::left);

    if (!cubes.count(pos.up()) && !cubes.count(pos.front()))
        surf.connect({pos, direction_3d::up}, direction::down, {pos, direction_3d::front}, direction::up);
    if (!cubes.count(pos.up()) && !cubes.count(pos.right()))
        surf.connect({pos, direction_3d::up}, direction::right, {pos, direction_3d::right}, direction::up);
    if (!cubes.count(pos.up()) && !cubes.count(pos.back()))
        surf.connect({pos, direction_3d::up}, direction::up, {pos, direction_3d::back}, direction::up);
    if (!cubes.count(pos.up()) && !cubes.count(pos.left()))
        surf.connect({pos, direction_3d::up}, direction::left, {pos, direction_3d::left}, direction::up);

    if (!cubes.count(pos.down()) && !cubes.count(pos.front()))
        surf.connect({pos, direction_3d::down}, direction::down, {pos, direction_3d::front}, direction::down);
    if (!cubes.count(pos.down()) && !cubes.count(pos.right()))
        surf.connect({pos, direction_3d::down}, direction::left, {pos, direction_3d::right}, direction::down);
    if (!cubes.count(pos.down()) && !cubes.count(pos.back()))
        surf.connect({pos, direction_3d::down}, direction::up, {pos, direction_3d::back}, direction::down);
    if (!cubes.count(pos.down()) && !cubes.count(pos.left()))
        surf.connect({pos, direction_3d::down}, direction::right, {pos, direction_3d::left}, direction::down);
}
void polycube::connect_face_neighbors(position pos, surface &surf)
{
    if (cubes.count(pos.right()))
    {
        if (!cubes.count(pos.front()) && !cubes.count(pos.front().right()))
            surf.connect({pos, direction_3d::front}, direction::right, {pos.right(), direction_3d::front}, direction::left);
        if (!cubes.count(pos.up()) && !cubes.count(pos.up().right()))
            surf.connect({pos, direction_3d::up}, direction::right, {pos.right(), direction_3d::up}, direction::left);
        if (!cubes.count(pos.back()) && !cubes.count(pos.back().right()))
            surf.connect({pos, direction_3d::back}, direction::left, {pos.right(), direction_3d::back}, direction::right);
        if (!cubes.count(pos.down()) && !cubes.count(pos.down().right()))
            surf.connect({pos, direction_3d::down}, direction::left, {pos.right(), direction_3d::down}, direction::right);
    }
    if (cubes.count(pos.up()))
    {
        if (!cubes.count(pos.front()) && !cubes.count(pos.front().up()))
            surf.connect({pos, direction_3d::front}, direction::up, {pos.up(), direction_3d::front}, direction::down);
        if (!cubes.count(pos.right()) && !cubes.count(pos.right().up()))
            surf.connect({pos, direction_3d::right}, direction::up, {pos.up(), direction_3d::right}, direction::down);
        if (!cubes.count(pos.back()) && !cubes.count(pos.back().up()))
            surf.connect({pos, direction_3d::back}, direction::up, {pos.up(), direction_3d::back}, direction::down);
        if (!cubes.count(pos.left()) && !cubes.count(pos.left().up()))
            surf.connect({pos, direction_3d::left}, direction::up, {pos.up(), direction_3d::left}, direction::down);
    }
    if (cubes.count(pos.back()))
    {
        if (!cubes.count(pos.left()) && !cubes.count(pos.left().back()))
            surf.connect({pos, direction_3d::left}, direction::left, {pos.back(), direction_3d::left}, direction::right);
        if (!cubes.count(pos.up()) && !cubes.count(pos.up().back()))
            surf.connect({pos, direction_3d::up}, direction::up, {pos.back(), direction_3d::up}, direction::down);
        if (!cubes.count(pos.right()) && !cubes.count(pos.right().back()))
            surf.connect({pos, direction_3d::right}, direction::right, {pos.back(), direction_3d::right}, direction::left);
        if (!cubes.count(pos.down()) && !cubes.count(pos.down().back()))
            surf.connect({pos, direction_3d::down}, direction::up, {pos.back(), direction_3d::down}, direction::down);
    }
}
void polycube::connect_edge_neighbors(position pos, surface &surf)
{
    if (cubes.count(pos.up().front()))
    {
        if (!cubes.count(pos.front()))
            surf.connect({pos, direction_3d::front}, direction::up, {pos.up().front(), direction_3d::down}, direction::up);
        if (!cubes.count(pos.up()))
            surf.connect({pos, direction_3d::up}, direction::down, {pos.up().front(), direction_3d::back}, direction::down);
    }
    if (cubes.count(pos.up().back()))
    {
        if (!cubes.count(pos.back()))
            surf.connect({pos, direction_3d::back}, direction::up, {pos.up().back(), direction_3d::down}, direction::down);
        if (!cubes.count(pos.up()))
            surf.connect({pos, direction_3d::up}, direction::up, {pos.up().back(), direction_3d::front}, direction::down);
    }
    if (cubes.count(pos.up().left()))
    {
        if (!cubes.count(pos.left()))
            surf.connect({pos, direction_3d::left}, direction::up, {pos.up().left(), direction_3d::down}, direction::left);
        if (!cubes.count(pos.up()))
            surf.connect({pos, direction_3d::up}, direction::left, {pos.up().left(), direction_3d::right}, direction::down);
    }
    if (cubes.count(pos.up().right()))
    {
        if (!cubes.count(pos.right()))
            surf.connect({pos, direction_3d::right}, direction::up, {pos.up().right(), direction_3d::down}, direction::right);
        if (!cubes.count(pos.up()))
            surf.connect({pos, direction_3d::up}, direction::right, {pos.up().right(), direction_3d::left}, direction::down);
    }
    if (cubes.count(pos.back().left()))
    {
        if (!cubes.count(pos.left()))
            surf.connect({pos, direction_3d::left}, direction::left, {pos.back().left(), direction_3d::front}, direction::right);
        if (!cubes.count(pos.back()))
            surf.connect({pos, direction_3d::back}, direction::right, {pos.back().left(), direction_3d::right}, direction::left);
    }
    if (cubes.count(pos.back().right()))
    {
        if (!cubes.count(pos.right()))
            surf.connect({pos, direction_3d::right}, direction::right, {pos.back().right(), direction_3d::front}, direction::left);
        if (!cubes.count(pos.back()))
            surf.connect({pos, direction_3d::back}, direction::left, {pos.back().right(), direction_3d::left}, direction::right);
    }
}

surface polycube::get_surface()
{
    surface surf;
    for (auto pair : cubes)
    {
        position pos = pair.first;
        connect_on_cube(pos, surf);
        connect_face_neighbors(pos, surf);
        connect_edge_neighbors(pos, surf);
    }
    return surf;
}

// returns false if there is a cycle
bool polycube::ortho_dfs(position pos, position from)
{
    if (!cubes.count(pos))
        return true;
    if (cubes[pos].visited)
        return false;
    cubes[pos].visited = true;
    for (auto neighbor : pos.neighbors())
        if (neighbor != from && !ortho_dfs(neighbor, pos))
            return false;
    return true;
}

// returs true iff the polycube is orthotree
bool polycube::orthotree()
{
    for (auto pair : cubes)
        cubes[pair.first].visited = false;
    if (!n)
        return true;
    return ortho_dfs(cubes.begin()->first, cubes.begin()->first);
}

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

// checks, whether the polycube is a polyhedron (simple connected surface)
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
    if (pc.orthotree())
        std::cerr << "The polycube is an orthotree." << std::endl;
    surface surf = pc.get_surface();
    for(auto pair:surf.graph)
    {
        std::cerr << pair.first.pos.x << " " << pair.first.pos.y << " " << pair.first.pos.z << " " << pair.first.dir << ":" << std::endl;
        for (auto fc : pair.second)
            std::cerr << "\t" << fc.pos.x << " " << fc.pos.y << " " << fc.pos.z << " " << fc.dir << std::endl;
    }
    if (pc.one_layer())
    {
        std::cerr << "The polycube is one-layered." << std::endl;
        plane_polycube pl_pc = pc.to_one_layer();
        std ::cout << pl_pc;
        pl_pc.caluclate_circumference();
        std::cerr << "The circumference has lenght " << pl_pc.circumference.size() << "." << std::endl;
        pl_pc.calculate_holes();
        if (pc.orthotree())
        {
            std::cerr << "The polycube contains no holes, it can be unfolded to a 3-wide stripe." << std::endl;
            unfolding uf = pl_pc.unfold_orthotree();
            std::cout << uf;
        }
        else if (pl_pc.h == 0)
        {
            std::cerr << "The polycube contains no holes, it can be unfolded using simple algorithm." << std::endl;
            unfolding uf = pl_pc.unfold_no_holes();
            std::cout << uf;
        }
        else if (pl_pc.h == (int)pl_pc.hole_cubes.size())
        {
            std::cerr << "The polycube contains " << pl_pc.h << " holes, all of which are cubic. I can unfold this." << std::endl;
            unfolding uf = pl_pc.unfold_1x1();
            std::cout << uf;
        }
        else if (pl_pc.big_holes())
        {
            std::cerr << "The polycube contains " << pl_pc.h << " holes, all of which are at least 2-wide. I can unfold this." << std::endl;
            unfolding uf = pl_pc.unfold_big_holes();
            std::cout << uf;
        }
        else
        {
            std::cerr << "The polycube contains " << pl_pc.h << " holes." << std::endl;
            std::cerr << "I can't unfold general one-layer polycubes yet." << std::endl;
        }
    }
    else
    {
        std::cerr << "I can only unfold one-layer polycubes now, this may change in the future." << std::endl;
    }
    return 0;
}
