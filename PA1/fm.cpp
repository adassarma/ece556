#include<iostream>
#include<fstream>
#include<string>
#include<sstream>
#include<map>
#include<vector>
#include "essentials.hpp"
using namespace std;

void copy_map_best_current()
{
    for(auto&[nodenum,node]:BestCellLookUp)
    {
        Cell *newcell = CurrentCellLookUp[nodenum];
        newcell->exterior_connections = node->exterior_connections;
        newcell->partition = node->partition;
        newcell->gain = node->gain;
        newcell->interior_connections = node->interior_connections; 
        newcell->net = node->net;
    }
}

void copy_map_current_best()
{
    for(auto&[nodenum,node]:CurrentCellLookUp)
    {
        Cell *newcell;
        if(BestCellLookUp.find(nodenum)==BestCellLookUp.end())
        newcell = new Cell(nodenum,node->partition);
        else
        newcell = BestCellLookUp[nodenum];
        newcell->exterior_connections = node->exterior_connections;
        newcell->gain = node->gain;
        newcell->interior_connections = node->interior_connections; 
        newcell->net = node->net;
        newcell->partition = node->partition;
        BestCellLookUp[nodenum] = newcell;
    }
}


void readfromFile(char *file)
{
    int flip = 1;
    string line;
    bool first = true;
    ifstream inputFile;
    inputFile.open(file,ios::in);
    if (inputFile.is_open())
    {
        while (getline(inputFile,line))
            {
                if(first)
                {
                    r = stof(line);
                    first = !first;
                }
                else
                {
                    istringstream ss(line);
                    string temp;
                    vector<int> net;
                    bool flag = true;
                    while(getline(ss,temp,' '))
                    {
                        char ch = temp[0];
                        if(temp[0]=='c')
                        {
                            temp.erase(temp.begin());
                            int cellnum = stoi(temp);
                            Cell *cell;
                            if(CurrentCellLookUp.find(cellnum)==CurrentCellLookUp.end())
                            {
                            flip = 1-flip;    
                            cell = new Cell(cellnum,flip);
                            CurrentCellLookUp[cellnum] = cell;
                            if(flip==0)
                            ++partition0size;
                            else
                            ++partition1size;
                            }
                            else
                            cell = CurrentCellLookUp[cellnum];
                            for(auto &net_elem:net)
                                {
                                    Cell *placeholder = CurrentCellLookUp[net_elem];
                                    if(placeholder->partition==cell->partition)
                                    {
                                        placeholder->interior_connections.insert(cellnum);
                                        cell->interior_connections.insert(net_elem);
                                    }    
                                    else
                                    {
                                    placeholder->exterior_connections.insert(cellnum);
                                    cell->exterior_connections.insert(net_elem);
                                    if(flag)
                                    {
                                        flag = false;
                                        ++currentcutsize;
                                    }
                                    }
                                }
                            net.push_back(cellnum);
                        }
                    }

                    for(auto &net_elem:net)
                        CurrentCellLookUp[net_elem]->net.push_back(net);
                }
            }
    inputFile.close();
    }

  else cout << "Unable to open file";
}

void computegain(int nodenum)
{
        if(locked.find(nodenum)!=locked.end())
        return;
        
        int fs = 0,te = 0;
        Cell *node = CurrentCellLookUp[nodenum];
        
        for(auto &net_elem:node->net)
        {
            int internal_count = 0;
            int external_count = 0;
            for(auto& net_num:net_elem)
            {
                if(net_num==nodenum)
                continue;

                if(node->interior_connections.find(net_num)!=node->interior_connections.end())
                {
                    ++internal_count;
                    continue;
                }

                // for(auto &same_partition_neighbour:node->interior_connections)
                // {
                //     if(CurrentCellLookUp[same_partition_neighbour]->exterior_connections.find(net_num)!=CurrentCellLookUp[same_partition_neighbour]->exterior_connections.end())
                //     {
                //         --external_count;
                //         break;
                //     }
                // }

                ++external_count;

            }

            if(internal_count==net_elem.size()-1)
            ++te;
            else if(external_count==net_elem.size()-1)
            ++fs;
        }

        if(gaintable[node->gain].find(nodenum)!=gaintable[node->gain].end())
        gaintable[node->gain].erase(nodenum);
        if(gaintable[node->gain].empty())
             gaintable.erase(node->gain);

        node->gain = fs-te;
        gaintable[node->gain].insert(nodenum);

}

void computegainfromlookup()
{
    for(auto &[nodenum,node]:CurrentCellLookUp)
        computegain(nodenum);
}

bool isoktomove(int nodenum)
{
    if(CurrentCellLookUp[nodenum]->partition==0)
    {
        if(partition0size-1<lowerbound||partition1size+1>upperbound)
        return false;
    }
    else{

        if(partition1size-1<lowerbound||partition0size+1>upperbound)
        return false;
    }
    
    return true;
}

bool performswap(int nodenum)
{
     
    Cell* maxnode = CurrentCellLookUp[nodenum];
    currentcutsize-=maxnode->gain;

    for(auto &int_nodes:maxnode->interior_connections)
    {
        CurrentCellLookUp[int_nodes]->interior_connections.erase(nodenum);
        CurrentCellLookUp[int_nodes]->exterior_connections.insert(nodenum);
        computegain(int_nodes);
    }

    for(auto &ext_nodes:maxnode->exterior_connections)
    {
        CurrentCellLookUp[ext_nodes]->exterior_connections.erase(nodenum);
        CurrentCellLookUp[ext_nodes]->interior_connections.insert(nodenum);
        computegain(ext_nodes);
    }

    if(maxnode->partition==0)
    {
        maxnode->partition = 1;
        --partition0size,++partition1size;
    }
    else
    {
        maxnode->partition = 0;
        --partition1size,++partition0size;
    }
    currentareadiff = abs(partition0size-partition1size);
    maxnode->exterior_connections = exchange(maxnode->interior_connections,maxnode->exterior_connections);

    if(currentcutsize<=bestcutsize)
    {
        if(currentcutsize<bestcutsize)
        {
        bestcutsize = currentcutsize;
        bestareadiff = currentareadiff;
        bestpartition0size = partition0size;
        bestpartition1size = partition1size;
        return true;
        }
        else if(currentareadiff<bestareadiff)
        {
        bestareadiff = currentareadiff;
        bestpartition0size = partition0size;
        bestpartition1size = partition1size;
        return true;
        }
    }

    return false;
}

bool fm()
{
    copy_map_best_current();
    locked.clear();
    computegainfromlookup();
    bool stagnate = true;

        for(int i =0;i<CurrentCellLookUp.size();++i)
        {
            int maxgainnodenum;
            for(auto&[currentgain,nodenum]:gaintable)
            {
                if(isoktomove(*nodenum.begin()))
                {
                    maxgainnodenum = *nodenum.begin();
                    break;
                }
            }

            gaintable[CurrentCellLookUp[maxgainnodenum]->gain].erase(maxgainnodenum);
            if(gaintable[CurrentCellLookUp[maxgainnodenum]->gain].empty())
            gaintable.erase(CurrentCellLookUp[maxgainnodenum]->gain);

            locked.insert(maxgainnodenum);

            if(performswap(maxgainnodenum))
            {
                stagnate=false;
                copy_map_current_best();
            }

        }

    return stagnate;
}

void writetoFile(char* file)
{
        
        vector<int>G1;
        vector<int>G2;
        for(auto &[nodenum,node]:BestCellLookUp)
        {
            if(node->partition==0)
            G1.push_back(nodenum);
            else
            G2.push_back(nodenum);
        }
        
        
        ofstream outputFile;
        outputFile.open(file); 
        outputFile<<"Cutsize = "<<bestcutsize<<"\n";
        outputFile<<"G1 "<<bestpartition0size<<"\n";
        for(auto&g1_elem:G1)
        outputFile<<"c"<<g1_elem<<" ";
        outputFile<<";"<<"\n"; 
        outputFile<<"G2 "<<bestpartition1size<<"\n";
        for(auto&g2_elem:G2)
        outputFile<<"c"<<g2_elem<<" ";
        outputFile<<";";
        outputFile.close();
}

int main(int argc,char **argv)
{
        readfromFile(argv[1]);
        lowerbound = (float)CurrentCellLookUp.size()*(1-r)/2;
        upperbound = (float)CurrentCellLookUp.size()*(1+r)/2;
        currentareadiff = abs(partition0size-partition1size);
        bestareadiff = currentareadiff;
        bestcutsize = currentcutsize;
        copy_map_current_best();
        while(!fm());

        writetoFile(argv[2]);

}