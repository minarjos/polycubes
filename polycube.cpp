#include <map>
#include <vector>
#include <fstream>
#include <iostream>

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

// main class for polycube
class polycube
{
public:
    int n;
    std::map<position, cube> cubes;
    bool connected();
    bool polyhedron();
    int one_layer();

private:
    int dfs(position pos);
};

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

// it might be useful to distinguish different types of squares
enum class square_type
{
    top_base,
    bottom_base,
    circumference,
    hole
};

// same as position, one for two coordinates
struct plane_position
{
    int x, y;

    bool operator<(const plane_position &pos) const
    {
        return x == pos.x ? y < pos.y : x < pos.x;
    }

    plane_position up() { return {x, y + 1}; }
    plane_position down() { return {x, y - 1}; }
    plane_position left() { return {x - 1, y}; }
    plane_position right() { return {x + 1, y}; }
};

// struct representing square of unfolding
struct square
{
    plane_position pos;
    square_type type;
};

// plane of the unfolding, consists of several squares
class plane
{
public:
    std::map<plane_position, square> squares;
};

int main()
{
    polycube pc;
    std::cin >> pc;
    std::cout << pc.connected() << std::endl;
    std::cout << pc.one_layer() << std::endl;
    return 0;
}
