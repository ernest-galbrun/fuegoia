//----------------------------------------------------------------------------
/** @file GoUtil.cpp
    See GoUtil.h
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "GoUtil.h"

#include <cmath>
#include <iomanip>
#include <sstream>

using namespace std;

//----------------------------------------------------------------------------

std::string GoUtil::ScoreToString(float score)
{
    bool blackWin = (score > 0);
    score = fabs(score);
    const float epsilon = float(0.01);
    if (score < epsilon)
        return "0";
    ostringstream out;
    bool isFractional = (fabs(static_cast<int>(score) - score) > epsilon);
    int precision = (isFractional ? 1 : 0);
    out << (blackWin ? "B+" : "W+") << fixed << setprecision(precision)
        << score;
    return out.str();
}

//----------------------------------------------------------------------------
