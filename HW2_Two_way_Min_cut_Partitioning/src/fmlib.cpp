#include <string>
#include <vector>
#include <list>
#include <iterator>
#include "fmlib.h"
using namespace std;

Net::Net(string name) {
    this->name = name;
    this->size = 0;
    this->cntBucket[0] = 0;
    this->cntBucket[1] = 0;
    this->lock[0] = 0;
    this->lock[1] = 0;
}
Cell::Cell(string name, double size) {
    this->name = name;
    this->size = size;
    this->partition = 1;
    this->gain = 0;
    this->lock = 0;
    this->put = 0;
}
Bucket::Bucket(string name, int Pmax) {
    this->name = name;
    this->cnt = 0;
    this->size = 0;
    this->maxIndex = 0;
    this->bucketList.resize(2*Pmax+1);
}