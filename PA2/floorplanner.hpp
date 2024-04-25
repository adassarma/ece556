#include<vector>
#include<memory>
#include<string>
#include<random>
#include<unordered_map>
#include<unordered_set>
#include<algorithm>
#include<map>

class Floorplanner
{
    
    public:

    //parser methods
    void readfrom_block(std::fstream &file);
    void readfrom_nets(std::fstream &file);
    void writeto_output(std::fstream &file,int64_t &duration);

    //initial sequence setup method
    void setup_sequencepair();

    //solver methods
    void solve();
    void setup_directions();
    std::pair<std::size_t,std::size_t> perform_validperturbation();
    std::size_t calculate_area_x();
    std::size_t calculate_area_y();
    float calculate_hpwl();
    std::size_t calculate_penalty();
    //bool check_solution();
    //bool check_for_correctness();
    //bool overlap(std::vector<int>&,std::vector<int>&);
    float calculate_cost(std::size_t temp_chiparea,float temp_totalwirelength,std::size_t temp_penalty);
    void restore();
    void clear();
    // accessor methods
    void getnumblocks(int numblocks)
    {
        this->_numblocks = numblocks;
    }
    
    void getnumterminals(int numterminals)
    {
        this->_numterminals = numterminals;
    }

    void getnumnets(int numnets)
    {
        this->_numnets = numnets;
    }

    void getoutline(int x, int y)
    {
        this->_outline_x = x;
        this->_outline_y = y;
    }
    

    void add_to_terminal(std::string name, int width_x, int height_y)
    {
            this->_terminal_x.emplace(std::make_pair(name,width_x));
            this->_terminal_y.emplace(std::make_pair(name,height_y));
    }

    void add_to_block(std::string name, int width_x, int height_y)
    {
            this->_block_width.push_back(width_x);
            this->_block_height.push_back(height_y);
    }

    void setalpha(float alpha)
    {
        this->alpha = alpha;
    }

    private:
    std::size_t _numblocks;
    std::size_t _numterminals;
    std::size_t _numnets;
    float alpha;
    float _gamma;
    std::size_t _outline_x;
    std::size_t _outline_y;
    std::size_t _penalty;
    std::uniform_int_distribution<> distrib;
    std::uniform_int_distribution<> dist;
    std::uniform_real_distribution<> dis;
    std::size_t annealing_iterations;
    float _avg_area;
    float _avg_wire;
    

    // for output file
    float _finalcost;
    float _totalwirelength;
    std::size_t _chiparea;
    std::size_t _chip_width;
    std::size_t _chip_height;
    int _choice;
    std::size_t SIZE;


    // important data structures for state
    std::vector<std::vector<std::string>> _nets;
    std::unordered_map<int,std::string> _map;
    std::vector<int> _pos_idx_map;
    std::vector<int> _neg_idx_map;
    std::unordered_map<std::string,int> _name_to_idx;
    std::size_t rand1;
    std::size_t rand2;
    std::size_t rand3;
    std::size_t rand4;
    
    //current

    std::vector<int> _block_x;
    std::vector<int> _block_y;

    std::vector<int> _block_height;
    std::vector<int> _block_width;

    std::unordered_map<std::string,int> _terminal_x;
    std::unordered_map<std::string,int> _terminal_y;

    std::vector<std::vector<int>> above;
    std::vector<std::vector<int>> right;

    std::vector<int> _pos_seq;
    std::vector<int> _neg_seq;

    //backup
    std::unordered_map<std::string,int> b_block_height;
    std::unordered_map<std::string,int> b_block_width;
    std::vector<std::vector<int>> b_right;
    std::vector<std::vector<int>> b_above;

    //scratchpad
    std::vector<int> x_cost;
    std::vector<int> y_cost;
};