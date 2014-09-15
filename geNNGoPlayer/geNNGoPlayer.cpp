
#include "SgSystem.h"
#include "geNNGoPlayer.h"
#include "SpMoveGenerator.h"
#include "SgEvaluatedMoves.h"

#include "GoEyeUtil.h"

using namespace std;

//----------------------------------------------------------------------------

int GeNNMoveGenerator::Evaluate() {
//	brain.ClearNodes();
	for (int i=0;i<(int)brainInputs.size();i++)
		brainInputs[i] = 0;
	for (SgSetIterator it(m_board.All(m_board.ToPlay())); it; ++it)
		brainInputs[FtoG(*it, m_board.Size())] = 1;
	for (SgSetIterator it(m_board.All(m_board.Opponent())); it; ++it)
		brainInputs[FtoG(*it, m_board.Size())] = -1;
	brainOutputs = brain.Compute(brainInputs, 5);
    float sum = 0;
    for (size_t i=0;i<brainOutputs.size();++i) sum+=brainOutputs[i];
	return int(sum*100);
}


void GeNNMoveGenerator::GenerateMoves(SgEvaluatedMoves& eval, SgBlackWhite toPlay)
{
    GoRestoreToPlay restoreToPlay(m_board);
    m_board.SetToPlay(toPlay);
    GoRestoreSuicide restoreSuicide(m_board, false);
    for (SgSetIterator it(eval.Relevant()); it; ++it)
    {
        SgPoint p(*it);
		int score = EvaluateMove(p);
		if (score > max(numeric_limits<int>::min(),eval.BestValue()))
			eval.AddMove(p, score);
    }
}

//int GeNNMoveGenerator::Score(SgPoint p)
//{
//    SG_UNUSED(p);
//    // GeNNMoveGenerator uses whole-board move generation, 
//    // it does not work by scoring individual moves.
//    SG_ASSERT(false);
//    return INT_MIN; 
//}

//----------------------------------------------------------------------------
//
//void GeNNMoveGenerator::GenerateMoves(SgEvaluatedMoves& eval, SgBlackWhite toPlay){
//	GoRestoreToPlay restoreToPlay(m_board);
//    m_board.SetToPlay(toPlay);
//    GoRestoreSuicide restoreSuicide(m_board, false);
//	for (int i=0;i<(int)brainInputs.size();i++)
//		brainInputs[i] = 0;
//
//	for (SgSetIterator it(m_board.All(toPlay)); it; ++it)
//		brainInputs[FtoG(*it, m_board.Size())] = 1;
//	
//	for (SgSetIterator it(m_board.All(SgOppBW(toPlay))); it; ++it)
//		brainInputs[FtoG(*it, m_board.Size())] = -1;
//
//	brainOutputs = brain.Compute(&brainInputs, 1);
//
//    for (SgSetIterator it(eval.Relevant()); it; ++it)
//    {
//        SgPoint p(*it);
//		if (!GoEyeUtil::IsSinglePointEye(m_board, p, m_board.ToPlay()))
//			eval.AddMove(p, (int)brainOutputs[FtoG(p, m_board.Size())]);
//	}
//}
//    
