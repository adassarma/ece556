#include<iostream>
#include<fstream>
#include<string>
#include<sstream>
#include<map>
#include<vector>
#include "essentials.hpp"
#include<chrono>
using namespace std;
void construct_current_map()
{
    for(auto&[nodenum,partitionnum]:partitiontable)
        CurrentCellLookUp[nodenum]->partition = partitionnum;

    netpart0.clear();
    netpart1.clear();

    for(auto&net:unique_nets)
    {
        int goto0=0;
        int goto1=0;

        for(auto&net_num:net)
        {
            if(CurrentCellLookUp[net_num]->partition==0)
            goto0++;
            else
            goto1++;
        }

        netpart0.push_back(goto0);
        netpart1.push_back(goto1);
    }

}
void readfromFile(char *file)
{
    int flip = 1;
    string line;
    bool first = true;
    ifstream inputFile;
    int netnumber = 0;
    vector<int> net;
    bool flag = true;
    int zeropartition=0;
    int firstpartition=0;
    unordered_set<int> duplicate;
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
                    char lastelem;
                    while(getline(ss,temp,' '))
                    {
                        char ch = temp[0];
                        lastelem = ch;
                        if(temp[0]=='c')
                        {
                            temp.erase(temp.begin());
                            int cellnum = stoi(temp);
                           if(duplicate.find(cellnum)!=duplicate.end())
                            continue;
                            duplicate.insert(cellnum);
                            Cell *cell;
                            if(CurrentCellLookUp.find(cellnum)==CurrentCellLookUp.end())
                            {
                            flip = 1-flip;
                            cell = new Cell(cellnum,flip);
                            CurrentCellLookUp[cellnum] = cell;
                            partitiontable[cellnum] = flip;
                            if(flip==0)
                            {
                                ++partition0size;
                            }
                            else
                            {
                            ++partition1size;
                            }
                            }
                            else
                            cell = CurrentCellLookUp[cellnum];
                    cell->connections.push_back(netnumber);
                    if(cell->partition==0)
                    ++zeropartition;
                    else
                    ++firstpartition;
                    if(flag&&!net.empty()&&partitiontable[net.back()]!=partitiontable[cellnum])
                    {
                        flag = false;
                        ++currentcutsize;
                    }
                            net.push_back(cellnum);
                        }
                    }
                    if(lastelem==';')
                    {
                        unique_nets.push_back(net);
                        netpart0.push_back(zeropartition);
                        netpart1.push_back(firstpartition);
                        flag = true;
                        ++netnumber;
                        //cout<<"Netnumber :"<<netnumber<<"\n";
                        duplicate.clear();
                        net.clear();
                        firstpartition=0;
                        zeropartition=0;
                    }
                }
            }
    inputFile.close();
    }
  else cout << "Unable to open file";
}
void computegain()
{
            for(int i =0;i<unique_nets.size();++i)
            {
                vector<int>&net = unique_nets[i];

                for(auto&net_num:net)
                {

                auto node = CurrentCellLookUp[net_num];
                    if(visitednode.find(net_num)==visitednode.end())
                    {
                    node->fs = 0;
                    node->te = 0;
                    visitednode.insert(net_num);
                    }   
                }

            int p0size = netpart0[i];
            int p1size = netpart1[i];    


            if(p0size==net.size()||p1size==net.size())
            {
                for(auto&net_num:net)
                    ++CurrentCellLookUp[net_num]->te;
            }    
            
            else if(p0size==1&&p1size==1)
            {
                int p0cell,p1cell;

                for(auto&net_num:net)
                {
                    if(CurrentCellLookUp[net_num]->partition==0)
                    p0cell = net_num;
                    else
                    p1cell = net_num;
                }
                
                ++CurrentCellLookUp[p0cell]->fs;
                ++CurrentCellLookUp[p1cell]->fs;
            }

            else if(p0size==1)
            {
                int p0cell;
                for(auto&net_num:net)
                {
                    if(CurrentCellLookUp[net_num]->partition==0)
                    {
                        p0cell = net_num; 
                        break;
                    }
                }
                ++CurrentCellLookUp[p0cell]->fs;
            }    
            
            else if(p1size==1)
            {    
                int p1cell;
                for(auto&net_num:net)
                {
                    if(CurrentCellLookUp[net_num]->partition==1)
                    {
                        p1cell = net_num; 
                        break;
                    }
                }
                ++CurrentCellLookUp[p1cell]->fs;
            }            
    }
}

void consolidategain()
{
        for(auto&[nodenum,node]:CurrentCellLookUp)
        {    node->gain = node->fs-node->te;
            gaintable[node->gain].insert(nodenum);
        }        
}

void consolidateparticulargain()
{
    
    for(auto&nodenum:locallyvisited)
    {
    
    auto maxnode = CurrentCellLookUp[nodenum];
    
    if(gaintable[maxnode->gain].find(nodenum)!=gaintable[maxnode->gain].end())
            gaintable[maxnode->gain].erase(nodenum);
    
    if(gaintable[maxnode->gain].empty())
            gaintable.erase(maxnode->gain);
    
    maxnode->gain = maxnode->fs-maxnode->te;        
    
    if(locked.find(nodenum)==locked.end())
        gaintable[maxnode->gain].insert(nodenum);

    }    
}

void computeparticulargain(int numericnet,int targetnode)
{
            vector<int> &net = unique_nets[numericnet];

            if(CurrentCellLookUp[targetnode]->partition==1)
            {
                --netpart0[numericnet];
                ++netpart1[numericnet];
            }
            else
            {
                ++netpart0[numericnet];
                --netpart1[numericnet];
            }
            

            int p0size = netpart0[numericnet];
            int p1size = netpart1[numericnet];      
            locallyvisited.insert(targetnode);


            if(p0size==net.size()||p1size==net.size())
            {
                for(auto &net_num:net)
                    locallyvisited.insert(net_num);
                
                for(auto&net_num:net)
                        ++CurrentCellLookUp[net_num]->te;
                
                if(net.size()==2)
                {
                    for(auto&net_num:net)
                        --CurrentCellLookUp[net_num]->fs;
                }
                else
                --CurrentCellLookUp[targetnode]->fs;

            }    
            
            else if(p0size==1&&p1size==1)
            {
                for(auto &net_num:net)
                    locallyvisited.insert(net_num);
                
                int p0cell,p1cell;

                for(auto&net_num:net)
                {
                    if(CurrentCellLookUp[net_num]->partition==0)
                    p0cell = net_num;
                    else
                    p1cell = net_num;
                }
                
                ++CurrentCellLookUp[p0cell]->fs;
                ++CurrentCellLookUp[p1cell]->fs;
                --CurrentCellLookUp[p0cell]->te;
                --CurrentCellLookUp[p1cell]->te;
            }

            else if(p0size==1)
            {
                for(auto &net_num:net)
                    locallyvisited.insert(net_num);
                
                int p0cell;
                for(auto &net_num:net)
                {
                if(CurrentCellLookUp[net_num]->partition==0)
                {
                    p0cell = net_num;
                    break;
                }
                }
                ++CurrentCellLookUp[p0cell]->fs;

                if(p0cell==targetnode)
                {
                for(auto&net_num:net)
                        --CurrentCellLookUp[net_num]->te;
                }

                else if(p1size==2&&CurrentCellLookUp[targetnode]->partition==1)
                    {
                for(auto&net_num:net)
                {
                    if(net_num!=targetnode&&CurrentCellLookUp[net_num]->partition==1)
                    {
                    --CurrentCellLookUp[net_num]->fs;
                    break;
                    }
                }
                    }        
            }    
            
            else if(p1size==1)
            {
                for(auto&net_num:net)
                    locallyvisited.insert(net_num);
                
                int p1cell;
                for(auto &net_num:net)
                {
                if(CurrentCellLookUp[net_num]->partition==1)
                {
                    p1cell = net_num;
                    break;
                }
                }
                ++CurrentCellLookUp[p1cell]->fs;
                
                    
                if(p1cell==targetnode)
                {
                for(auto&net_num:net)
                        --CurrentCellLookUp[net_num]->te;
                }

                else if(p0size==2&&CurrentCellLookUp[targetnode]->partition==0)
                    {
                for(auto&net_num:net)
                {
                    if(net_num!=targetnode&&CurrentCellLookUp[net_num]->partition==0)
                    {
                    --CurrentCellLookUp[net_num]->fs;
                    break;
                    }
                }
                    }

            }

            else if(p0size==2&&CurrentCellLookUp[targetnode]->partition==0)
            {
                for(auto&net_num:net)
                {    
                    locallyvisited.insert(net_num);
               if(net_num!=targetnode&&CurrentCellLookUp[net_num]->partition==0)
                    --CurrentCellLookUp[net_num]->fs;
                }    
            }
            else if(p1size==2&&CurrentCellLookUp[targetnode]->partition==1)
            {
                for(auto&net_num:net)
                {    
                    locallyvisited.insert(net_num);
                
                if(net_num!=targetnode&&CurrentCellLookUp[net_num]->partition==1)
                    --CurrentCellLookUp[net_num]->fs;
                }    
            }    
            
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

    for(auto &net:maxnode->connections)
            computeparticulargain(net,nodenum);

    consolidateparticulargain();
    locallyvisited.clear();        

    currentareadiff = abs(partition0size-partition1size);
    
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
    //cout<<iteration<<" "<<currentcutsize<<" "
    //<<" "<<partition0size<<" "<<partition1size<<"\n";
    ++iteration;
    if(iteration==5) return true;
    bool stagnate = true;
        while(locked.size()!=CurrentCellLookUp.size())
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
            pendingpartitionswap.push_back(maxgainnodenum);
            if(performswap(maxgainnodenum))
            {
                stagnate=false;
                for(auto&pendingnodes:pendingpartitionswap)
                    partitiontable[pendingnodes] = CurrentCellLookUp[pendingnodes]->partition;
                pendingpartitionswap.clear();
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
        auto start_time = chrono::high_resolution_clock::now();
        lowerbound = (double)CurrentCellLookUp.size()*(1-r)/2.0;
        upperbound = (double)CurrentCellLookUp.size()*(1+r)/2.0;
        currentareadiff = abs(partition0size-partition1size);
        bestareadiff = currentareadiff;
        bestcutsize = currentcutsize;
        computegain();
        consolidategain();
        while(!fm())
        {
            construct_current_map();
            //cout<<"Size of gaintable is: "<<gaintable.size()<<"\n";
            locked.clear();
            visitednode.clear();
            pendingpartitionswap.clear();
            computegain();
            consolidategain();
            //cout<<"Highest gain value for this iteration: "<<gaintable.begin()->first<<"\n";
            currentcutsize = bestcutsize;
            partition0size = bestpartition0size;
            partition1size = bestpartition1size;
        }
        auto end_time = chrono::high_resolution_clock::now();
        cout<<"Runtime: "<<chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count() << "\n";
        cout<<"Cutsize: "<<bestcutsize<<"\n";
        writetoFile(argv[2]);
}
