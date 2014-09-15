//#include "geNN.h"
#include "geNNFuegoEcosystem.h"

#include "geNNBrain2.h"


#include <iostream>
#include <string>
//#include <fstream>

#include "boost/filesystem.hpp"
#include "boost/filesystem/fstream.hpp"
#include "boost/array.hpp"
#include "boost/foreach.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/program_options.hpp"
#include <boost/random.hpp>



#include <boost/config.hpp>
#include <boost/graph/subgraph.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>


using namespace std;
using namespace geNN;
using namespace boost;

int main2(int,char*[])
{
 filesystem::path p("C:\\fuegoia\\cincinnati\\Players\\player_v2_40");
 geNN::Brain2 test2(p);
 ofstream out("C:/Users/Ernest/Documents/out_graphviz.dot");
 test2.WriteGraph(out);
	return 0;
}


int main(int argc, char* argv[]){
	int maxCores;
	bool activateFtp;
	float metaMultiplier;
	string referenceOpponent;
	/*#ifdef _WIN32
	FreeConsole();
	#endif*/
	program_options::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("ftp", program_options::value<bool>(&activateFtp)->default_value(true),"FTP transfer activated")
		("basemuta", program_options::value<float>(&Brain2::BASE_MUTA)->default_value(float(0.001)),"Default mutation probability")
		("maxmuta", program_options::value<float>(&Brain2::MAX_MUTA)->default_value(float(0.01)),"Maximum mutation probability allowed")
		("minmuta", program_options::value<float>(&Brain2::MIN_MUTA)->default_value(float(10)/float(RAND_MAX)),"Minimum mutation probability allowed")
		("metamuta", program_options::value<float>(&metaMultiplier)->default_value(float(0.001)),"Mutation probability of every mutation probability")
		("reference_opponent", program_options::value<string>(&referenceOpponent)->default_value("random"),"Reference opponent can be average, capture, dumbtactic, greedy, influence, ladder, liberty, maxeye, minlib, random, safe, everyone")
		("max_core", program_options::value<int>(&maxCores)->default_value(8),"Maximum number of cores the program will use.")
		;
	program_options::variables_map vm;
	program_options::store(program_options::command_line_parser(argc, argv).options(desc).run(),vm);
	program_options::notify(vm);
	Brain2::SetMetaMuta(metaMultiplier);
	if (vm.count("help")) {
		cout << desc << '\n';
		return 0;
	}

	filesystem::path p(argv[0]);
	posix_time::ptime t0,t1;
	t0 = posix_time::second_clock::local_time();
	string version = p.filename().generic_string().substr(7,3);
	srand ((unsigned int) time(NULL) );
	FuegoEcosystem world(argv[0], maxCores);
	world.SetReferenceOpponent(referenceOpponent);
	string simplePlayersLibrary[11] = {"average", "capture", "dumbtactic", "greedy", "influence", "ladder", "liberty", "maxeye", "minlib", "random", "safe"} ;
	
	//world.CheckVersion(atoi(version.c_str()), argc, argv);

	int i=0;
	while(true) {
		cout<<"Starting tournament "<<i++<<".\n";
		if (referenceOpponent == "everyone")
			world.SetReferenceOpponent(simplePlayersLibrary[rand()%11]);
		world.ScoreContestants();
		if (activateFtp) {
		world.SendBestContestant();
		}
		world.Reproduction();
		world.Mutation();
		world.SaveAll();
		if (activateFtp) {
			world.GetRandomContestant();
		}
		/*t1 = posix_time::second_clock::local_time();		
		if ((t1-t0).hours() > 0)
			world.CheckVersion(atoi(version.c_str()),argc,argv);
		t0 = t1;*/
	}
	return 0;
}
