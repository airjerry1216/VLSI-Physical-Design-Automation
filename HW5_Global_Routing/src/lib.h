#ifndef LIB_H_
#define LIB_H_
#include <string>
#include <vector>

class GCell {
public:
    std::pair<int, int> coord;
    std::vector<std::pair<GCell*, int> > adjList; // <0無邊 1有邊, remain capacity>
    double costF;
    double costG;
    double costH;
    double costC;
    GCell *parent;
    bool open;
    bool close;
    GCell();
    void setAdjList(std::vector<std::vector<GCell> > &GCells, int row, int column, std::pair<int, int> &grid, int &vertCapacity, int &horizCapacity);
};
class Net {
public:
    std::string name;
    int id;
    int HPWL;
    std::vector<std::pair<int, int> > pinsCoord;
    std::vector<std::pair<int, int> > path;
    std::vector<std::pair<int, int> > corner;
    int overflow;
    Net();
};
/*class Edge {
public:
    std::vector<std::vector<std::pair<int, int> > > vertEdge;
    std::vector<std::vector<std::pair<int, int> > > horizEdge;
    Edge(std::pair<int, int> &grid);
};*/
#endif
