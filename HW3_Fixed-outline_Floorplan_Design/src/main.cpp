#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <utility>
#include <string>
#include <vector>
#include <iterator>
#include <ctime>
#include <cstring>
#include <climits>
#include <cmath>
#include "lib.h"
using namespace std;
//../bin/hw3 ../testcase/n100.hardblocks ../testcase/n100.nets ../testcase/n100.pl ../output/n100.floorplan 0.1 ../output/n100.draw
//../bin/hw3 ../testcase/n200.hardblocks ../testcase/n200.nets ../testcase/n200.pl ../output/n200.floorplan 0.1 ../output/n200.draw
//../bin/hw3 ../testcase/n300.hardblocks ../testcase/n300.nets ../testcase/n300.pl ../output/n300.floorplan 0.1 ../output/n300.draw
bool hrbCmp(const HRB &a, const HRB &b) 
{
    if (a.h == b.h)
        return a.w > b.w;
    else
        return a.h > b.h; 
}
void saRotate(HRB *hrb)
{
    hrb->rotation = !hrb->rotation;
    swap(hrb->w, hrb->h);
    return;
}
void saSwap(HRB *hrb1, HRB *hrb2)
{
    swap(hrb1->name, hrb2->name);
    swap(hrb1->w, hrb2->w);
    swap(hrb1->h, hrb2->h);
    swap(hrb1->rotation, hrb2->rotation);
    return; 
}
bool saAccept(double &randnum, double &T, int &cost)
{
    return exp(-cost/T) > randnum;
}
void clearContour(int *contour, int &flpWidth)
{
    memset(contour, 0, sizeof(int) * flpWidth);
}
void maintainContour(HRB *hrb, int *contour, string type, int &flpWidth)
{
    if (type == "left")
        hrb->coord.first = hrb->parent->coord.first + hrb->parent->w;
    else if (type == "right")
        hrb->coord.first = hrb->parent->coord.first;
    hrb->coord.second = 0;
    int start = hrb->coord.first, end = hrb->coord.first + hrb->w;
    for (int i = start; i < end; ++i) {
        if (i >= flpWidth)
            break;
        if (hrb->coord.second < contour[i])
            hrb->coord.second = contour[i];
    }
    int upper = hrb->coord.second + hrb->h;
    for (int i = start; i < end; ++i) {
        if (i >= flpWidth)
            break;
        contour[i] = upper;
    }
    return;
}
void dfsBStarTree(HRB *hrb, int *contour, string type, int &flpWidth)
{
    if (hrb == NULL)
        return;
    maintainContour(hrb, contour, type, flpWidth);
    dfsBStarTree(hrb->left, contour, "left", flpWidth);
    dfsBStarTree(hrb->right, contour, "right", flpWidth);
    return;
}
int calculateWireLength(vector<Net> &nets, vector<HRB> &hrbs, vector<int> &hrbToIndex, vector<TERM> & terms)
{
    int wirelength = 0;
    for (auto &n : nets) {
        int xLeft = INT_MAX, xRight = 0, yDown = INT_MAX, yUp = 0;
        for (auto &p : n.pins) {
            pair<int, int> coord;
            if (p[0] == 's') {
                HRB *tmp = &hrbs[hrbToIndex[stoi(p.substr(2))]];
                coord = make_pair((2 * tmp->coord.first + tmp->w) / 2, (2 * tmp->coord.second + tmp->h) / 2);
            }
            else {
                TERM *tmp = &terms[stoi(p.substr(1))-1];
                coord = tmp->coord;
            }
            if (coord.first < xLeft)
                xLeft = coord.first;
            if (coord.first > xRight)
                xRight = coord.first;
            if (coord.second < yDown)
                yDown = coord.second;
            if (coord.second > yUp)
                yUp = coord.second;
        }
        wirelength += (xRight - xLeft) + (yUp - yDown);
    }
    return wirelength;
}
bool fixedOutlineCheck(vector<HRB> &hrbs, int &flpWidth)
{
    for (auto &hrb : hrbs)
        if (hrb.coord.first + hrb.w > flpWidth || hrb.coord.second + hrb.h > flpWidth)
            return 0;
    return 1;
}
int main(int argc, char* argv[])
{
    ifstream hb_File(argv[1]);
    ifstream nets_File(argv[2]);
    ifstream pl_File(argv[3]);
    ofstream out_File(argv[4]);
    ofstream draw_File(argv[6]);
    int num_hbFile[2] = {0};
    int num_netsFile[2] = {0};
    double dead_space_ratio = 0, total_block_area = 0;
    sscanf(argv[5], "%lf", &dead_space_ratio);
    int flpWidth = 0, flpHeight = 0, wirelength = 0;
    stringstream ss;
    string line, tmp;
/*------------read hardblocks---------------------------------------------------------------------------------*/
    for (int i = 0; i < 2; ++i) {
        getline(hb_File, line);
        ss << line;
        ss >> tmp >> tmp >> num_hbFile[i];
        ss.str("");
        ss.clear();
    }
    vector<HRB> hrbs(num_hbFile[0]);
    vector<int> hrbToIndex(num_hbFile[0]);
    vector<TERM> terms(num_hbFile[1]);
    int hrbIndex = 0;
    while (getline(hb_File, line)) {
        if (line[0] == 's') {
            ss << line;
            ss >> hrbs[hrbIndex].name;
            ss >> tmp >> tmp >> tmp >> tmp >> tmp >> tmp >> tmp;
            hrbs[hrbIndex].w = stoi(tmp.substr(1, tmp.size()-2)); // (43,
            ss >> tmp;
            hrbs[hrbIndex].h = stoi(tmp.substr(0, tmp.size()-1)); // 33)
            if (hrbs[hrbIndex].w > hrbs[hrbIndex].h)
                saRotate(&hrbs[hrbIndex]);
            total_block_area += hrbs[hrbIndex].w * hrbs[hrbIndex].h;
            ++hrbIndex;
            ss.str("");
            ss.clear();
        }
        else if (line[0] == 'p')
            break;
    }
    hb_File.close();
    flpWidth = flpHeight = sqrt(total_block_area * (1 + dead_space_ratio));
    int contour[flpWidth];
    clearContour(contour, flpWidth);
/*------------read nets---------------------------------------------------------------------------------------*/
    for (int i = 0; i < 2; ++i) {
        getline(nets_File, line);
        ss << line;
        ss >> tmp >> tmp >> num_netsFile[i];
        ss.str("");
        ss.clear();
    }
    vector<Net> nets(num_netsFile[0]);
    int netIndex = 0;
    while(getline(nets_File, line)) {
        ss << line;
        ss >> tmp >> tmp >> nets[netIndex].degree;
        for (int i = 0; i < nets[netIndex].degree; ++i) {
            getline(nets_File, line);
            nets[netIndex].pins.push_back(line);
        }
        ++netIndex;
        ss.str("");
        ss.clear();
    }
    nets_File.close();
/*------------read pl-----------------------------------------------------------------------------------------*/
    int termIndex = 0;
    while (getline(pl_File, line)) {
        ss << line;
        ss >> terms[termIndex].name;
        ss >> terms[termIndex].coord.first;
        ss >> terms[termIndex].coord.second;
        ++termIndex;
        ss.str("");
        ss.clear();
    }
    pl_File.close();
    cout << "total_block_area: " << total_block_area << endl;
    cout << "dead space ratio: " << dead_space_ratio << endl;
    cout << "floorplan width & height: " << flpWidth << endl;
/*------------B* tree-----------------------------------------------------------------------------------------*/
    sort(hrbs.begin(), hrbs.end(), hrbCmp);
    HRB *hrbLead = &hrbs[0];
    for (int i = 0; i < hrbs.size(); ++i) {
        if (i != 0) {
            if (hrbs[i-1].coord.first + hrbs[i-1].w + hrbs[i].w <= flpWidth) {         
                hrbs[i].parent = &hrbs[i-1];
                hrbs[i].coord.first = hrbs[i].parent->coord.first + hrbs[i].parent->w;
                hrbs[i-1].left = &hrbs[i];
            }
            else {
                hrbs[i].parent = hrbLead;
                hrbs[i].coord.first = hrbs[i].parent->coord.first;
                hrbLead->right = &hrbs[i];
                hrbLead = &hrbs[i];
            }
        }
        hrbToIndex[stoi(hrbs[i].name.substr(2))] = i;
    }
/*------------Fixed Outline Constraint------------------------------------------------------------------------*/
    cout << "--------------------Fixed Outline Constraint--------------------" << endl;
    srand(1216);
    int stepOfFixOutline = 0;
    dfsBStarTree(&hrbs[0], contour, "root", flpWidth);
    if (!fixedOutlineCheck(hrbs, flpWidth)) {
        while(1) {
            double randnum = (double) rand() / (RAND_MAX + 1.0);
            int target = 0, target1 = 0, target2 = 0;
            if (randnum >= 0) {
                target = rand() % ((num_hbFile[0] - 1) - 0 + 1) + 0;
                saRotate(&hrbs[target]);
            }
            else {
                target1 = rand() % ((num_hbFile[0] - 1) - 1 + 1) + 1;
                target2 = rand() % ((num_hbFile[0] - 1) - 1 + 1) + 1;
                saSwap(&hrbs[target1], &hrbs[target2]);
                swap(hrbToIndex[stoi(hrbs[target1].name.substr(2))], hrbToIndex[stoi(hrbs[target2].name.substr(2))]);
            }
            dfsBStarTree(&hrbs[0], contour, "root", flpWidth);
            ++stepOfFixOutline;
            if (fixedOutlineCheck(hrbs, flpWidth))
                break;
            clearContour(contour, flpWidth);
        }
    }
    cout << "FixedOutline step: " << stepOfFixOutline << endl;
    wirelength = calculateWireLength(nets, hrbs, hrbToIndex, terms);
    cout << "init wirelength: " << wirelength << endl;
    clearContour(contour, flpWidth);
/*------------Simulated Annealing-----------------------------------------------------------------------------*/
    cout << "--------------------Simulated Annealing--------------------" << endl;
    int attempt = 0, stepOfWireLength = 0, outOfBound = 0, accept = 0, unaccept = 0, better = 0, cnt = 0;
    double T = 10000, endingT = 1;
    int P = 1, cost = 0, minWireLength = wirelength;
    vector<HRB> hrbsBest = hrbs;
    vector<int> hrbToIndexBest = hrbToIndex;
    while (T > endingT) {
        for (int i = 0; i < P; ++i) {
            bool saAction = 0;
            int target = 0, target1 = 0, target2 = 0;
            double randnum = 0;
            while (1) {
                double saActionSelect = (double) rand() / (RAND_MAX + 1.0);
                randnum = saActionSelect;
                if (saActionSelect >= 0.7) {
                    saAction = 0;
                    target = rand() % ((num_hbFile[0] - 1) - 0 + 1) + 0;
                    saRotate(&hrbs[target]);
                }
                else {
                    saAction = 1;
                    target1 = rand() % ((num_hbFile[0] - 1) - 1 + 1) + 1;
                    target2 = rand() % ((num_hbFile[0] - 1) - 1 + 1) + 1;
                    if (target1 == target2)
                        continue;
                    saSwap(&hrbs[target1], &hrbs[target2]);
                    swap(hrbToIndex[stoi(hrbs[target1].name.substr(2))], hrbToIndex[stoi(hrbs[target2].name.substr(2))]);
                }
                ++attempt;
                dfsBStarTree(&hrbs[0], contour, "root", flpWidth);
                if (!fixedOutlineCheck(hrbs, flpWidth)) {
                    if (!saAction)
                        saRotate(&hrbs[target]);
                    else {
                        saSwap(&hrbs[target1], &hrbs[target2]);
                        swap(hrbToIndex[stoi(hrbs[target1].name.substr(2))], hrbToIndex[stoi(hrbs[target2].name.substr(2))]);
                    }
                    clearContour(contour, flpWidth);
                    ++outOfBound;
                }
                else
                    break;
            }
            int newWireLength = calculateWireLength(nets, hrbs, hrbToIndex, terms);
            cost = newWireLength - wirelength;
            if (cost > 0) {
                randnum = (double) rand() / (RAND_MAX + 1.0);
                if (saAccept(randnum, T, cost)) {
                    wirelength = newWireLength;
                    ++accept;
                }
                else {
                    if (!saAction)
                        saRotate(&hrbs[target]);
                    else {
                        saSwap(&hrbs[target1], &hrbs[target2]);
                        swap(hrbToIndex[stoi(hrbs[target1].name.substr(2))], hrbToIndex[stoi(hrbs[target2].name.substr(2))]);
                    }
                    ++unaccept;
                }
            }
            else {
                wirelength = newWireLength;
                if (minWireLength > wirelength) {
                    minWireLength = wirelength;
                    hrbsBest = hrbs;
                    hrbToIndexBest = hrbToIndex;
                }
                ++better;
            }
            clearContour(contour, flpWidth);
            ++stepOfWireLength;
        }   
        T *= 0.9999;
    }
    cout << "attempt: " << attempt << endl;
    cout << "outOfBound: " << outOfBound << endl;
    cout << "stepOfWireLength: " << stepOfWireLength << endl;
    cout << "accept: " << accept << endl;
    cout << "unaccept: " << unaccept << endl;
    cout << "better: " << better << endl;
    cout << "minWireLength: " << minWireLength << endl;
    cout << "finalwirelength: " << wirelength << endl;
/*------------output------------------------------------------------------------------------------------------*/
    out_File << "Wirelength" << " " << minWireLength << "\n";
    out_File << "Blocks" << "\n";
    for (int i = 0; i < hrbsBest.size(); ++i) {
        out_File << hrbsBest[hrbToIndexBest[i]].name << " ";
        out_File << hrbsBest[hrbToIndexBest[i]].coord.first << " " << hrbsBest[hrbToIndexBest[i]].coord.second << " ";
        out_File << hrbsBest[hrbToIndexBest[i]].rotation << "\n";
    }
    out_File.close();
/*------------draw--------------------------------------------------------------------------------------------*/
    draw_File << flpWidth << "\n";
    for (auto &hrb : hrbsBest)
        draw_File << hrb.name << " " << hrb.coord.first << " " << hrb.coord.second << " " << hrb.w << " " << hrb.h << "\n";
    draw_File.close();
/*------------------------------------------------------------------------------------------------------------*/
    return 0;
}
