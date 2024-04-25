#include<iostream>
#include<fstream>
#include<chrono>
#include "floorplanner.hpp"

int main(int argc, char** argv)
{
    Floorplanner fp;
    std::fstream blockinput,netinput,output;

    if(argc==5)
    {
        fp.setalpha(std::stof(argv[1]));
        blockinput.open(argv[2],std::ios::in);
        netinput.open(argv[3],std::ios::in);
        output.open(argv[4],std::ios::out);

        if(!blockinput)
        {
            std::cerr<<"Error opening input.block file!\n";
            exit(1);
        }

        if(!netinput)
        {
            std::cerr<<"Error opening input.net file!\n";
            exit(1);
        }

        if(!output)
        {
            std::cerr<<"Error opening output file!\n";
            exit(1);
        }
        
    }
    else
    {
        std::cerr<<"Usage: ./floorplanner <alpha value> <input.block name> <input.net name> <output file name>\n";
    }

    fp.readfrom_block(blockinput);
    blockinput.close();
    fp.readfrom_nets(netinput);
    netinput.close();
    // time start
    auto start_time = std::chrono::high_resolution_clock::now();
    fp.setup_sequencepair();
    
    fp.solve();

    //time end
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    fp.writeto_output(output,duration);
    output.close();
}