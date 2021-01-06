#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <string>
#include <vector>
#include <iterator>
#include <ctime>
#include "lib.h"
using namespace std;
//../bin/hw3 ../testcase/n100.hardblocks ../testcase/n100.nets ../testcase/n100.pl ../output/n100.floorplan 0.1 ../output/n100.draw
//../bin/hw3 ../testcase/n200.hardblocks ../testcase/n200.nets ../testcase/n200.pl ../output/n200.floorplan 0.1 ../output/n200.draw
//../bin/hw3 ../testcase/n300.hardblocks ../testcase/n300.nets ../testcase/n300.pl ../output/n300.floorplan 0.1 ../output/n300.draw
Contour *contourHead = new Contour(NULL);
bool hrbCmp(const HRB &a, const HRB &b) 
{
    if (a.h == b.h)
        return a.w > b.w;
    else
        return a.h > b.h; 
}
void swap(int &a, int &b)
{
    int tmp = a;
    a = b;
    b = tmp;
    return;
}
void rotate(HRB &hrb)
{
    swap(hrb.w, hrb.h);
    hrb.rotation = !hrb.rotation;
    return;
}
void printContour(Contour *ptr)
{
    bool first = 1;
    int size = 0;
    while (ptr != NULL) {
        ++size;
        if (first == 0) {
            cout << ptr->hrb->name << " ";
        }
        first = 0;
        ptr = ptr->next;
    }
    cout << endl;
    cout << "contour.size(): " << size << endl;
}
void maintainContour(HRB *hrb, Contour *ptr, string type)
{
    cout << "maintain: " << hrb->name << endl;
    if (type == "left")
        hrb->coord.first = hrb->parent->coord.first + hrb->parent->w;
    else if (type == "right")
        hrb->coord.first = hrb->parent->coord.first;
    Contour *node = new Contour(hrb);
    if (ptr->next == NULL) { //當前hrb要擺的contour為空
        cout << "--back--" << endl;
        ptr->next = node;
        node->prev = ptr;
    }
    else {
        //cout << "next contour " << ptr->next->hrb->name << endl;
        cout << "--front--" << endl;
        node->next = ptr->next;
        node->next->prev = node;
        ptr->next = node;
        node->prev = ptr;
        node->hrb->coord.second = (node->next->hrb->coord.second) + (node->next->hrb->h);
        while ((node->hrb->coord.first) + node->hrb->w >= (node->next->hrb->coord.first) + (node->next->hrb->w)) {   
            if (node->hrb->coord.second < (node->next->hrb->coord.second) + (node->next->hrb->h))
                node->hrb->coord.second = (node->next->hrb->coord.second) + (node->next->hrb->h);
            Contour *tmp = node->next;
            if (tmp->next != NULL) {
                node->next = tmp->next;
                tmp->next->prev = node;
                delete tmp;
            }
            else {
                node->next = NULL;
                delete tmp;
                break;
            }
        }
    }
    return;
}
void clearContour(Contour *ptr)
{
    while (ptr->next != NULL) {
        Contour *tmp = ptr->next;
        if (tmp->next != NULL) {
            ptr->next = tmp->next;
            tmp->next->prev = ptr;
            //cout << tmp->hrb->name << endl;
            delete tmp;
        }
        else {
            ptr->next = NULL;
            //cout << tmp->hrb->name << endl;
            delete tmp;
        }
    }
    return;
}
void dfs(HRB *hrb, Contour *ptr, string type)
{
    if (hrb == NULL)
        return;
    //cout << "dfs " << hrb->name << endl;
    maintainContour(hrb, ptr, type);
    //printContour(contourHead);
    //cout << &contourHead << endl;
    //cout << "success" << endl;
    ptr = ptr->next;
    dfs(hrb->left, ptr, "left");
    ptr = ptr->prev;
    //if (hrb->right != NULL)
      //  printContour(contourHead);
    dfs(hrb->right, ptr, "right");
    return;
}
int main(int argc, char* argv[])
{
    ifstream hb_File(argv[1]);
    ifstream nets_File(argv[2]);
    ifstream pl_File(argv[3]);
    ofstream outFile(argv[4]);
    ofstream drawFile(argv[6]);
    int num_hbFile[2] = {0};
    int num_netsFile[2] = {0};
    double dead_space_ratio = 0, total_block_area = 0;
    sscanf(argv[5], "%lf", &dead_space_ratio);
    int flpWidth = 0, flpHeight = 0;
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
                rotate(hrbs[hrbIndex]);
            total_block_area += hrbs[hrbIndex].w * hrbs[hrbIndex].h;
            ++hrbIndex;
            ss.str("");
            ss.clear();
        }
        else if (line[0] == 'p')
            break;
    }
    flpWidth = flpHeight = sqrt(total_block_area * (1 + dead_space_ratio));
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
    cout << "total_block_area: " << total_block_area << endl;
    cout << "dead space ratio: " << dead_space_ratio << endl;
    cout << "floorplan width & height: " << flpWidth << " " << flpHeight << endl;
/*------------B* tree-----------------------------------------------------------------------------------------*/
    sort(hrbs.begin(), hrbs.end(), hrbCmp);
    Contour *ptr = contourHead;
    HRB *hrbLead = &hrbs[0];
    for (int i = 1; i < hrbs.size(); ++i) {
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
    dfs(&hrbs[0], contourHead, "root");
    printContour(contourHead);
    clearContour(contourHead);
    cout << "-----------clearContour------------" << endl;
    printContour(contourHead);
    dfs(&hrbs[0], contourHead, "root");
    //printContour(contourHead);
/*------------draw--------------------------------------------------------------------------------------------*/
    drawFile << flpWidth << "\n";
    for (auto &hrb : hrbs) {
        drawFile << hrb.name << " " << hrb.coord.first << " " << hrb.coord.second << " " << hrb.w << " " << hrb.h << "\n";
    }
/*------------------------------------------------------------------------------------------------------------*/

    /*for (auto &hrb : hrbs) {
        cout << hrb.name << " " << hrb.w << " " << hrb.h << " " << "(" << hrb.coord.first << ", " << hrb.coord.second << ") " << hrb.rotation << endl;
    }*/
    //printContour(contourHead);
    return 0;
}