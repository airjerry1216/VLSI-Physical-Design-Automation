#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <iterator>
#include "lib.h"
using namespace std;

HRB::HRB() {
    this->w = 0;
    this->h = 0;
    this->coord.first = 0;
    this->coord.second = 0;
    this->rotation = 0;
}
TERM::TERM() {
    this->coord.first = 0;
    this->coord.second = 0;
}
Net::Net() {
    this->degree = 0;
}
Contour::Contour(HRB *hrb) {
    this->hrb = hrb;
}