#include <iostream>
#include <string>
#include <vector>
#include <omp.h>
#include "lib.h"
using namespace std;

GCell::GCell() 
{
    this->adjList.resize(4, pair<GCell*, int>(NULL, 0));
    this->costF = 0;
    this->costG = 0;
    this->costH = 0;
    this->costC = 0;
    this->parent = NULL;
    this->open = 0;
    this->close = 0;
}
void GCell::setAdjList(vector<vector<GCell> > &GCells, int row, int column, pair<int, int> &grid, int &vertCapacity, int &horizCapacity)
{
    //cout << column << " " << row << ": ";
    if (0 <= row && row < grid.second - 1) {
        this->adjList[0].first = &GCells[row+1][column]; //上
        this->adjList[0].second = vertCapacity;
        //cout << column << " " << row+1 << ", ";
    }    
    if (0 < row && row <= grid.second - 1) {
        this->adjList[2].first = &GCells[row-1][column]; //下
        this->adjList[2].second = vertCapacity;
        //cout << column << " " << row-1 << ", ";
    }
    if (0 <= column && column < grid.first - 1) {
        this->adjList[1].first = &GCells[row][column+1]; //右
        this->adjList[1].second = horizCapacity;
        //cout << column+1 << " " << row << ", ";
    }
    if (0 < column && column <= grid.first - 1) {
        this->adjList[3].first = &GCells[row][column-1]; //左
        this->adjList[3].second = horizCapacity;
        //cout << column-1 << " " << row;
    }
    //cout << endl;
    return;
}
Net::Net() 
{
    this->pinsCoord.resize(2);
    this->id = 0;
    this->HPWL = 0;
    this->overflow = 0;
}
/*Edge::Edge(pair<int, int> &grid)
{
    this->vertEdge.resize(grid.first, vector<pair<int, int>>(grid.second-1));
    this->horizEdge.resize(grid.second, vector<pair<int, int>>(grid.first-1));
}*/