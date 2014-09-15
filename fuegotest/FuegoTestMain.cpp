//----------------------------------------------------------------------------
/** @file FuegoTestMain.cpp
    Main function for FuegoTest.
*/
//----------------------------------------------------------------------------

#define BOOST_LIB_DIAGNOSTIC

#include "SgSystem.h"

#include <iostream>
#include "FuegoTestEngine.h"
#include "GoInit.h"
#include "SgCmdLineOpt.h"
#include "SgDebug.h"
#include "SgException.h"
#include "SgInit.h"

using namespace std;

//----------------------------------------------------------------------------

namespace {

/** @name Settings from command line options */
// @{

bool g_quiet;

string g_config;

/** Player string as in FuegoTestEngine::SetPlayer */
string g_player;

/** Player geNN Path as in FuegoTestEngine::SetPlayer */
string g_geNNPath;

int g_boardsize;

const char* g_programPath;

// @} // @name

void MainLoop()
{
    GtpInputStream gtpIn(cin);
    GtpOutputStream gtpOut(cout);
    FuegoTestEngine engine(gtpIn, gtpOut, g_boardsize, g_programPath, g_player, g_geNNPath);
    GoGtpAssertionHandler assertionHandler(engine);
    if (g_config != "")
        engine.ExecuteFile(g_config);
    engine.MainLoop();
}

void ParseOptions(int argc, char** argv)
{
    SgCmdLineOpt opt;
    vector<string> specs;
    specs.push_back("config:");
    specs.push_back("help");
    specs.push_back("player:");
    specs.push_back("quiet");
    specs.push_back("srand:");
	specs.push_back("geNNPath:");
	specs.push_back("boardsize:");
    opt.Parse(argc, argv, specs);
    if (opt.GetArguments().size() > 0)
        throw SgException("No arguments allowed");
    if (opt.Contains("help"))
    {
        cout <<
            "Options:\n"
            "  -config file execute GTP commands from file before\n"
            "               starting main command loop\n"
            "  -help        display this help and exit\n"
            "  -player      player (average|capture|dumbtactic|greedy\n"
            "                |influence|ladder|liberty|maxeye|minlib\n"
            "                |no-search|random|safe|geNN)\n"
            "  -quiet       don't print debug messages\n"
            "  -srand       set random seed (-1:none, 0:time(0))\n"
			"  -geNNPath	path to geNN player to load"
			"  -boardsize	size of the board";
        exit(0);
    }
    g_config = opt.GetString("config", "");
    g_player = opt.GetString("player", "");
	g_geNNPath = opt.GetString("geNNPath", "");
    g_quiet = opt.Contains("quiet");
	g_boardsize = opt.GetInteger("boardsize", 19);
    if (opt.Contains("srand"))
        SgRandom::SetSeed(opt.GetInteger("srand"));
}

} // namespace

//----------------------------------------------------------------------------

int main(int argc, char** argv)
{
    if (argc > 0 && argv != 0)
    {
        g_programPath = argv[0];
        try
        {
            ParseOptions(argc, argv);
        }
        catch (const SgException& e)
        {
            SgDebug() << e.what() << "\n";
            return 1;
        }
    }
    if (g_quiet)
        SgDebugToNull();
    try
    {
        SgInit();
        GoInit();
        MainLoop();
        GoFini();
        SgFini();
    }
    catch (const GtpFailure& e)
    {
        SgDebug() << e.Response() << '\n';
        return 1;
    }
    catch (const std::exception& e)
    {
        SgDebug() << e.what() << '\n';
        return 1;
    }
    return 0;
}

//----------------------------------------------------------------------------

