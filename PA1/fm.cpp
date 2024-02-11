#include<iostream>
#include<fstream>
#include<string>
#include<sstream>
#include<map>
#include<vector>
#include "essentials.hpp"
using namespace std;

void construct_current_map()
{
    for(auto&[nodenum,partitionnum]:partitiontable)
    {
        CurrentCellLookUp[nodenum]->partition = partitionnum;
        CurrentCellLookUp[nodenum]->connections.clear();
    } 
        for(auto&[nodenum,partitionum]:partitiontable)
        {   
        auto nets = unique_nets[nodenum];
        for(auto&net:nets)
        {

         for(auto &net_elem:net)
        {
            if(net_elem==nodenum) continue;
            Cell *placeholder = CurrentCellLookUp[net_elem];
            placeholder->connections.insert(nodenum);
            CurrentCellLookUp[nodenum]->connections.insert(net_elem);
        }

        }

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
                    r = stod(line);
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
                            partitiontable[cellnum] = flip;
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
                                    placeholder->connections.insert(cellnum);
                                    cell->connections.insert(net_elem);
                                    if(flag&&cell->partition!=placeholder->partition)
                                    {
                                        flag = false;
                                        ++currentcutsize;
                                    }
                                }
                            net.push_back(cellnum);
                        }
                    }

                    for(auto &net_elem:net)
                        unique_nets[net_elem].push_back(net);

                    for(auto &net_elem:net)
                    {
                        for(auto&net_key:net)
                        {
                            if(net_elem==net_key) continue;
                        CurrentCellLookUp[net_elem]->net[net_key].push_back(net);
                        }
                    }    
                }
            }
    inputFile.close();
    }

  else cout << "Unable to open file";
}

void computegain()
{
        
            for(auto &[nodenum,net]:unique_nets)
            {       
            
            auto node = CurrentCellLookUp[nodenum];
            node->fs = 0;
            node->te = 0;
            for(auto& net_num_vec:net)
            {
                int internal_count = 0;
                int external_count = 0;
                for(auto&net_num:net_num_vec)
                {
                if(net_num==nodenum)
                continue;

                if(node->partition==CurrentCellLookUp[net_num]->partition)
                {
                    ++internal_count;
                    continue;
                }

                ++external_count;
                }

            if(internal_count==net_num_vec.size()-1)
            ++node->te;
            else if(external_count==net_num_vec.size()-1)
            ++node->fs;
            } 

        node->gain = node->fs-node->te;
        gaintable[node->gain].insert(nodenum);

         }  

}



void computeparticulargain(int nodenum)
{
              
            
            auto net = unique_nets[nodenum];
            auto node = CurrentCellLookUp[nodenum];
            node->fs = 0;
            node->te = 0;
            for(auto& net_num_vec:net)
            {
                int internal_count = 0;
                int external_count = 0;
                for(auto&net_num:net_num_vec)
                {
                if(net_num==nodenum)
                continue;

                if(node->partition==CurrentCellLookUp[net_num]->partition)
                {
                    ++internal_count;
                    continue;
                }

                ++external_count;
                }

            if(internal_count==net_num_vec.size()-1)
            ++node->te;
            else if(external_count==net_num_vec.size()-1)
            ++node->fs;
            }

    if(gaintable[node->gain].find(nodenum)!=gaintable[node->gain].end())
            gaintable[node->gain].erase(nodenum);

    if(gaintable[node->gain].empty())
            gaintable.erase(node->gain);     

        node->gain = node->fs-node->te;
        gaintable[node->gain].insert(nodenum);


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

    for(auto &nodes:maxnode->connections)
        computeparticulargain(nodes);
    
    currentareadiff = abs(partition0size-partition1size);
    swap(maxnode->fs,maxnode->te);
    if(gaintable[maxnode->gain].find(nodenum)!=gaintable[maxnode->gain].end())
            gaintable[maxnode->gain].erase(nodenum);

    if(gaintable[maxnode->gain].empty())
            gaintable.erase(maxnode->gain);

    maxnode->gain = -maxnode->gain;        
        
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

void setup_gaintable()
{
    gaintable.clear();
    for(auto&[nodenum,node]:CurrentCellLookUp)
        gaintable[node->gain].insert(nodenum);
}
 
bool fm()
{
    cout<<iteration<<" "<<currentcutsize<<" "
    <<" "<<partition0size<<" "<<partition1size<<"\n";
    ++iteration;
    bool stagnate = true;

        for(int i =0;i<CurrentCellLookUp.size();++i)
        {
            int maxgainnodenum;
            for(auto&[currentgain,nodenum]:gaintable)
            {
                for(auto&node:nodenum)
                {
                if(isoktomove(node))
                {
                    maxgainnodenum = node;
                    goto escape;
                }
                }
            }
            return stagnate;
            escape:
            locked.insert(maxgainnodenum);

            if(performswap(maxgainnodenum))
            {
                stagnate=false;
                partitiontable[maxgainnodenum] = CurrentCellLookUp[maxgainnodenum]->partition;
            }

        }

    return stagnate;
}

void writetoFile(char* file)
{
        
        vector<int>G1;
        vector<int>G2;
        for(auto &[nodenum,partitionnum]:partitiontable)
        {
            if(partitionnum==0)
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
        lowerbound = (double)CurrentCellLookUp.size()*(1-r)/2.0;
        upperbound = (double)CurrentCellLookUp.size()*(1+r)/2.0;
        currentareadiff = abs(partition0size-partition1size);
        bestareadiff = currentareadiff;
        bestcutsize = currentcutsize;
        computegain();
        while(!fm())
        {
            construct_current_map();
            computegain();
            locked.clear();
            currentcutsize = bestcutsize;
            currentareadiff = bestareadiff;
            partition0size = bestpartition0size;
            partition1size = bestpartition1size;
        }

        writetoFile(argv[2]);

}