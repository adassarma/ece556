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
    unordered_set<int> connections;
    unordered_map<int,vector<vector<int>>> net;
};

unordered_map<int,vector<vector<int>>> unique_nets;
unordered_map<int,int> partitiontable;
map<int,Cell*>CurrentCellLookUp;
map<int,unordered_set<int>,greater<int>>gaintable;
unordered_set<int> locked;
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

void writetoFile(char* file);