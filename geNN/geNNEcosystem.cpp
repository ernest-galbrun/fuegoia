#include "geNNEcosystem.h"
#include "geNNBrain2.h"

#include "boost/foreach.hpp"
#include "boost/filesystem.hpp"
#include "boost/filesystem/fstream.hpp"
#include "boost/asio.hpp"
#include "boost/progress.hpp"
#include "boost/bind.hpp"

#include <iostream>
#include <sstream>
#include <string>

using namespace geNN;
using namespace std;
using namespace boost;


Ecosystem::Ecosystem(const char* exePath){
	cout << "Loading players.\n";
    progress_display progress(NCONTESTANTS);
	filesystem::path path(exePath);
	folder = path.parent_path();
	for (int i=0;i<NCONTESTANTS;i++) {
		++progress;
		stringstream name;
		name<<"player_v2_"<<i;
		filesystem::path filePath(folder/"Players"/name.str());
		if (filesystem::exists(filePath)) {
			try {
				contestants[i] = Brain2(filePath);
			}			
			catch (std::exception& e) {
				cout << e.what() << '\n';
				filesystem::remove(filePath);
			}	
			scores[i] = pair<float,int>((float)0,i);
		}
	}
}

void Ecosystem::Mutation() {
    cout<<"Mutation.\n";
    progress_display progress(NCONTESTANTS-10);
	//test : 10 first member dont mutate
	for (int i=min(10,(int)contestants.size()-1);i<(int)contestants.size();i++){
	    contestants[i].Mutate();
	    ++progress;
	}
}

bool Ecosystem::SameSpecies(int first, int second) {
	int ageFirst = contestants[first].Age();
	int ageSecond = contestants[second].Age();
	int testRange = max(ageFirst, ageSecond) - 100;
	if (testRange<=0) {
		return true;
	}
	if (min(ageFirst,ageSecond) < testRange) {
		return false;
	}
	int i=0;
	while (true) {
		if (contestants[first].origins[i].first > testRange)
			return (true);
		if (contestants[first].origins[i] != contestants[second].origins[i])
			return false;
		else {
			testRange -= contestants[first].origins[i].first;
			i++;
		}
	}
}


void Ecosystem::Reproduction() {
	// Sort the score vector, and let the best reproduce
	// the reset the score vector
	cout<<"Reproduction.\n";
	progress_display progress(NCONTESTANTS);
	sort(scores.begin(),scores.end());
	int firstPositive=0;
	cout << "Final score :\n";
	//for (int i=0;i<NCONTESTANTS;i++)		
	//	cout << "rank "<< NCONTESTANTS - i<<":\t"<<scores[i].second<<'\t'<<scores[i].first<<" points\n";
	for (int i=0;i<NCONTESTANTS;i++) {
        if (scores[i].first>=0) {
            firstPositive = i;
            break;
        }
	}
	// building a species tag for each contestant, determining wether they are close enough to each other so that
	// we try a sexual reproduction. Species are built to be associative, which is not true but simplify
	// greatly the code, without introducing much error.
	vector<int> species(NCONTESTANTS);
	int speciesNb = 1;
	for (int i=1;i<NCONTESTANTS;i++) {
		bool newSpecies = true;
		for (int j=0;j<i;j++) {
			if (SameSpecies(i,j)) {
				species[i] = species[j];
				newSpecies = false;
				break;
			}
		}
		if (newSpecies) {
			species[i] = speciesNb;
			speciesNb++;
		}
	}
	//cout << "Species:\n";
	//for (int i=0;i<NCONTESTANTS;i++)
	//	cout<<species[i]<<'\n';
	// building new Generation, every one in the top half get to be a mother,
	// the fathers are chosen randomly amongst the positive ranking players of the same species
	// if none is available, the player is just duplicated.
	boost::array<Brain2,NCONTESTANTS> newGeneration;
	for (int i=0;i<NCONTESTANTS/2;i++) {
		vector<int> fathersAvailable;
		for(int j=firstPositive;j<NCONTESTANTS;j++){
			if (species[scores[i+NCONTESTANTS/2].second] == species[scores[j].second])
				fathersAvailable.push_back(scores[j].second);
		}
		if (fathersAvailable.size()>0) {
			for (int j=0;j<2;j++) {
				int father = fathersAvailable[rand()%fathersAvailable.size()];
				//cout << scores[i+NCONTESTANTS/2].second << " mates with " <<father<<'\n';
				newGeneration[2*i+j] = Brain2(contestants[scores[i+NCONTESTANTS/2].second],
											 contestants[father]);
				++progress;
			}
		}
		else {
			for (int j=0;j<2;j++) {
				cout<<"duplication of player " << scores[i+NCONTESTANTS/2].second << '\n';
				newGeneration[2*i+j] = Brain2(contestants[scores[i+NCONTESTANTS/2].second]);
				++progress;
			}
		}
	}
	contestants = newGeneration;
	for (int i=0;i<NCONTESTANTS;i++) {
		scores[i] = pair<float,int>((float)0,i);
		contestants[i].AddOneGeneration(folder.filename().generic_string());
	}
}

void	Ecosystem::CheckVersion(int version, int argc, char* argv[]){
	filesystem::path newVersion(folder/"version");
	ReceiveFromFTP("version", newVersion);
	filesystem::ifstream is(newVersion);
	int nVersion;
	is >> nVersion;
	if (nVersion > version) {
		cout<<"New version available. Downloading fuegoia"<<nVersion<<'\n';
		char c[3];
		sprintf(c,"%03d",nVersion);
		string newFile = "fuegoia";
		newFile += c;
#ifdef _WIN32
		newFile += ".exe";
#endif
		filesystem::path p(folder/newFile);
		ReceiveFromFTP(newFile,p,true);
		string cmd(p.generic_string());
		for (int i=1;i<argc;i++){
		cmd+=' ';
		cmd+=argv[i];
		}
		::system(cmd.c_str());
		exit(0);
	}
}

int		Ecosystem::SendToFTP(boost::filesystem::path fileHere, string fileThere, bool binary){
        filesystem::path paramftp(folder/"cmdftp.txt");
        filesystem::ofstream ftp (paramftp); 
		#ifdef _WIN32
        ftp << "open\n" << "hephaestos.fr\n" << "u52181846-go\n" << "Pmdlpen1\n";
		if (binary)
			ftp << "binary\n";
        ftp << "put\n";
        ftp << fileHere/*.file_string()*/ <<'\n';
        ftp << fileThere << '\n';
        ftp << "bye";
        #else
        ftp << "open hephaestos.fr -u u52181846-go,Pmdlpen1\n";
        ftp << "put " <<fileHere.file_string() << binary?" ":" -a "<<"-o " << fileThere <<'\n';
        ftp << "quit";
        #endif  
        ftp.close();
        stringstream command;
        #ifdef _WIN32
		command<<"ftp -v -s:"<< paramftp.generic_string(); //for windows
        #else
        command<<"lftp -f "<< paramftp.file_string();// for linux
        #endif
        string test = command.str();
        cout<<command.str().c_str();
        return ::system(command.str().c_str());      
}

int		Ecosystem::ReceiveFromFTP(string fileThere, boost::filesystem::path fileHere, bool binary){
        filesystem::path paramftp(folder/"cmdftp.txt");
        filesystem::ofstream ftp (paramftp); 
		#ifdef _WIN32
        ftp << "open\n" << "hephaestos.fr\n" << "u52181846-go\n" << "Pmdlpen1\n";
		if (binary)
			ftp << "binary\n";
        ftp << "get\n";
        ftp << fileThere << '\n';
        ftp << fileHere.generic_string() <<'\n';
        ftp << "bye";
        #else
        ftp << "open hephaestos.fr -u u52181846-go,Pmdlpen1\n";
        ftp << "get " << fileThere << binary?" ":" -a "<<"-o "  << fileHere.file_string() << '\n';
        ftp << "quit";
        #endif        
        ftp.close();
        stringstream command;
        #ifdef _WIN32
        command<<"ftp -v -s:"<< paramftp.generic_string(); //for windows
        #else
        command<<"lftp -f "<< paramftp.file_string();// for linux
        #endif
        string test = command.str();
        cout<<command.str().c_str();
        return ::system(command.str().c_str());
}

void Ecosystem::GetRandomContestant() {
        stringstream     incomingLocal, incomingServer;
        int newLocal = rand()%NCONTESTANTS;
        incomingLocal << "player_v2_" << newLocal;
        incomingServer << "player_v2_" << rand()%1000;

        filesystem::path p = folder/"Players"/incomingLocal.str();
		ReceiveFromFTP(incomingServer.str(),p);

        if (filesystem::exists(p) &&  filesystem::file_size(p) != 0) {
			try {
				contestants[newLocal] = Brain2(p);
			}
			catch (std::exception& e) {
				cout << e.what() << '\n';
				contestants[newLocal].SaveAs(p);
			}	
		}
        else
            contestants[newLocal].SaveAs(p);
}

void Ecosystem::SendBestContestant() {
        stringstream     leavingLocal, leavingServer;
        leavingLocal << "player_v2_" << NCONTESTANTS - 1;
        leavingServer << "player_v2_" << (int) pow(float(10),float(3) * float(rand()) / float (RAND_MAX)) -1;
        filesystem::path p = folder/"Players"/leavingLocal.str();
		SendToFTP(p, leavingServer.str());
}

void Ecosystem::GetSendBestContestant() {
        stringstream     leavingLocal, incomingLocal, leavingServer, incomingServer;
        leavingLocal << "player_v2_" << NCONTESTANTS - 1;
        leavingServer << "player_v2_" << (int) pow(float(10),float(3) * float(rand()) / float (RAND_MAX)) -1;
        int newLocal = rand()%NCONTESTANTS;
        incomingLocal << "player_v2_" << newLocal;
        incomingServer << "player_v2_" << rand()%1000;

        filesystem::path p = folder/"Players"/leavingLocal.str();
		SendToFTP(p, leavingServer.str());
        p = folder/"Players"/incomingLocal.str();
		ReceiveFromFTP(incomingServer.str(),p);

        if (filesystem::exists(p) &&  filesystem::file_size(p) != 0) {
			try {
				contestants[newLocal] = Brain2(p);
			}
			catch (std::exception& e) {
				cout << e.what() << '\n';
				contestants[newLocal].SaveAs(p);
			}	
		}
        else
            contestants[newLocal].SaveAs(p);
}

void Ecosystem::SaveAll() {
	for (int i=0;i<NCONTESTANTS;i++) {
		stringstream name;
		name<<"player_v2_"<<i;
		filesystem::path filePath(folder/"Players"/name.str());
		contestants[i].SaveAs(filePath);
	}
}
