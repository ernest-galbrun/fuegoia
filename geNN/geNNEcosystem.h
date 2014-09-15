#pragma once

#include "boost/array.hpp"
#include "boost/filesystem.hpp"
#include "geNNBrain2.h"

#define NCONTESTANTS 100


namespace geNN {

class Ecosystem {


protected:
	boost::array<std::pair<float,int>,NCONTESTANTS>		scores;
	boost::array<Brain2,NCONTESTANTS>			contestants;
protected:
	virtual void GetDefaultBrainParameter(int* genes, int* neurons, int* inputs, int* outputs)=0;
	boost::filesystem::path folder;
	void handle_writeDUMMY(const boost::system::error_code& /*error*/,
      size_t /*bytes_transferred*/){}
public:
	Ecosystem(const char* exePath);
	Ecosystem(){}
	virtual void ScoreContestants() =0;
	void	CheckVersion(int version, int argc, char* argv[]);
	void	Reproduction();
	void	Mutation();
	// Send the best contestant to the tcp server
	void	GetSendBestContestant();
	void	GetRandomContestant();
	void	SendBestContestant();
	void    SaveAll();
private:
	int		SendToFTP(boost::filesystem::path fileHere, std::string fileThere, bool binary=false);
	int		ReceiveFromFTP(std::string fileThere, boost::filesystem::path fileHere, bool binary=false);
	bool	SameSpecies(int first, int second);
};
}
