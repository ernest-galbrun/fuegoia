#include "geNNFuegoEcosystem.h"
#include "GoBoard.h"
#include "GoBoardUtil.h"
#include "GoInit.h"
#include "SgInit.h"
#include "SgTimeRecord.h"
#include "GoRules.h"
#include "boost/filesystem.hpp"
#include "boost/filesystem/fstream.hpp"
#include "boost/thread.hpp"
#include "boost/foreach.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/timer.hpp"
#include "boost/progress.hpp"


#include "SpAveragePlayer.h"
#include "SpCapturePlayer.h"
#include "SpDumbTacticalPlayer.h"
#include "SpGreedyPlayer.h"
#include "SpInfluencePlayer.h"
#include "SpLadderPlayer.h"
#include "SpLibertyPlayer.h"
#include "SpMaxEyePlayer.h"
#include "SpMinLibPlayer.h"
#include "SpRandomPlayer.h"
#include "SpSafePlayer.h"

#include <sstream>
#include <string>
#include <algorithm>
#include <vector>

using namespace std;
using namespace geNN;
using namespace boost;

//const int FuegoEcosystem::g_maxThread;
//extern int FuegoEcosystem::g_maxThread;


FuegoEcosystem::FuegoEcosystem(const char* exePath, int maxCores) : Ecosystem(exePath),
	referenceOpponent("random"),
	maxThread(maxCores)
{
	SgInit();
    GoInit();
	filesystem::path path(exePath);
	filesystem::path folder = path.parent_path();
	for (int i=0;i<NCONTESTANTS;i++) {
		stringstream name;
		name<<"player_v2_"<<i;
		filesystem::path filePath(folder/"Players"/name.str());
		if (!filesystem::exists(filePath)) {// brain was not created : create a new random one
			int genes, neurons, inputs, outputs;
			GetDefaultBrainParameter(&genes, &neurons, &inputs, &outputs);
			contestants[i] = Brain2(genes, neurons, inputs, outputs);
			filesystem::create_directory(filePath.parent_path());
			contestants[i].SaveAs(filePath);
		}
	}
}

void FuegoEcosystem::SetReferenceOpponent(std::string ref){
	referenceOpponent = ref;
}

void FuegoEcosystem::GetDefaultBrainParameter(int* genes, int* neurons, int* inputs, int* outputs){
	*genes = 6;//rand()%15+1;
	*neurons = 2 * g_boardSize * g_boardSize;//rand()%15+1;
	*inputs = g_boardSize * g_boardSize;
	*outputs = *inputs;
}

void FuegoEcosystem::ScoreContestants(){
	vector<pair<int,int> > matchListPreparation;
	thread threadList[MAX_THREAD];
	int n_thread = min(FuegoEcosystem::maxThread,(int)thread::hardware_concurrency());
	n_thread = min(n_thread, MAX_THREAD);

	// Play against reference opponent
	for (int i=0;i<NCONTESTANTS;i++)
        matchListPreparation.push_back(pair<int,int>(i,-1));
    cout<<"Starting preparation matchs\n";
	progress_display progressPreparation(NCONTESTANTS);
	for (int i=0;i<n_thread;i++) {
		threadList[i] = thread(&geNN::FuegoEcosystem::PlayGames, this, &matchListPreparation, &progressPreparation);
	}
	for (int i=0;i<n_thread;i++){
		threadList[i].join();
	}

	//count the losers
	int nLosers = 0;
    for (int i=0;i<(int)scores.size();i++)
        nLosers += scores[i].first<0?1:0;
	if (nLosers == 0)
        cout<<"We only have winners !\n";
    else {
        nLosers = (float)nLosers * 100 / NCONTESTANTS;
		cout<<nLosers<<" % of losers against "<< referenceOpponent<< '\n';
    }
    boost::filesystem::ofstream log(folder/"log", ios::app);
    log<< referenceOpponent << '\t' << nLosers<<'\n';
    log.close();
	if (nLosers==100)
		nRounds = 5;
    else if (nLosers>70)
        nRounds = 0;
    else if (nLosers>50)
        nRounds = 1;
    else if (nLosers>30)
        nRounds = 2;
    else if (nLosers>20)
        nRounds = 3;
    else if (nLosers>10)
        nRounds = 4;
    else
        nRounds = 5;

    cout<<"Starting tournament with "<<nRounds<<" rounds between goia players and "<<5-nRounds<<" against reference opponent.\n";
	progress_display progress(nRounds * NCONTESTANTS);
	// Create matchList
	vector<pair<int,int> > matchList;
	for (int i=0;i<nRounds;i++) {
		vector<int> players_id(NCONTESTANTS);
		for (int j=0;j<NCONTESTANTS;j++)
			players_id[j] = j;
		random_shuffle(players_id.begin(),players_id.end());
		for (int j=0;j<NCONTESTANTS;j++) {
			matchList.push_back(pair<int,int>(players_id[j],players_id[(j+1)%NCONTESTANTS]));
		}
	}
	
	//Launch games
	for (int i=0;i<n_thread;i++) {
		threadList[i] = thread(&geNN::FuegoEcosystem::PlayGames, this, &matchList, &progress);
	}
	for (int i=0;i<n_thread;i++){
		threadList[i].join();
	}
	
	vector<pair<int,int> > matchListReference;
	for (int i=nRounds+1;i<5;i++) {
		for (int i=0;i<NCONTESTANTS;i++)
			matchListReference.push_back(pair<int,int>(i,-1));
	}
	progress_display progressReference(NCONTESTANTS*(5-nRounds));
	//Launch games
	for (int i=0;i<n_thread;i++) {
		threadList[i] = thread(&geNN::FuegoEcosystem::PlayGames, this, &matchListReference, &progressReference);
	}
	for (int i=0;i<n_thread;i++){
		threadList[i].join();
	}
}

void FuegoEcosystem::PlayGames(vector<pair<int,int> >* matchList, progress_display* progress){
	pair<int,int> match;
	while(true) {
		matchListUpdate.lock();
		if (matchList->empty()) {
			matchListUpdate.unlock();
			break;
		}
		match = matchList->back();
		matchList->pop_back();
		matchListUpdate.unlock();
		float score;
		if (match.second == -1) {
            score = GetScore(match.first, referenceOpponent);
        //    score = score>0?0:-1000;
		}
        else {
            score = GetScore(match.first, match.second);
          //  score = score>0?(float)3:(float)-3;
        }
		score = score>0?(float)3:(float)-3;
		scoreUpdate.lock();
		++(*progress);
		scores[match.first].first+=score;
		if (match.second!=-1)
            scores[match.second].first-=score;
		scoreUpdate.unlock();
	}
}

boost::shared_ptr<GoPlayer> FuegoEcosystem::CreatePlayer(const string& playerId, GoBoard& bd)
{
    if (playerId == "")
        return boost::shared_ptr<GoPlayer>();
    if (playerId == "average")
        return boost::shared_ptr<GoPlayer>(new SpAveragePlayer(bd));
    if (playerId == "capture")
        return boost::shared_ptr<GoPlayer>(new SpCapturePlayer(bd));
    if (playerId == "dumbtactic")
        return boost::shared_ptr<GoPlayer>(new SpDumbTacticalPlayer(bd));
    if (playerId == "greedy")
        return boost::shared_ptr<GoPlayer>(new SpGreedyPlayer(bd));
    if (playerId == "influence")
        return boost::shared_ptr<GoPlayer>(new SpInfluencePlayer(bd));
    if (playerId == "ladder")
        return boost::shared_ptr<GoPlayer>(new SpLadderPlayer(bd));
    if (playerId == "liberty")
        return boost::shared_ptr<GoPlayer>(new SpLibertyPlayer(bd));
    if (playerId == "maxeye")
        return boost::shared_ptr<GoPlayer>(new SpMaxEyePlayer(bd, true));
    if (playerId == "minlib")
        return boost::shared_ptr<GoPlayer>(new SpMinLibPlayer(bd));
    if (playerId == "random")
        return boost::shared_ptr<GoPlayer>(new SpRandomPlayer(bd));
    if (playerId == "safe")
        return boost::shared_ptr<GoPlayer>(new SpSafePlayer(bd));
    throw SgException("unknown player " + playerId);
}

float FuegoEcosystem::GetScore(boost::shared_ptr<GoPlayer> players[2], GoBoard& board){
	SgTimeRecord time;
    SgPoint move = SG_NULLMOVE;
	int pass = 0;
	while (pass<2) {
		move = players[board.ToPlay()]->GenMove(time, board.ToPlay());
		// we have to update the three actual boards
		players[SG_BLACK]->Board().Play(move);
		players[SG_WHITE]->Board().Play(move);
		board.Play(move);
		if (move == SG_PASS) {
			pass++;
		}
		else {
			pass = 0;
		}
	}
	return GoBoardUtil::TrompTaylorScore(board, board.Rules().Komi().ToFloat());
}

float FuegoEcosystem::GetScore(int black_id, string white_sp) {
	// GoBoard construction is not thread safe, and a board is created
	// for the game, and one per player.
	/*====================================================================*/
	/*                          Protected access                          */
	/*====================================================================*/
	boardConstruction.lock();
	GoBoard board_real(g_boardSize);
	board_real.Rules().SetNamedRules("cgos");
	GoBoard& board = board_real;
	boost::shared_ptr<GoPlayer> players[2];
	Brain2 blackBrain(contestants[black_id]);
	players[SG_BLACK] = boost::shared_ptr<GoPlayer>(new GeNNGoPlayer(board,&blackBrain));
	players[SG_WHITE] = CreatePlayer(white_sp, board);
	boardConstruction.unlock();
	/*====================================================================*/
	/*                          \Protected access                         */
	/*====================================================================*/
	return GetScore(players, board);
}


float FuegoEcosystem::GetScore(int black_id, int white_id){
	// GoBoard construction is not thread safe, and a board is created
	// for the game, and one per player.
	/*====================================================================*/
	/*                          Protected access                          */
	/*====================================================================*/
	boardConstruction.lock();
	GoBoard board_real(g_boardSize);
	board_real.Rules().SetNamedRules("cgos");
	GoBoard& board = board_real;
	boost::shared_ptr<GoPlayer> players[2];
	Brain2 blackBrain(contestants[black_id]);
	Brain2 whiteBrain(contestants[white_id]);
	players[SG_BLACK] = boost::shared_ptr<GoPlayer>(new GeNNGoPlayer(board,&blackBrain));
	players[SG_WHITE] = boost::shared_ptr<GoPlayer>(new GeNNGoPlayer(board,&whiteBrain));
	boardConstruction.unlock();
	/*====================================================================*/
	/*                          \Protected access                         */
	/*====================================================================*/
	return GetScore(players, board);
}
