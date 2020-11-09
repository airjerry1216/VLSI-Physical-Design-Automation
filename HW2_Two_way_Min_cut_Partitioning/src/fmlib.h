#ifndef FMLIB_H
#define FMLIB_H
#include <string>
#include <vector>
#include <list>
#include <iterator>
#include <set>
using namespace std;

class Net {
public:
    string name;
    double size;
    int cntBucket[2];
    bool lock[2];
    vector<string> cells;
    Net(string);
};
class Cell {
public:
    string name;
    double size;
    bool partition;
    int gain;
    bool lock;
    bool put;
    vector<string> nets; //cell's connected nets
    list<Cell*>::iterator it;
    Cell(string, double);
};
class Bucket {
public:
    string name;
    int cnt;
    double size;
    int maxIndex;
    vector<list<Cell*> > bucketList;
    set<string> bucketSet;
    Bucket(string, int);
};

#endif