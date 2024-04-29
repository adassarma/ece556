#pragma once

#include<fstream>
#include<algorithm>
#include <tao/pegtl.hpp>

namespace pegtl = tao::pegtl;

namespace Steiner
{
    
    struct Coordinate
    {
        std::size_t _x = 0;
        std::size_t _y = 0;
        Coordinate() = default;
        Coordinate(const size_t x, const size_t y): _x{x},_y{y} {}
    };

    struct Pin
    {
        std::string _name;
        Coordinate _coord;
        Pin() = default;
        Pin(const std::string name,const Coordinate coord): _name{name}, _coord{coord} {};
    };

    class Steiner
    {
        public:
        Steiner(const std::string&);
        void solve();
        void writeToOutput(std::fstream&);

        private:
        std::vector<Pin> _pin;
        std::pair<Coordinate,Coordinate> _chip_boundary;
        std::size_t numpins = 0;
        std::vector<int> _source;
        std::vector<bool> _picked;
        std::vector<int> _dist;
        void handle(const std::string&,std::pair<Coordinate,Coordinate>&, std::size_t&, std::vector<Pin>&);
        int _assign_min_source(int const);
        int _calc_distance(int const,int const);

    };

    inline Steiner::Steiner(const std::string& input)
    {
        handle(input,_chip_boundary,numpins,_pin);
        std::sort(_pin.begin(),_pin.end(),[](const Pin&a,const Pin&b){return a._coord._x+a._coord._y<b._coord._x+b._coord._y;});
    }

    inline void Steiner::solve()
    {
        _picked.resize(numpins);
        _source.resize(numpins);
        _dist.resize(numpins,INT32_MAX);
        int chosen = 0;
        _source[chosen] = -1;
        _dist[chosen] = 0;
        for(int i=0;i<numpins-1;++i)
        {
            chosen = _assign_min_source(chosen);
            _picked[chosen] = true;
        }

    }

    inline int Steiner::_assign_min_source(int const chosen)
    {
        int min_dist = INT32_MAX, min_index= -1,span = 2, _span = 0;
        while(1)
        {
            for(;_span<span;++_span)
            {
                if((chosen-_span)>=0&&!_picked[chosen-_span]&&_dist[chosen-_span]<min_dist)
                    {
                        min_index = chosen-_span;
                        min_dist = _dist[chosen-_span];
                    }
                if((chosen+_span)<numpins&&!_picked[chosen+_span]&&_dist[chosen+_span]<min_dist)
                {
                    min_index = chosen+_span;
                    min_dist = _dist[chosen+_span];
                }     
            }

            if(min_index!=-1) break;
            ++span;
        }

        for(int _span = 1;_span<span;++_span)
        {
            if((min_index-_span)>=0&&!_picked[min_index-_span])
            {
                int dist = _calc_distance(min_index,(min_index-_span));
                if(dist<_dist[min_index-_span])
                {
                    _dist[min_index-_span] = dist;
                    _source[min_index-_span] = min_index;
                }
            }

            if((min_index+_span)<numpins&&!_picked[min_index+_span])
            {
                int dist = _calc_distance(min_index,(min_index+_span));
                if(dist<_dist[min_index+_span])
                {
                    _dist[min_index+_span] = dist;
                    _source[min_index+_span] = min_index;
                }
            }
        }

        return min_index;
    }

    inline int Steiner::_calc_distance(int const a,int const b)
    {
        return std::abs(static_cast<int>(_pin[a]._coord._x-_pin[b]._coord._x))+std::abs(static_cast<int>(_pin[a]._coord._y-_pin[b]._coord._y));
    }

    inline void Steiner::writeToOutput(std::fstream&file)
    {
        std::size_t wirelength = 0;

        for(int i =1;i<numpins;++i)
            wirelength += _calc_distance(i,_source[i]);

        file<<"NumRoutedPins: = "<<numpins<<"\n";
        file<<"WireLength = "<<wirelength<<"\n";

        for(int i =1;i<numpins;++i)
        {
            if(_pin[i]._coord._x!=_pin[_source[i]]._coord._x)
            {
                file<<"H-line ("<<_pin[i]._coord._x<< ","<< _pin[_source[i]]._coord._y
              << ") ("<< _pin[_source[i]]._coord._x<< ","<< _pin[_source[i]]._coord._y<<")\n";
            }
            else if(_pin[i]._coord._y!=_pin[_source[i]]._coord._y)
            {
                file<<"V-line ("<<_pin[i]._coord._x<< ","<< _pin[i]._coord._y
              << ") ("<< _pin[i]._coord._x<< ","<< _pin[_source[i]]._coord._y<<")\n";
            }
        }

    }
}