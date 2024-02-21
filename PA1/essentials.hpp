#pragma once
#include<string>
#include<unordered_set>
#include<unordered_map>
#include<vector>
using namespace std;
struct Cell
{

    Cell(int number,int partition)
    {
        this->number = number;
        this->partition = partition;
    }
    Cell(const Cell&) = delete;
    Cell(Cell&&) = delete;
    Cell& operator=(const Cell&) = delete;
    Cell&& operator=(Cell&&) = delete;

    int number;
    int partition;
    int gain;
    int fs;
    int te;
    vector<int> connections;
};

vector<vector<int>> unique_nets;
vector<int> netpart0;
vector<int> netpart1;
unordered_map<int,int> partitiontable;
map<int,Cell*>CurrentCellLookUp;
unordered_set<int> visitednode;
unordered_set<int> locallyvisited;
map<int,unordered_set<int>,greater<int>>gaintable;
unordered_set<int> locked;
vector<int> pendingpartitionswap;
double lowerbound;
double upperbound;
double partition0size;
double partition1size;
double bestpartition0size;
double bestpartition1size;
int currentcutsize;
int bestcutsize;
double currentareadiff;
double bestareadiff;
int iteration = 0;
double r = 0.0;