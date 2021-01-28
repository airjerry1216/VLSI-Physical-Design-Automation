#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <queue>
#include <climits>
#include <cmath>
#include <omp.h>
#include "lib.h"
using namespace std;
// ../bin/hw5 ../testcase/ibm01.modified.txt ../output/ibm01.result
//./verifier/verify ./testcase/ibm01.modified.txt ./output/ibm01.result
class Compare {
public:
    bool operator()(GCell *a, GCell *b) {
        return a->costF > b->costF;
    }    
};
void setGCell(vector<vector<GCell> > &GCells, pair<int, int> &grid, int &vertCapacity, int &horizCapacity)
{
    //#pragma omp parallel for collapse(2)
    for (int i = 0; i < grid.second; ++i) {
        for (int j = 0; j < grid.first; ++j) {
            GCells[i][j].setAdjList(GCells, i, j, grid, vertCapacity, horizCapacity);
            GCells[i][j].coord.first = j;
            GCells[i][j].coord.second = i;
        }  
    }
    return;
}
void clearGCellCost(vector<vector<GCell> > &GCells, pair<int, int> &grid)
{
    //#pragma omp parallel for collapse(2)
    for (int i = 0; i < grid.second; ++i) {
        for (int j = 0; j < grid.first; ++j) {
            GCells[i][j].costF = 0;
            GCells[i][j].costG = 0;
            GCells[i][j].costH = 0;
            GCells[i][j].parent = NULL;
            GCells[i][j].open = 0;
            GCells[i][j].close = 0;
        }
    }
    return;
}
void clearPath(vector<vector<GCell> > &GCells, Net &net)
{
    //cout << "---" << net.id << "---" << endl;
    for (int i = 0; i < net.path.size() - 1; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (GCells[net.path[i].second][net.path[i].first].adjList[j].first == &GCells[net.path[i+1].second][net.path[i+1].first]) {
                ++(GCells[net.path[i].second][net.path[i].first].adjList[j].second);
                ++(GCells[net.path[i].second][net.path[i].first].adjList[j].first->adjList[(j+2)%4].second);
                break;
            }
        }
    }
    return;
}
void LShapeRoute(vector<vector<GCell> > &GCells, vector<Net> &nets, int &num_net)
{
    for (int i = 0; i < num_net; ++i) {
        nets[i].corner.push_back(nets[i].pinsCoord[0]);
        if(nets[i].pinsCoord[0].first != nets[i].pinsCoord[1].first && nets[i].pinsCoord[0].second != nets[i].pinsCoord[1].second) {
            double shapeType = (double) rand() / (RAND_MAX + 1.0);
            if (shapeType >= 0.5)
                nets[i].corner.push_back(make_pair(nets[i].pinsCoord[1].first, nets[i].pinsCoord[0].second)); //水平走
            else
                nets[i].corner.push_back(make_pair(nets[i].pinsCoord[0].first, nets[i].pinsCoord[1].second)); //垂直走
        }
        nets[i].corner.push_back(nets[i].pinsCoord[1]);
    }
    return;
}
bool pushToOpen(vector<vector<GCell> > &GCells, Net &net, pair<int, int> &grid, GCell *top, priority_queue<GCell*, vector<GCell*>, Compare> &open, int &vertCapacity, int &horizCapacity)
{
    bool finish = 0;
    for (int i = 0; i < 4; ++i) {
        if (top->adjList[i].first != NULL) {
            //如果此GCell已在priority queue中
            if (top->adjList[i].first->open) {
                double costG = top->costG + 1;
                double costH = abs(net.pinsCoord[1].first - top->adjList[i].first->coord.first) + abs(net.pinsCoord[1].second - top->adjList[i].first->coord.second);
                double costC = 0;
                if (top->adjList[i].second > 0) {
                    if (i%2)
                        costC = (horizCapacity - top->adjList[i].second) / 4;// / horizCapacity * 2;
                    else
                        costC = (vertCapacity - top->adjList[i].second) / 4;// / vertCapacity * 2;
                    //costC = costC / 4;
                }
                else
                    costC =  abs(top->adjList[i].second - 1) * 4096;
                double costM = min(min(grid.first -  top->adjList[i].first->coord.first, top->adjList[i].first->coord.first), min(grid.second -  top->adjList[i].first->coord.second, top->adjList[i].first->coord.second));
                double costF = costG + costH * 0.01 + costC + costM / 2;
                if (costF < top->adjList[i].first->costF) {
                    vector<GCell*> tmp;
                    while(!open.empty()) {
                        GCell *it = open.top();
                        open.pop();
                        if (it != top->adjList[i].first)
                            tmp.push_back(it);
                    }
                    for (int k = 0; k < tmp.size(); ++k)
                        open.push(tmp[k]); 
                    top->adjList[i].first->parent = top;
                    top->adjList[i].first->costG = costG;
                    top->adjList[i].first->costH = costH;
                    top->adjList[i].first->costC = costC;
                    top->adjList[i].first->costF = costF;
                    open.push(top->adjList[i].first);
                }
            }
            //如果此GCell不在priority queue中且還沒拜訪過
            else if (!top->adjList[i].first->close) {
                top->adjList[i].first->parent = top;
                top->adjList[i].first->costG = top->costG + 1;
                top->adjList[i].first->costH = abs(net.pinsCoord[1].first - top->adjList[i].first->coord.first) + abs(net.pinsCoord[1].second - top->adjList[i].first->coord.second);
                if (top->adjList[i].second > 0) {
                    if (i%2)
                        top->adjList[i].first->costC = (horizCapacity - top->adjList[i].second) / 4;// / horizCapacity * 2;
                    else
                        top->adjList[i].first->costC = (vertCapacity - top->adjList[i].second) / 4;// / vertCapacity * 2;
                    //top->adjList[i].first->costC = top->adjList[i].first->costC / 4;
                }
                else
                    top->adjList[i].first->costC =  abs(top->adjList[i].second - 1) * 4096;
                double costM = min(min(grid.first -  top->adjList[i].first->coord.first, top->adjList[i].first->coord.first), min(grid.second -  top->adjList[i].first->coord.second, top->adjList[i].first->coord.second));
                top->adjList[i].first->costF = top->adjList[i].first->costG + top->adjList[i].first->costH * 0.01  + top->adjList[i].first->costC + costM / 2;
                open.push(top->adjList[i].first);
                top->adjList[i].first->open = 1;
            }
        }
    }
    return finish;
}
void AStarSearch(vector<vector<GCell> > &GCells, Net &net, pair<int, int> &grid, int &vertCapacity, int &horizCapacity)
{
    priority_queue<GCell*, vector<GCell*>, Compare> open; //可走到的GCell
    open.push(&GCells[net.pinsCoord[0].second][net.pinsCoord[0].first]);
    while(!open.empty()) {
        GCell *top = open.top();
        open.pop();
        top->open = 0;
        top->close = 1;
        if (top->coord.first == net.pinsCoord[1].first && top->coord.second == net.pinsCoord[1].second) {
            double costF = INT_MAX;
            for (int i = 0; i < 4; ++i) {
                if (top->adjList[i].first !=NULL) {
                    if (costF > top->adjList[i].first->costF && top->adjList[i].first->close) {
                        costF = top->adjList[i].first->costF;
                        top->parent = top->adjList[i].first;
                    }
                }
            }
            break;
        }
        pushToOpen(GCells, net, grid, top, open, vertCapacity, horizCapacity);
    }
    
    GCell *prev = &GCells[net.pinsCoord[1].second][net.pinsCoord[1].first];
    GCell *ptr =  prev->parent;
    bool isHoriz = (prev->coord.first != ptr->coord.first) ? 1 : 0;
    vector<pair<int, int>> corner, path;
    corner.push_back(make_pair(prev->coord.first, prev->coord.second));
    while (ptr != NULL) {
        path.push_back(make_pair(prev->coord.first, prev->coord.second));
        if (isHoriz != (prev->coord.first != ptr->coord.first)) {
            corner.push_back(make_pair(prev->coord.first, prev->coord.second));
            isHoriz = !isHoriz;
        }
        for (int i = 0; i < 4; ++i) {
            if (prev->adjList[i].first == prev->parent) {
                --(prev->adjList[i].second);
                --(prev->parent->adjList[(i+2)%4].second);
                if (prev->adjList[i].second <= 4)
                    ++net.overflow;
                break;
            }
        }
        prev = prev->parent;
        ptr = ptr->parent;
    }
    path.push_back(make_pair(prev->coord.first, prev->coord.second));
    net.path = path;
    corner.push_back(make_pair(prev->coord.first, prev->coord.second));
    net.corner = corner;
    return;
}
int main(int argc, char *argv[])
{
    ifstream txt_file(argv[1]);
    ofstream result_file(argv[2]);
    ofstream draw_file(argv[3]);
    stringstream ss;
    string line, tmp;
/*--------------read modified.txt------------------------------------------------------------------------------------------------*/
    pair<int, int> grid;
    int vertCapacity = 0, horizCapacity = 0, num_net = 0;
    getline(txt_file, line);
    ss << line;
    ss >> tmp >> grid.first >> grid.second;
    ss.str("");
    ss.clear();
    getline(txt_file, line);
    ss << line;
    ss >> tmp >> tmp >> vertCapacity;
    ss.str("");
    ss.clear();
    getline(txt_file, line);
    ss << line;
    ss >> tmp >> tmp >> horizCapacity;
    ss.str("");
    ss.clear();
    getline(txt_file, line);
    ss << line;
    ss >> tmp >> tmp >> num_net;
    ss.str("");
    ss.clear();

    vector<vector<GCell> > GCells(grid.second, vector<GCell>(grid.first)); //建立二維vector
    setGCell(GCells, grid, vertCapacity, horizCapacity);
    vector<Net> nets(num_net);
    for (int i = 0; i < num_net; ++i) {
        getline(txt_file, line);
        ss << line;
        ss >> nets[i].name >> nets[i].id >> tmp;
        ss.str("");
        ss.clear();
        for (int j = 0; j < 2; ++j) {
            getline(txt_file, line);
            ss << line;
            ss >> nets[i].pinsCoord[j].first >> nets[i].pinsCoord[j].second;
            ss.str("");
            ss.clear();
        }
        nets[i].HPWL = abs(nets[i].pinsCoord[0].first - nets[i].pinsCoord[1].first) + abs(nets[i].pinsCoord[0].second - nets[i].pinsCoord[1].second);
    }
/*-------------------------------------------------------------------------------------------------------------------------------*/
    srand(114);
    //LShapeRoute(GCells, nets, num_net);
    for (int i = 0; i < num_net; ++i) {
        if (nets[i].HPWL < max(grid.first, grid.second) / 4) {
            clearGCellCost(GCells, grid);
            AStarSearch(GCells, nets[i], grid, vertCapacity, horizCapacity);
        }
    }
    for (int i = 0; i < num_net; ++i) {
        if (nets[i].HPWL >= max(grid.first, grid.second) / 4) {
            clearGCellCost(GCells, grid);
            AStarSearch(GCells, nets[i], grid, vertCapacity, horizCapacity);
        }
    }
    for (int reroute = 0; reroute < 6; ++reroute) {
        //int cnt = 0;
        for (int i = 0; i < num_net; ++i) {
            if (nets[i].overflow) {
                //++cnt;
                nets[i].overflow = 0;
                clearPath(GCells, nets[i]);
                clearGCellCost(GCells, grid);
                AStarSearch(GCells, nets[i], grid, vertCapacity, horizCapacity);
            }
        }
        //cout << cnt << endl;
    }
    /*for (int reroute = 0; reroute < 3; ++reroute) {
        int cnt = 0;
        for (int i = num_net - 1; i >= 0; --i) {
            if (nets[i].overflow && nets[i].HPWL < max(grid.first, grid.second) / 4) {
                ++cnt;
                nets[i].overflow = 0;
                clearPath(GCells, nets[i]);
                clearGCellCost(GCells, grid);
                AStarSearch(GCells, nets[i], grid, vertCapacity, horizCapacity);
            }
        }
        cout << cnt << endl;
    }*/
    for (int reroute = 0; reroute < 7; ++reroute) {
        //int cnt = 0;
        for (int i = num_net - 1; i >= 0; --i) {
            if (nets[i].overflow && nets[i].HPWL) {
                //++cnt;
                nets[i].overflow = 0;
                clearPath(GCells, nets[i]);
                clearGCellCost(GCells, grid);
                AStarSearch(GCells, nets[i], grid, vertCapacity, horizCapacity);
            }
        }
        //cout << cnt << endl;
    }
    /*for (int reroute = 0; reroute < 5; ++reroute) {
        int cnt = 0;
        for (int i = num_net - 1; i >= 0; --i) {
            if (nets[i].overflow) {
                ++cnt;
                nets[i].overflow = 0;
                clearPath(GCells, nets[i]);
                clearGCellCost(GCells, grid);
                AStarSearch(GCells, nets[i], grid, vertCapacity, horizCapacity);
            }
        }
        cout << cnt << endl;
    }
    for (int reroute = 0; reroute < 10; ++reroute) {
        int cnt = 0;
        for (int i = 0; i < num_net; ++i) {
            if (nets[i].overflow > 5) {
                ++cnt;
                nets[i].overflow = 0;
                clearPath(GCells, nets[i]);
                clearGCellCost(GCells, grid);
                AStarSearch(GCells, nets[i], grid, vertCapacity, horizCapacity);
            }
        }
        cout << cnt << endl;
    }
    for (int reroute = 0; reroute < 5; ++reroute) {
        int cnt = 0;
        for (int i = 0; i < num_net; ++i) {
            if (nets[i].overflow) {
                ++cnt;
                nets[i].overflow = 0;
                clearPath(GCells, nets[i]);
                clearGCellCost(GCells, grid);
                AStarSearch(GCells, nets[i], grid, vertCapacity, horizCapacity);
            }
        }
        cout << cnt << endl;
    }*/
    /*for (int i = num_net - 1; i > num_net - 10; --i) {
        if (nets[i].overflow) {
            cout << "overflow" << endl;
            nets[i].corner.push_back(make_pair(999, 999));
            for (int j = 0; j < 3; ++j) {
                cout << "(" << nets[i].corner[j].first << ", " << nets[i].corner[j].second << ") ";
            }
        }
    }*/
    /*for (int i = 0; i < GCells.size(); ++i) {
        for (int j = 0; j < GCells[i].size(); ++j) {
            cout << "(" << j << ", " << i <<") ";
            cout << GCells[i][j].costG << " " << GCells[i][j].costH << " " << GCells[i][j].costF << " ";
            if (GCells[i][j].parent != NULL)
                cout << "(" << GCells[i][j].parent->coord.first << ", " << GCells[i][j].parent->coord.second << ")";
            cout << endl;
        }
    }*/
    /*for (int i = 0; i < GCells.size(); ++i) {
        for (int j = 0; j < GCells[i].size(); ++j) {
            cout << "(" << GCells[i][j].coord.first << ", " << GCells[i][j].coord.second <<") : ";
            for (int k = 0; k < 4; ++k)
                cout << GCells[i][j].adjList[k].second << " ";
            cout << endl;
            for (int k = 0; k < 4; ++k) {
                cout << k << ": ";
                if (GCells[i][j].adjList[k].first != NULL) {
                    cout << GCells[i][j].adjList[k].first->coord.first << " " << GCells[i][j].adjList[k].first->coord.second << ": ";
                    for (int g = 0; g < 4; ++g)
                        cout << GCells[i][j].adjList[k].first->adjList[g].second << " ";
                }
                cout << endl;
            }
            cout << endl;
        }
    }*/           
/*--------------output-----------------------------------------------------------------------------------------------------------*/
    for (int i = 0; i < num_net; ++i) {
        result_file << nets[i].name << " " << nets[i].id << "\n";
        for (int j = nets[i].corner.size() - 1; j > 0; --j) {
            result_file << "(" << nets[i].corner[j].first << ", " << nets[i].corner[j].second << ", 1)-";
            result_file << "(" << nets[i].corner[j-1].first << ", " << nets[i].corner[j-1].second << ", 1)" << "\n";
        }
        result_file << "!" << "\n";
    }
    result_file.close();
/*-------------------------------------------------------------------------------------------------------------------------------*/
    for (int x = 0; x < grid.first - 1; ++x) {
        for (int y = grid.second - 1; y > 0; --y) {
            double usage = (double)(vertCapacity - GCells[y][x].adjList[2].second) / (double)vertCapacity;
            draw_file << usage << " ";
        }
        draw_file << "\n";
        for (int y = grid.second - 1; y >= 0; --y) {
            double usage = (double)(horizCapacity - GCells[y][x].adjList[1].second) / (double)horizCapacity;
            draw_file << usage << " ";
        }
        draw_file << "\n";
    }
    for (int y = grid.second - 1; y > 0; --y) {
        double usage = (double)(vertCapacity - GCells[y][grid.first-1].adjList[2].second) / (double)vertCapacity;
        draw_file << usage << " ";
    }
    draw_file.close();
    return 0;
}
