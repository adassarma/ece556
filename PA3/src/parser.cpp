#include <iostream>
#include <string>
#include <map>
#include <tao/pegtl.hpp>
#include "steiner.hpp"

namespace pegtl = tao::pegtl;

namespace parser {

    // Define grammar rules

    //Rule for integers
    struct integer: pegtl::seq<pegtl::opt<pegtl::one<'+','-'>>,pegtl::plus<pegtl::digit>> {};

    //Rule for decimals
    struct decimal: pegtl::seq<integer,pegtl::opt<pegtl::seq<pegtl::one<'.'>,pegtl::star<pegtl::digit>>>>{};

    //Rule for coordinates
    struct Coordinate: pegtl::seq<pegtl::one<'('>,pegtl::star<pegtl::space>,integer,pegtl::star<pegtl::space>,pegtl::one<','>,pegtl::star<pegtl::space>,integer,pegtl::star<pegtl::space>,pegtl::one<')'>> {};

    // Rule for parsing Boundary instruction
    struct Boundary : pegtl::seq<pegtl::istring<'B', 'o', 'u', 'n', 'd', 'a', 'r', 'y'>, pegtl::plus<pegtl::space>, pegtl::one<'='>,pegtl::plus<pegtl::space>,Coordinate,pegtl::star<pegtl::space>,pegtl::one<','>,pegtl::star<pegtl::space>,Coordinate,pegtl::sor<pegtl::star<pegtl::space>,pegtl::eol>> {};

    // Rule for parsing NumPins instruction
    struct Numpins : pegtl::seq<pegtl::istring<'N', 'u', 'm', 'P','i','n','s'>, pegtl::plus<pegtl::space>, pegtl::one<'='>,pegtl::plus<pegtl::space>,integer, pegtl::sor<pegtl::star<pegtl::space>,pegtl::eol>> {};

    // Rule for parsing Pin instruction
    struct Pin : pegtl::seq<pegtl::istring<'P', 'I', 'N'>, pegtl::plus<pegtl::space>, pegtl::plus<pegtl::identifier_other>,pegtl::plus<pegtl::space>,Coordinate,pegtl::sor<pegtl::star<pegtl::space>,pegtl::eol>> {};

    

    // Rule for parsing any instruction
    struct Grammar : pegtl::until<pegtl::eof,pegtl::sor<Boundary, Numpins, Pin>> {};

    // Define action to store parsed values

    template<typename Rule>
    struct action : pegtl::nothing<Rule> {};

    template<>
    struct action<Boundary> {
        template<typename Input>
        static void apply(const Input& in, std::pair<Steiner::Coordinate,Steiner::Coordinate> &_chip_boundary, std::size_t&, std::vector<Steiner::Pin>&) {
            std::stringstream ss(in.string());
            Steiner::Coordinate _left,_right;
            std::string temp1,temp2;
            int buf_len = ss.rdbuf()->in_avail();
            while(buf_len>1)
            {
                ss>>temp2;
                temp1+=temp2;
                buf_len = ss.rdbuf()->in_avail();
            }
            int critical_index;
            bool x_touch,y_touch;
            for(int i = temp1.length()-1;i>=0;--i)
            {
                if(temp1[i]==')')
                    critical_index = i;
                else if(temp1[i]==','&&temp1[i+1]!='(')
                {
                    if(!y_touch)
                    {
                    _right._y = std::stoi(temp1.substr(i+1,critical_index-(i+1)));
                    y_touch = true;
                    }
                    else
                    _left._y = std::stoi(temp1.substr(i+1,critical_index-(i+1)));
                    critical_index = i; 
                }
                else if(temp1[i]=='(')
                {
                    if(!x_touch)
                    {
                    _right._x = std::stoi(temp1.substr(i+1,critical_index-(i+1)));
                    x_touch = true;
                    }
                    else
                    _left._x = std::stoi(temp1.substr(i+1,critical_index-(i+1)));
                    critical_index = i;
                }
            }
            _chip_boundary = {_left,_right};
        }
    };

    template<>
    struct action<Numpins> {
        template<typename Input>
        static void apply(const Input& in, std::pair<Steiner::Coordinate,Steiner::Coordinate>&, std::size_t &numpins, std::vector<Steiner::Pin>&) {
            std::stringstream ss(in.string());
            std::string temp1,temp2;
            int buf_len = ss.rdbuf()->in_avail();
            while(buf_len>1)
            {
                ss>>temp2;
                temp1+=temp2;
                buf_len = ss.rdbuf()->in_avail();
            }
            for(int i = temp1.length()-1;i>=0;--i)
            {
                if(temp1[i]=='=')
                 {   
                    numpins = std::stoi(temp1.substr(i+1));
                    break;
                 }
            }
        }
    };

    template<>
    struct action<Pin> {
        template<typename Input>
        static void apply(const Input& in, std::pair<Steiner::Coordinate,Steiner::Coordinate>&, std::size_t&, std::vector<Steiner::Pin> &_pin) {
            Steiner::Pin pin;
            std::stringstream ss(in.string());
            std::string temp1,temp2;
            int buf_len = ss.rdbuf()->in_avail();
            while(buf_len>1)
            {
                ss>>temp2;
                temp1+=temp2;
                buf_len = ss.rdbuf()->in_avail();
            }
            int critical_index;
            for(int i = temp1.length()-1;i>=0;--i)
            {
                if(temp1[i]==')')
                    critical_index = i;
                else if(temp1[i]==','&&temp1[i+1]!='(')
                {
                    pin._coord._y = std::stoi(temp1.substr(i+1,critical_index-(i+1)));
                    critical_index = i; 
                }
                else if(temp1[i]=='(')
                {
                    pin._coord._x = std::stoi(temp1.substr(i+1,critical_index-(i+1)));
                    pin._name = temp1.substr(3,i-3);
                    break;
                }
            }
            _pin.push_back(pin);
        }
    };
} // namespace parser

void Steiner::Steiner::handle(const std::string& input, std::pair<Coordinate,Coordinate>& _chip_boundary, std::size_t& numpins, std::vector<Pin>& _pin)
    {
        pegtl::file_input in(input);
        pegtl::parse<parser::Grammar,parser::action>(in,_chip_boundary,numpins,_pin);
    }
