#include "boost/asio.hpp"
#include "boost/filesystem.hpp"
#include <iostream>

using namespace geNN;
using namespace boost;

int main(int argc, char* argv[]){
	srand ((unsigned int) time(NULL) );
while(true) {
 try
  {
    asio::io_service io_service;
    filesystem::path poolFolder(argv[0]);
    poolFolder = poolFolder.parent_path()/"pool";
    UdpServer server(io_service, poolFolder);
    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
}
return 0;
}
