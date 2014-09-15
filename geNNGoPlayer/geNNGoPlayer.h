#pragma once

#include "geNNBrain2.h"
#include "SpSimplePlayer.h"
#include "SpMoveGenerator.h"
#include "SgDebug.h"
#include "SgWrite.h"

//----------------------------------------------------------------------------

/** Plays move with highest score */
class GeNNMoveGenerator
    : public Sp1PlyMoveGenerator
{
public:
	GeNNMoveGenerator(GoBoard& board, geNN::Brain2* brain_in)
        : Sp1PlyMoveGenerator(board)
    {
	brain = *brain_in;
	brainInputs = vector<float> (board.Size() * board.Size());
	brainOutputs = vector<float> (board.Size() * board.Size());
	}

	GeNNMoveGenerator(GoBoard& board, const std::string& geNNPath)
        : Sp1PlyMoveGenerator(board)
    {
	brain = geNN::Brain2(geNNPath.c_str());
	brainInputs = vector<float> (board.Size() * board.Size());
	brainOutputs = vector<float> (board.Size() * board.Size());
	}
    //virtual int Score(SgPoint p);
	void GenerateMoves(SgEvaluatedMoves& eval, SgBlackWhite toPlay);

	int Evaluate();

protected:
	geNN::Brain2 brain;
	vector<float> brainInputs;
	vector<float> brainOutputs;
};

//----------------------------------------------------------------------------

/** Simple player using SpRandomMoveGenerator */
class GeNNGoPlayer
    : public SpSimplePlayer
{
public:
    GeNNGoPlayer(GoBoard& board, geNN::Brain2* brain_in)
        : SpSimplePlayer(board, new GeNNMoveGenerator(board, brain_in))
    {
	}

	  GeNNGoPlayer(GoBoard& board, const std::string& brain_in)
        : SpSimplePlayer(board, new GeNNMoveGenerator(board, brain_in))
    {
	}

    std::string Name() const
    {
        return "geNN";
    }
protected:
    bool UseFilter() const
    {
        return true;
    }
};

inline int FtoG (int fuegoPoint, int boardSize) {
	return (SgPointUtil::Row(fuegoPoint) - 1) + boardSize * (SgPointUtil::Col(fuegoPoint) - 1);
}

inline int GtoF (int geNNPoint, int boardSize) {
	return SgPointUtil::Pt(geNNPoint%boardSize +1, geNNPoint/boardSize + 1);
}
