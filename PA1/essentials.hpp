#pragma once
#include<string>
#include<unordered_set>
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
    unordered_set<int> interior_connections;
    unordered_set<int> exterior_connections;
    vector<vector<int>> net;
};

map<int,Cell*>CurrentCellLookUp;
map<int,Cell*>BestCellLookUp;
map<int,unordered_set<int>,greater<int>>gaintable;
unordered_set<int> locked;
float lowerbound;
float upperbound;
float partition0size;
float partition1size;
float bestpartition0size;
float bestpartition1size;
int currentcutsize;
int bestcutsize;
float currentareadiff;
float bestareadiff;
float r = 0.0;