#pragma once

#include "geNNEcosystem.h"
#include "geNNGoPlayer.h"
#include "boost/thread.hpp"
#include "boost/progress.hpp"
#include <string>

#define MAX_THREAD 50

namespace geNN{

class FuegoEcosystem: public Ecosystem {
public:
	FuegoEcosystem(const char* path, int maxCores=8);
	void SetReferenceOpponent(std::string ref);
	void ScoreContestants();
	int nRounds;
private:
	static const int g_boardSize=9;
	int maxThread;
protected:
	void GetDefaultBrainParameter(int* genes, int* neurons, int* inputs, int* outputs);
private:
	std::string referenceOpponent;
	float GetScore(boost::shared_ptr<GoPlayer> players[2], GoBoard& board);
	float GetScore(int black, int white);
	float GetScore(int black_id, std::string white_sp);
	static boost::shared_ptr<GoPlayer> CreatePlayer(const std::string& name, GoBoard& bd);
	void PlayGames(std::vector<std::pair<int,int> >* matchList, boost::progress_display* progress);
	boost::mutex matchListUpdate;
	boost::mutex scoreUpdate;
	boost::mutex boardConstruction;
	//boost::progress_display	progress;
};

}
