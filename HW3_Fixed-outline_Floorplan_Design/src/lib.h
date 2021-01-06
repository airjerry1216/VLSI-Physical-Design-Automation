#ifndef LIB_H
#define LIB_H
#include <string>
#include <vector>
#include <list>
#include <iterator>
using namespace std;

class HRB {
public:
    string name;
    int w;
    int h;
    bool rotation;
    pair<int, int> coord;
    HRB *parent;
    HRB *left;
    HRB *right;
    HRB();
};
class TERM {
public:
    string name;
    pair<int, int> coord;
    TERM();
};
class Net {
public:
    int degree;
    vector<string> pins;
    Net();
};
class Contour {
public:
    HRB *hrb;
    Contour *prev;
    Contour *next;
    Contour(HRB*);
};
#endif