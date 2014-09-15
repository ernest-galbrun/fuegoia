#pragma once
#include "boost/filesystem.hpp"
#include <boost/serialization/utility.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/random.hpp>
#include <boost\graph\filtered_graph.hpp>

#include <set>
#include <queue>
#include <vector>
#include <utility>
#include <limits>
#include <iostream>


namespace geNN {

/* a Brain is made of at least 1 Gene*/
class Brain2 {
public:
static float BASE_MUTA;
static float MAX_MUTA;
static float MIN_MUTA;
static int META_MUTA;

private:
 
friend class boost::serialization::access;

static int seed;

enum	eneuronType {THRESHOLD, LINEAR, SIGMOID, PW_LINEAR};



struct Neuron{
friend class boost::serialization::access;
std::pair<unsigned int,float> subgraph;
std::pair<eneuronType,float> type;
float value;
int visits;
std::pair<float,float>	threshold;
std::pair<float,float>	coef;
float			duplicateOrDisappear;
//float			migrate;
template<class Archive>
 void serialize(Archive & ar, const unsigned int version)  {
  ar & subgraph;
  ar & type;
  ar & threshold;
  ar & coef;
  ar & duplicateOrDisappear;
  //ar & migrate;		
 };
};

struct Axon{
	friend class boost::serialization::access;
	std::pair<float,float> strength;
    float duplicateOrDisappear;
	template<class Archive>
    void serialize(Archive & ar, const unsigned int version)  {
        ar & strength;		
        ar & duplicateOrDisappear;
    };
};


typedef boost::adjacency_list<boost::listS,boost::listS,boost::bidirectionalS,boost::property<boost::vertex_index_t, std::size_t, Neuron>,boost::property<boost::edge_index_t, std::size_t, Axon> > Graph;
							  //boost::property<boost::vertex_index_t, int, Neuron>,
                             // boost::property<boost::edge_index_t, unsigned int, Axon> > Graph;
typedef boost::property_map<Graph, std::pair<unsigned int,float> Neuron::*>::type VertexSubgraphMap;

struct vertex_subgraph {
  vertex_subgraph() { }
  vertex_subgraph(const unsigned int sg, VertexSubgraphMap vsm) : index(sg), m_subgraph(vsm){ }
  template <typename Vertex>
  bool operator()(const Vertex& v) const {
    return index == get(m_subgraph, v).first;
  }
  unsigned int index;
  VertexSubgraphMap m_subgraph;
};

//
//
//class VertexWriter{
//private:
// Graph* g;
//public:
// VertexWriter(Graph* g_in) : g(g_in){};
// void operator() (std::ostream& out, const Graph::vertex_descriptor vd);
//};

typedef boost::filtered_graph<Graph,boost::keep_all,vertex_subgraph> fGraph;

private:
    boost::random::mt11213b randomGenerator;
	
	Graph g;
	//std::vector<boost::graph_traits<Graph>::vertex_descriptor> inputs;
	//std::vector<boost::graph_traits<Graph>::vertex_descriptor> outputs;
    std::set<unsigned int> subgraph_indices; //index 0 is for inputs, 1 for outputs, the rest for the remaining "genes"
	Neuron RandomNeuron(unsigned int sg);
	Neuron RandomNeuron();
	Axon RandomAxon();
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version);
    void MutatePair(std::pair<float,float>& in);
    void MetaMutate(float& in);
    void MutatePair(std::pair<eneuronType,float>& in);    
    void MutatePair(std::pair<unsigned int,float>& in);
    void ClearNodes();
    void UpdateValue(const boost::graph_traits<Graph>::vertex_descriptor & vd, std::queue<Graph::vertex_descriptor>& q);
    void UpdateIndices();
   
public:
    void WriteGraph(std::ostream& out);
    static void SetMetaMuta(float mm);
	std::vector<std::pair<int,std::string> >  origins;
	Brain2(const Brain2& b);
	Brain2(int geneNumber,int neuronNumber,int inputNumber,int outputNumber);
	Brain2(const Brain2& daddy,const Brain2& mummy);
	Brain2(const boost::filesystem::path& filePath);
	Brain2();
	void Mutate();
	void SaveAs(const boost::filesystem::path& filePath);
	std::vector<float> Compute(const std::vector<float>& in,const int& calculationNumber);
	void AddOneGeneration(std::string currentPlace); //add one generation to the origins vector
	int	 Age();
};
}