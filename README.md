fuegoia
=======

Implementation of a Genetic Neural Network applied to the game of go.

It uses largely fuego, a go game C++ library: http://sourceforge.net/projects/fuego/

Compile with Visual Studio. The projects not from fuego are:
- fuegoia, the main executable.
- geNN, the library implementing the neural network abstraction, the ecosystem and the input/output functions.
- geNNGoPlayer, the library implementing the go player compatible with fuego Library and using geNN library.
