#include "floorplanner.hpp"
#include<fstream>
#include<iostream>
#include<string>
#include<sstream>
#include<queue>
#include<limits>
#include<ctime>
#include<cstdlib>
#include<chrono>

void Floorplanner::readfrom_block(std::fstream& file)
{
    std::string line;
    std::string temp;
    getline(file,line);
    std::stringstream ss(line);
    std::vector<int>coordinate_holder;
    coordinate_holder.reserve(2);
    // read outline
    while(ss>>temp)
    {
        if(temp=="Outline:") continue;
        coordinate_holder.push_back(std::stoi(temp));
    }
    getoutline(coordinate_holder[0],coordinate_holder[1]);
    coordinate_holder.clear();
    // read numblocks
    getline(file,line);
    ss = std::stringstream();
    ss<<line;
    while(ss>>temp)
    {
        if(temp=="NumBlocks:") continue;
        getnumblocks(std::stoi(temp));
    }
    // read numterminals
    getline(file,line);
    ss = std::stringstream();
    ss<<line;
    while(ss>>temp)
    {
        if(temp=="NumTerminals:") continue;
        getnumterminals(std::stoi(temp));
    }
    
    // read macro & terminal names
    std::string name;
    std::string type;
    std::string width_x;
    std::string height_y;
    int index = 0;
    while(getline(file,line))
    {
        //if(line==""|line=="\r") continue;
        if(line[0] < 'A' || (line[0] > 'Z' && line[0] < 'a') || line[0] > 'z') continue;
        ss = std::stringstream();
        ss<<line;
        ss>>name;
        ss>>type;

        if(type!="terminal")
        {
            
            //Add block_x
            width_x = type;
            //Add block_y
            ss>>height_y;
            add_to_block(name,std::stoi(width_x),std::stoi(height_y));
            _map[index] = name;
            _name_to_idx[name] = index++;
            
        }
        else
        {
            //Add terminal_x
            ss>>width_x;
            //Add terminal_y
            ss>>height_y;
            add_to_terminal(name,std::stoi(width_x),std::stoi(height_y));
        }
    }
}

void Floorplanner::readfrom_nets(std::fstream &file)
{
    std::string line;
    std::string temp;
    getline(file,line);
    std::stringstream ss(line);
    ss>>temp;
    ss>>temp;
    getnumnets(std::stoi(temp));
    while(getline(file,line))
    {
        ss = std::stringstream();
        ss<<line;
        ss>>temp;
        ss>>temp;
        int net_size = std::stoi(temp);
        std::vector<std::string> visited;
        for(int i=0;i<net_size;++i)
        {
            getline(file,line);
            if(line.back()=='\r') line.pop_back();
            visited.push_back(line);
        }
        //finally add to _net
        _nets.push_back(visited);
    }
}


void Floorplanner::writeto_output(std::fstream &file,int64_t &duration)
{
    
    file<<_finalcost<<"\n";
    file<<_totalwirelength<<"\n";
    file<<_chiparea<<"\n";
    file<<_chip_width<<" "<<_chip_height<<"\n";
    file<<static_cast<float>(duration)/1000<<"\n";
    for(auto& [first,second]:_map)
    {
        file<<second<<" "<<_block_x[first]<<" "<<_block_y[first]<<" "<<
        _block_x[first] + _block_width[first]<<" "<<_block_y[first]+_block_height[first]<<"\n";
    }    

}

void Floorplanner::setup_sequencepair()
    {
        for(int i =0;i<_block_width.size();++i)
        {
            _pos_seq.emplace_back(i);
            _neg_seq.emplace_back(i);
        }

        SIZE = _pos_seq.size();
        distrib = std::uniform_int_distribution<>(0,SIZE-1);
        dist = std::uniform_int_distribution<>(1,4);
        dis = std::uniform_real_distribution<>(0,1);
        _gamma = 2.0;
        annealing_iterations = 1;
        above.resize(SIZE);
        right.resize(SIZE);
        b_above.resize(SIZE);
        b_right.resize(SIZE);
        x_cost.resize(SIZE+1);
        y_cost.resize(SIZE+1);
        _pos_idx_map.resize(SIZE);
        _neg_idx_map.resize(SIZE);
        _block_x.resize(SIZE);
        _block_y.resize(SIZE);
        _avg_area = 0.0;
        _avg_wire = 0.0;
        //randomizing initial seq_pair
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(_pos_seq.begin(), _pos_seq.end(), g);
        std::mt19937 h(rd());
        std::shuffle(_neg_seq.begin(),_neg_seq.end(),h);

        //_map[SIZE] = "tgt";

        for(int i =0;i<SIZE;++i)
            {
                _pos_idx_map[_pos_seq[i]] = i;
                _neg_idx_map[_neg_seq[i]] = i; 
            }    

        //generating initial solution
        setup_directions();
        _chip_width  = calculate_area_x();
        _chip_height = calculate_area_y();

        //x_cost.erase("tgt");
        //y_cost.erase("tgt");

        _chiparea = _chip_width*_chip_height;
        _totalwirelength = calculate_hpwl();
        _penalty = calculate_penalty();
        _finalcost = calculate_cost(_chiparea,_totalwirelength,_penalty);

        for(int i =0;i<SIZE;++i)
        {
                    _block_x[i] = -x_cost[i];
                    _block_y[i] = -y_cost[i];
        }  
    }

    // bool Floorplanner::check_for_correctness()
    // {
    //     for(int i =0;i<_pos_seq.size()-1;++i)
    //     {
    //         for(int j =i+1;j<_pos_seq.size();++j)
    //         {
    //             if(left[_pos_seq[i]].find(_pos_seq[j])==left[_pos_seq[i]].end()
    //             &&left[_pos_seq[j]].find(_pos_seq[i])==left[_pos_seq[j]].end()
    //             &&below[_pos_seq[i]].find(_pos_seq[j])==below[_pos_seq[i]].end()
    //             &&below[_pos_seq[j]].find(_pos_seq[i])==below[_pos_seq[j]].end())
    //                 return false;

    //             if((left[_pos_seq[i]].find(_pos_seq[j])!=left[_pos_seq[i]].end()
    //             &&below[_pos_seq[i]].find(_pos_seq[j])!=below[_pos_seq[i]].end())||
    //             (left[_pos_seq[j]].find(_pos_seq[i])!=left[_pos_seq[j]].end()
    //             &&below[_pos_seq[j]].find(_pos_seq[i])!=below[_pos_seq[j]].end()))
    //                 return false;    
    //         }
    //     }

    //     return true;
    // }

// bool Floorplanner::overlap(std::vector<int> &m1, std::vector<int> &m2)
// {
//     return !((m1[2] <= m2[0] or m1[0] >= m2[2] or m1[3] <= m2[1] or m1[1] >= m2[3]));
// }    

// bool Floorplanner::check_solution()
// {
    
//     std::vector<std::vector<int>> outline;

//     for(auto& [first,second]:_map)
//     {
//         std::vector<int> _temp;
//         _temp.push_back(-x_cost[first]);
//         _temp.push_back(-y_cost[first]);
//         _temp.push_back(-x_cost[first] + _block_width[first]);
//         _temp.push_back(-y_cost[first]+_block_height[first]);
//         outline.push_back(_temp);
//     }

//     for(int i =0;i<outline.size();++i)
//     {
//         for(int j =0;j<outline.size();++j)
//         {
//             if(i!=j&&overlap(outline[i],outline[j]))
//                 return false;
//         }
//     }

//     return true;

// }    

void Floorplanner::solve()
{
    retry:
    float T = 1000;
    float theta = 0.95;
    while(T>0.1)
    {
        std::size_t bad_solution_accept = 0;
        std::size_t good_solution_accept = 0;
        int _try = 3000;
        //auto _start = std::chrono::high_resolution_clock::now();
        while(--_try)
        {
        ++annealing_iterations;
        //auto _start = std::chrono::high_resolution_clock::now();
        auto[temp_chip_width,temp_chip_height] = perform_validperturbation();
        //auto _end = std::chrono::high_resolution_clock::now();
        //std::cout<<"Time needed per iteration: "<<std::chrono::duration_cast<std::chrono::microseconds>(_end - _start).count()<<"\n";
        // if(!check_for_correctness())
        //     {
        //         std::cout<<"Illegal solution";
        //         return;
        //     }

        // if(!check_solution())
        //     {
        //         std::cout<<"Overlap!";
        //         return;
        //     }

        std::size_t temp_chiparea = temp_chip_height*temp_chip_width;
        //auto _start = std::chrono::high_resolution_clock::now();
        float temp_totalwirelength = calculate_hpwl();
        //auto _end = std::chrono::high_resolution_clock::now();
        //std::cout<<"Time needed per iteration: "<<std::chrono::duration_cast<std::chrono::microseconds>(_end - _start).count()<<"\n";
        //auto _start = std::chrono::high_resolution_clock::now();
        std::size_t temp_penalty = calculate_penalty();
        //auto _end = std::chrono::high_resolution_clock::now();
        //std::cout<<"Time needed per iteration: "<<std::chrono::duration_cast<std::chrono::microseconds>(_end - _start).count()<<"\n";
        float temp_finalcost = calculate_cost(temp_chiparea,temp_totalwirelength,temp_penalty);
        //std::cout<<"Finalcost_diff: "<<temp_finalcost-_finalcost<<"\n";
        if(temp_finalcost<_finalcost)
            {
                ++good_solution_accept;
                _finalcost = temp_finalcost;
                _chip_height = temp_chip_height;
                _chip_width = temp_chip_width;
                _chiparea = temp_chiparea;
                _totalwirelength = temp_totalwirelength;

                for(int i =0;i<SIZE;++i)
        {
                    _block_x[i] = -x_cost[i];
                    _block_y[i] = -y_cost[i];
        }
  
        //std::cout<<"Chip Height: "<<_chip_height<<"\n";
        //std::cout<<"Chip Width: "<<_chip_width<<"\n";          

            }
        else
            {
                std::random_device rd;
                std::mt19937 gen_f(rd());
                
                float random = dis(gen_f);
                if(random<std::exp(-(temp_finalcost-_finalcost)*1000/T))
                {
                    ++bad_solution_accept;
                
                _finalcost = temp_finalcost;
                _chip_height = temp_chip_height;
                _chip_width = temp_chip_width;
                _chiparea = temp_chiparea;
                _totalwirelength = temp_totalwirelength;

                for(int i =0;i<SIZE;++i)
        {
                    _block_x[i] = -x_cost[i];
                    _block_y[i] = -y_cost[i];
        }

        //std::cout<<"Chip Height: "<<_chip_height<<"\n";
        //std::cout<<"Chip Width: "<<_chip_width<<"\n";       

                }
                else
                {
                //std::cout<<"Restored!\n";
                _avg_area = (_avg_area*annealing_iterations-temp_chiparea)/(annealing_iterations-1);
                _avg_wire = (_avg_wire*annealing_iterations-temp_totalwirelength)/(annealing_iterations-1);
                --annealing_iterations;
                restore();
                }
            }  
            //if(_chip_width<=_outline_x&&_chip_height<=_outline_y) return;
             clear();
            }
            //auto _end = std::chrono::high_resolution_clock::now();
            //std::cout<<"Time needed per iteration: "<<std::chrono::duration_cast<std::chrono::microseconds>(_end - _start).count()<<"\n";
            // std::cout<<"Average area is: "<<_avg_area<<"\n";
            // std::cout<<"Average wirelength is: "<<_avg_wire<<"\n";
            // std::cout<<"Temperature is:"<<T<<"\n";
            // std::cout<<"Good solution accept:"<<good_solution_accept<<"\n";
            // std::cout<<"Bad solution accept:"<<bad_solution_accept<<"\n"; 
            T*=theta;    
    }
        if(_chip_width<=_outline_x&&_chip_height<=_outline_y) return;
        _gamma*=2.0;
        _gamma = std::min(100.0f,_gamma);
        if(_gamma ==100.0f) _gamma = 2.0f;
        goto retry;    

}

void Floorplanner::setup_directions()
{

    for(int i =0;i<SIZE-1;++i)
    {
        for(int j = i+1;j<SIZE;++j)
        {
            if(_neg_idx_map[_pos_seq[j]]>_neg_idx_map[_pos_seq[i]])
                right[_pos_seq[i]].push_back(_pos_seq[j]); 
        }

        right[_pos_seq[i]].push_back(SIZE/*"tgt"*/);
    }
    right[_pos_seq[SIZE-1]].push_back(SIZE);

    for(int i = SIZE-1;i>0;--i)
    {    

        for(int j =i-1;j>=0;--j)
        {
            if(_neg_idx_map[_pos_seq[j]]>_neg_idx_map[_pos_seq[i]])
                above[_pos_seq[i]].push_back(_pos_seq[j]);
        }

        above[_pos_seq[i]].push_back(SIZE/*"tgt"*/);

    }
    above[_pos_seq[0]].push_back(SIZE);

}

std::size_t Floorplanner::calculate_area_x()
{
        for(int i =0;i<SIZE+1;++i)
            x_cost[i] = 0;
        
    std::queue<int> q;
    std::unordered_set<int> present;

    for(auto& first:_pos_seq)
        q.push(first);

    while(!q.empty())
    {
        auto u = q.front();
        q.pop();
        present.erase(u);
        for(auto &v: right[u])
            {
                if(x_cost[u]-_block_width[u]<x_cost[v])
                {
                    x_cost[v] = x_cost[u] - _block_width[u];
                    if(v!=SIZE&&present.find(v)==present.end())
                    {
                    q.push(v);
                    present.insert(v);
                    }
                }    
            }
    }

    return -x_cost[SIZE];
}

std::size_t Floorplanner::calculate_area_y()
{
    for(int i =0;i<SIZE+1;++i)
        y_cost[i] = 0;  

    std::queue<int> q;
    std::unordered_set<int> present;

    for(auto &first:_neg_seq)
        q.push(first);
    while(!q.empty())
    {
        auto u = q.front();
        q.pop();
        present.erase(u);
        for(auto &v: above[u])
            {
                if(y_cost[u]-_block_height[u]<y_cost[v])
                {
                    y_cost[v] = y_cost[u] - _block_height[u];
                    if(v!=SIZE&&present.find(v)==present.end())
                    {
                        q.push(v);
                        present.insert(v);
                    }    
                }    
            }
    }

    return -y_cost[SIZE];

}

inline std::size_t Floorplanner::calculate_penalty()
{
    std::size_t penalty = 1;
    for(int i =0;i<x_cost.size()-1;++i)
    {
        if(-x_cost[i]+_block_width[i]>_outline_x)
            penalty+= (-x_cost[i]+_block_width[i] - _outline_x)*_gamma;
        if(-y_cost[i]+_block_height[i]>_outline_y)
            penalty+= (-y_cost[i]+_block_height[i] - _outline_y)*_gamma;    
    }

    return penalty;
}

inline float Floorplanner::calculate_hpwl()
{
    float hpwl = 0;
    for(auto &net:_nets)
    {
        float max_x = std::numeric_limits<int>::min();
        float max_y = std::numeric_limits<int>::min();
        float min_x = std::numeric_limits<int>::max();
        float min_y = std::numeric_limits<int>::max();
        for(auto &str:net)
        {
            if(_terminal_x.find(str)!=_terminal_x.end())
            {
            max_x = std::max(max_x,static_cast<float>(_terminal_x[str]));
            min_x = std::min(min_x,static_cast<float>(_terminal_x[str]));
            max_y = std::max(max_y,static_cast<float>(_terminal_y[str]));
            min_y = std::min(min_y,static_cast<float>(_terminal_y[str]));
            }
            else
            {
            int a = _name_to_idx[str];    
            max_x = std::max(max_x,static_cast<float>(-x_cost[a]+(_block_width[a]/2)));
            min_x = std::min(min_x,static_cast<float>(-x_cost[a]+(_block_width[a]/2)));
            max_y = std::max(max_y,static_cast<float>(-y_cost[a]+(_block_height[a]/2)));
            min_y = std::min(min_y,static_cast<float>(-y_cost[a]+(_block_height[a]/2)));
            }
        }

        hpwl+=(max_x-min_x)+(max_y-min_y);
    }

    return hpwl;
}

inline float Floorplanner::calculate_cost(std::size_t temp_chiparea,float temp_totalwirelength,std::size_t temp_penalty)
{
    _avg_area = ((((annealing_iterations-1)*_avg_area)+temp_chiparea)/static_cast<float>(annealing_iterations));
    _avg_wire = ((((annealing_iterations-1)*_avg_wire)+temp_totalwirelength)/static_cast<float>(annealing_iterations));
    return (alpha)*(static_cast<float>(temp_chiparea)/static_cast<float>(_avg_area)) + (1-alpha)*((temp_totalwirelength+(_gamma*static_cast<float>(temp_penalty)))/_avg_wire);
}

std::pair<std::size_t,std::size_t> Floorplanner::perform_validperturbation()
{
        //auto _start = std::chrono::high_resolution_clock::now();
        std::random_device dev;
        std::mt19937 gen(dev());
        std::size_t width,height;
        std::unordered_set<int> changed;

        _choice = dist(gen);
        switch(_choice)
        {
            case 1:
            {
                //std::cout<<"At 1\n";
                do
                {
                    rand1 =  distrib(gen);
                    rand2 =  distrib(gen);
                } while (rand1==rand2);

                if(rand1>rand2)
                std::swap(rand1,rand2);

                for(int i = rand1;i<=rand2;++i)
                {
                    b_above[_pos_seq[i]] = above[_pos_seq[i]];
                    b_right[_pos_seq[i]] = right[_pos_seq[i]];
                    changed.insert(_pos_seq[i]);
                }    

                _pos_idx_map[_pos_seq[rand1]] = rand2;
                _pos_idx_map[_pos_seq[rand2]] = rand1;
                std::swap(_pos_seq[rand1],_pos_seq[rand2]);
                break;
            }

            case 2:
            {
                //std::cout<<"At 2\n";
                do
                {
                    rand1 =  distrib(gen);
                    rand2 =  distrib(gen);
                } while (rand1==rand2);

                if(rand1>rand2)
                std::swap(rand1,rand2);

                for(int i = rand1;i<=rand2;++i)
                {
                    b_above[_neg_seq[i]] = above[_neg_seq[i]];
                    b_right[_neg_seq[i]] = right[_neg_seq[i]];
                    changed.insert(_neg_seq[i]);
                }    

                _neg_idx_map[_neg_seq[rand1]] = rand2;
                _neg_idx_map[_neg_seq[rand2]] = rand1;
                std::swap(_neg_seq[rand1],_neg_seq[rand2]);
                break;
            }

            case 3:
            {
                //std::cout<<"At 3\n";
                do
                {
                    rand1 = distrib(gen); 
                    rand2 = distrib(gen);
  
                } while (rand1==rand2);

                    if(rand1>rand2)
                    std::swap(rand1,rand2);

                 rand3 = _neg_idx_map[_pos_seq[rand1]];
                 rand4 = _neg_idx_map[_pos_seq[rand2]];

                    if(rand3>rand4)
                    std::swap(rand3,rand4);    

                for(int i = rand1;i<=rand2;++i)
                {
                    b_above[_pos_seq[i]] = above[_pos_seq[i]];
                    b_right[_pos_seq[i]] = right[_pos_seq[i]];
                    changed.insert(_pos_seq[i]);
                }    

                for(int i = rand3;i<=rand4;++i)
                {
                    if(b_above[_neg_seq[i]].empty()) b_above[_neg_seq[i]] = above[_neg_seq[i]];
                    if(b_right[_neg_seq[i]].empty()) b_right[_neg_seq[i]] = right[_neg_seq[i]];
                    changed.insert(_neg_seq[i]);
                }
                _pos_idx_map[_pos_seq[rand1]] = rand2;
                _pos_idx_map[_pos_seq[rand2]] = rand1;
                _neg_idx_map[_neg_seq[rand3]] = rand4;
                _neg_idx_map[_neg_seq[rand4]] = rand3;
                std::swap(_pos_seq[rand1],_pos_seq[rand2]);
                std::swap(_neg_seq[rand3],_neg_seq[rand4]);
                break;

            }
            case 4:
            {
                //std::cout<<"At 4\n";
                rand1 = distrib(gen);
                std::swap(_block_width[rand1],_block_height[rand1]);
                break;
            }

        }
        //auto _end = std::chrono::high_resolution_clock::now();
        //std::cout<<"Choice: "<<_choice<<"\n";
        //std::cout<<"Time needed per iteration: "<<std::chrono::duration_cast<std::chrono::microseconds>(_end - _start).count()<<"\n";
        for(auto& index:changed)
        {
            right[index].clear();
            int pos_idx = _pos_idx_map[index]+1;
            int neg_idx = _neg_idx_map[index];
            while(pos_idx<SIZE)
            {
                if(neg_idx<_neg_idx_map[_pos_seq[pos_idx]])
                    right[index].push_back(_pos_seq[pos_idx]);
                ++pos_idx;    
            }
            right[index].push_back(SIZE);
        }
        for(auto& index:changed)
        {
            above[index].clear();
            int pos_idx = _pos_idx_map[index]-1;
            int neg_idx = _neg_idx_map[index];
            while(pos_idx>=0)
            {
                if(neg_idx<_neg_idx_map[_pos_seq[pos_idx]])
                    above[index].push_back(_pos_seq[pos_idx]);
                --pos_idx;    
            }
            above[index].push_back(SIZE);
        }
        //auto _start = std::chrono::high_resolution_clock::now();
        width = calculate_area_x();
        //auto _end = std::chrono::high_resolution_clock::now();
        //std::cout<<"Time needed per iteration: "<<std::chrono::duration_cast<std::chrono::microseconds>(_end - _start).count()<<"\n";
        height = calculate_area_y();
        //x_cost.erase("tgt");
        //y_cost.erase("tgt");

        return {width,height};
        
}

void Floorplanner::restore()
{

        if(_choice==1||_choice==3)
        {
        std::swap(_pos_seq[rand1],_pos_seq[rand2]);
        _pos_idx_map[_pos_seq[rand1]] = rand1;
        _pos_idx_map[_pos_seq[rand2]] = rand2;
        }
        if(_choice==2)
        {
        std::swap(_neg_seq[rand1],_neg_seq[rand2]);
        _neg_idx_map[_neg_seq[rand1]] = rand1;
        _neg_idx_map[_neg_seq[rand2]] = rand2;
        }
        if(_choice==3)
        {
        std::swap(_neg_seq[rand3],_neg_seq[rand4]);
        _neg_idx_map[_neg_seq[rand3]] = rand3;
        _neg_idx_map[_neg_seq[rand4]] = rand4;
        }
        if(_choice==4)
            std::swap(_block_width[rand1],_block_height[rand1]);

    for(int i =0;i<SIZE;++i)
    {
        if(!b_above[i].empty())
            above[i] = b_above[i];
        if(!b_right[i].empty())
            right[i] = b_right[i];
    }                                        


}

void Floorplanner::clear()
{
    for(int i =0;i<SIZE;++i)
    {
        b_above[i] = std::vector<int>();
        b_right[i] = std::vector<int>();
    }    
}