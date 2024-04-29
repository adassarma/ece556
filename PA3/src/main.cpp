#include<iostream>
#include<fstream>
#include "steiner.hpp"

#include <tao/pegtl.hpp>
#include <chrono>

int main(int argc,char**argv)
{
	if(argc!=3)
	{
	    std::cerr<<"Usage error!\n";
	    exit(-1);
	}



	std::fstream input, output;

	input.open(argv[1],std::ios::in);
	output.open(argv[2],std::ios::out);

	if(!input)
	{
		std::cerr<<"Error opening input file!\n";
		exit(1);
	}


	if(!output)
	{
		std::cerr<<"Error opening output file!\n";
		exit(1);
	}

	auto start = std::chrono::high_resolution_clock::now();
	Steiner::Steiner steiner(argv[1]);
	steiner.solve();
	auto end = std::chrono::high_resolution_clock::now();
	std::cout<<"Runtime: "<<std::chrono::duration_cast<std::chrono::microseconds>(end-start).count()<<"\n";
	steiner.writeToOutput(output);

}		
