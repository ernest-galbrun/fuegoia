#include "geNNBrain2.h"

#include <fstream>
#include <cmath>
#include <algorithm>
#include <queue>
#include <iostream>
#include <map>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/version.hpp>
#include <boost/graph/random.hpp>
#include <boost\graph\filtered_graph.hpp>
#include <boost/graph/copy.hpp>
#include <boost/graph/adj_list_serialize.hpp>
#include <boost\graph\graphml.hpp>
#include <boost\graph\graphviz.hpp>

using namespace std;
using namespace geNN;
using namespace boost;

//random::mt11213b Brain2::randomGenerator;// = random::mt11213b;
float	Brain2::BASE_MUTA;
float	Brain2::MAX_MUTA;
float	Brain2::MIN_MUTA;
int		Brain2::META_MUTA;
int Brain2::seed;


Brain2::Brain2(){
	//neurons = Graph(2);
}

Brain2::Brain2(const Brain2& b) :
subgraph_indices(b.subgraph_indices),
g(b.g),
origins(b.origins){
 ++seed;
 randomGenerator = random::mt11213b(seed);
}

Brain2::Brain2(int geneNumber,int neuronNumber, int inputNumber, int outputNumber){
 ++seed;
 randomGenerator = random::mt11213b(seed);
 const int K = neuronNumber; //number of neurons per gene
 graph_traits<Graph>::vertex_descriptor v;
 graph_traits<Graph>::vertex_iterator vi,vi_end;
 graph_traits<Graph>::edge_iterator ei,ei_end;
 subgraph_indices.insert(0);
 for (int i=0;i<inputNumber;++i) {
	 v = add_vertex(g);
     g[v] = RandomNeuron(0);
 }
 subgraph_indices.insert(1);
 for (int i=0;i<outputNumber;++i) {
	 v = add_vertex(g);
     g[v] = RandomNeuron(1);
 }
 //subg = vector<Graph>(geneNumber);
 for (int i=0;i<geneNumber;++i){
  int n=randomGenerator();
  while(subgraph_indices.find(n)!=subgraph_indices.end())
   n=randomGenerator();
  subgraph_indices.insert(n);
  for (int j=0;j<K;++j){
   v = add_vertex(g);
   g[v]=RandomNeuron(n);
  }
 }
 generate_random_graph(g,0,num_vertices(g)*5,randomGenerator); // add random edges
 for (boost::tie(ei,ei_end) = edges(g);ei!=ei_end;++ei) {
	 g[*ei] = RandomAxon();
 }
 //Graph test = g;
}

Brain2::Brain2(const Brain2& daddy,const Brain2& mummy) : 
subgraph_indices(mummy.subgraph_indices),
g(mummy.g),
origins(mummy.origins)
{
 ++seed;
 randomGenerator = random::mt11213b(seed);
 set<unsigned int>::iterator it;
 for(it = subgraph_indices.begin(); it != subgraph_indices.end(); ++it) {
  if (daddy.subgraph_indices.find(*it)!=daddy.subgraph_indices.end()) {
   fGraph fg(g,keep_all(),vertex_subgraph(*it,get(&Neuron::subgraph,g)));
   fGraph::edge_iterator ei,ei_end;
   fGraph::vertex_iterator vi,vi2,vi_end;
   fGraph dfg(daddy.g,keep_all(),vertex_subgraph(*it,get(&Neuron::subgraph,const_cast<Brain2&>(daddy).g)));
   fGraph::edge_iterator dei,dei_end;
   fGraph::vertex_iterator dvi,dvi2,dvi_end;
   boost::tie(ei,ei_end) = edges(fg);
   boost::tie(vi,vi_end) = vertices(fg);
   boost::tie(dvi,dvi_end) = vertices(dfg);
   for (;vi!=vi_end;++vi){
    dvi2 = find(dvi,dvi_end,*vi);
    //replace neuron if it exists in daddys brain
    if (dvi2==dvi_end) continue;
    g[*vi] = daddy.g[*dvi2];
    //erase all in_edges then all out_edge
    fGraph::in_edge_iterator iei,iei_end,next;
    boost::tie(iei,iei_end) = in_edges(*vi,fg);
    for (next = iei;iei!=iei_end;iei=next) {
     ++next;
     remove_edge(*iei,g);
    }
    fGraph::out_edge_iterator oei,oei_end,onext;
    boost::tie(oei,oei_end) = out_edges(*vi,fg);
    for (onext = oei;oei!=oei_end;oei=onext) {
     ++onext;
     remove_edge(*oei,g);
    }
    //if able put daddy's in_ then out_ edges
    for (boost::tie(iei,iei_end) = in_edges(*vi,dfg);iei!=iei_end;++iei){
     vi2 = find(vi,vi_end,iei->m_source);
     if (vi2==vi_end) continue;
     fGraph::edge_descriptor newedge;//lol
     bool ok;
     boost::tie(newedge,ok) = add_edge(iei->m_source,iei->m_target,g);
     g[newedge] = daddy.g[*iei];
    }    
    for (boost::tie(oei,oei_end) = out_edges(*vi,dfg);oei!=oei_end;++oei){
     vi2 = find(vi,vi_end,oei->m_target);
     if (vi2==vi_end) continue;
     fGraph::edge_descriptor newedge;//lol
     bool ok;
     boost::tie(newedge,ok) = add_edge(oei->m_source,oei->m_target,g);
     g[newedge] = daddy.g[*oei];
    }
   }
  }
 }
}

Brain2::Neuron Brain2::RandomNeuron(){
 set<unsigned int>::iterator it = subgraph_indices.begin();
 for (int i=0;i<random::uniform_int_distribution<>(2,subgraph_indices.size()-1)(randomGenerator);++i)
  ++it;
 return RandomNeuron(*it);
}

Brain2::Neuron Brain2::RandomNeuron(unsigned int sg){
	Neuron A;
    A.subgraph=pair<unsigned int,float>(sg,BASE_MUTA);
	A.type = pair<eneuronType,float>(eneuronType(random::uniform_int_distribution<>(0,3)(randomGenerator)),BASE_MUTA);
	A.coef = pair<float,float>(float(randomGenerator())/randomGenerator.max(), BASE_MUTA);
	A.threshold = pair<float,float>(float(randomGenerator())/randomGenerator.max() * 2 - 1, BASE_MUTA);
	A.duplicateOrDisappear = BASE_MUTA;
//	A.migrate = BASE_MUTA;
	return A;
}

Brain2::Axon Brain2::RandomAxon(){
	Axon A;
	A.strength = pair<float,float>(float(randomGenerator())/randomGenerator.max() * 2 - 1,BASE_MUTA);
    A.duplicateOrDisappear = BASE_MUTA;
	return A;
}

void	Brain2::SaveAs(const boost::filesystem::path& filePath){
	ofstream ofs(filePath.generic_string());
	archive::text_oarchive oa(ofs);
	oa << *this;
}

Brain2::Brain2(const boost::filesystem::path& filePath){
	ifstream ifs(filePath.generic_string());
	archive::text_iarchive ia(ifs);
	ia >> (*this);
}


BOOST_CLASS_VERSION(Brain2,2)

template<class Archive>
void Brain2::serialize(Archive & ar, const unsigned int version) {
    if (version>1)
    ar & g;
    else {

     typedef adjacency_list<listS,listS,boost::bidirectionalS,Neuron,Axon > OldGraph;
     OldGraph og;
     ar & og;
     OldGraph::vertex_iterator vi,vi_end;
     //OldGraph::vertex_descriptor ovd1,ovd2;
     Graph::vertex_descriptor vd;
     OldGraph::edge_iterator ei,ei_end;
     OldGraph::edge_descriptor ed;
     map<OldGraph::vertex_descriptor,Graph::vertex_descriptor> m;
     bool ok;
     for (boost::tie(vi,vi_end)=vertices(og);vi!=vi_end;++vi){
      vd = add_vertex(g);
      g[vd] = og[*vi];
      m[*vi] = vd;
     }
     for (boost::tie(ei,ei_end)=edges(og);ei!=ei_end;++ei){
      boost::tie(ed,ok) = add_edge(m[source(*ei,og)],m[target(*ei,og)],g);
      g[ed] = og[*ei];
     }
    }
    ar & subgraph_indices;
    if (version>0)
     ar & origins;
}


void Brain2::MutatePair(std::pair<float,float>& in){
 if (float(randomGenerator())/randomGenerator.max() < in.second){
  float	coef;
  coef = pow(float(2),float(randomGenerator())/randomGenerator.max() * 2 -1); //get random number between 0.5 and 2
	if (random::uniform_int_distribution<>(1,10)(randomGenerator)==10)
		coef*=-1;
	in.first *= coef;
 }
  MetaMutate(in.second);
}

void Brain2::SetMetaMuta(float mm){
  boost::random::mt11213b rg;
  META_MUTA = int(mm*rg.max() );
}

void Brain2::MetaMutate(float& in) {
 float	coef;
 if (randomGenerator()< META_MUTA){
  coef = pow(float(2),float(randomGenerator())/randomGenerator.max() * 2 -1);  //get random number between 0.5 and 2
  float newValue = in * coef;
  if (newValue<MAX_MUTA && newValue>MIN_MUTA){
   in = newValue;
  }
 }
}

void Brain2::MutatePair(std::pair<eneuronType,float>& in) { 
 if (float(randomGenerator())/randomGenerator.max() < in.second){
  in.first = (eneuronType)random::uniform_int_distribution<>(0,3)(randomGenerator);
 }
 MetaMutate(in.second);
}

void Brain2::MutatePair(std::pair<unsigned int,float>& in){
 if (float(randomGenerator())/randomGenerator.max() < in.second){
  auto it = subgraph_indices.begin();
  size_t N = random::uniform_int_distribution<>(2,subgraph_indices.size()-1)(randomGenerator);
  for (size_t n=0;n<N;++n) ++it;
  in.first = *it;
 }
 MetaMutate(in.second);
}


void Brain2::Mutate(){
 //mutation of every edge
 Graph::edge_iterator ei, ei_end;
 for (boost::tie(ei,ei_end) = edges(g);ei!=ei_end;){
  Graph::edge_descriptor ed = *ei;
  ++ei;
  MutatePair(g[ed].strength);
  if (float(randomGenerator())/randomGenerator.max() < g[ed].duplicateOrDisappear){
   if (random::uniform_int_distribution<>(0,1)(randomGenerator)==0) { // add one random edge with a common vertex
    Graph::edge_descriptor new_e;
    bool ok;
    if(random::uniform_int_distribution<>(0,1)(randomGenerator)==0){ //keep source
     boost::tie(new_e,ok) = add_edge(source(ed,g),random_vertex(g,randomGenerator),g);
    } else{ //keep target
     boost::tie(new_e,ok) = add_edge(random_vertex(g,randomGenerator),target(ed,g),g);
    }    
    if (ok)
     g[new_e] = RandomAxon();
   }
   else{ // delete the edge
    remove_edge(ed,g);    
   }
  }
 }
 //mutation of every vertex
 Graph::vertex_iterator vi,vi_end;
 bool do_once=true;
 for(boost::tie(vi,vi_end)=vertices(g);vi!=vi_end;) {
  Graph::vertex_descriptor vd = *vi;
  ++vi;
  MutatePair(g[vd].coef);
  MutatePair(g[vd].threshold);
  MutatePair(g[vd].type);
  if(g[vd].subgraph.first != 0 && g[vd].subgraph.first != 1) {
   MutatePair(g[vd].subgraph);
   if (float(randomGenerator())/randomGenerator.max() < g[vd].duplicateOrDisappear || (num_vertices(g) < 81*5 && do_once)){
    do_once=false;
    if (random::uniform_int_distribution<>(0,1)(randomGenerator)==0 || num_vertices(g) < 81*5) { // add one random vertex with 2 in and out edges
     Graph::vertex_descriptor nvd = add_vertex(g);
     g[nvd] = RandomNeuron();
     Graph::edge_descriptor new_e;
     bool ok;
     for (int i=0;i<2;++i){
      boost::tie(new_e,ok) = add_edge(nvd,random_vertex(g,randomGenerator),g);
      g[new_e] = RandomAxon();
     }
     for (int i=0;i<2;++i){
      boost::tie(new_e,ok) = add_edge(random_vertex(g,randomGenerator),nvd,g);
      g[new_e] = RandomAxon();
     }
    } else {
     clear_vertex(vd,g);
     remove_vertex(vd,g);
    }
   }
  }
 }
 //Mutation at the subgraph level
 set<unsigned int>::iterator it = subgraph_indices.begin();
 ++it;
 ++it;
 for(;it!=subgraph_indices.end();){
  int count = 0;
  Graph::vertex_iterator vi,vi_end;
  for(boost::tie(vi,vi_end)=vertices(g);vi!=vi_end;++vi){
   if (g[*vi].subgraph.first == *it) ++count;
  }
  if (count<5 && subgraph_indices.size()>3){ // merge neurons in other subgraphes
   int k = *it;
   set<unsigned int>::iterator it2 = it;
   ++it;
   subgraph_indices.erase(it2); 
   it2 = subgraph_indices.begin();
   size_t N = random::uniform_int_distribution<>(2,subgraph_indices.size()-1)(randomGenerator);
   for (size_t n=0;n<N;++n) ++it2;
   for(boost::tie(vi,vi_end)=vertices(g);vi!=vi_end;++vi){
    if (g[*vi].subgraph.first == k) g[*vi].subgraph.first = *it2;
   }
  } else if (count>100) {//split subgraph in two
   int k = *it;
   int x=0;
   ++it;
   int n=randomGenerator();
   while(subgraph_indices.find(n)!=subgraph_indices.end())
    n=randomGenerator();
   subgraph_indices.insert(n);
   for(boost::tie(vi,vi_end)=vertices(g);vi!=vi_end && x<count/2;++vi){
    if (g[*vi].subgraph.first == k) {
     g[*vi].subgraph.first = n;
     ++x;
    }
   }
  } else ++it;
 }
}

void Brain2::ClearNodes(){
 Graph::vertex_iterator vi,vi_end;
 for(boost::tie(vi,vi_end)=vertices(g);vi!=vi_end;++vi) {
  g[*vi].value = 0;
  g[*vi].visits = 0;
 }
}

int Brain2::Age(){
	int age=0;
	for (int i=0;i<(int)origins.size();i++){
		age+=origins[i].first;
	}
	return age;
}

void Brain2::AddOneGeneration(string currentPlace) {
	if (!origins.empty() && currentPlace == origins.back().second)
		origins.back().first++;
	else
		origins.push_back(pair<int,string>(1,currentPlace));
}

vector<float> Brain2::Compute(const vector<float>& in,const int& calculationNumber){
 ClearNodes();
 Graph::vertex_iterator vi,vi_end,vi2;
 boost::tie(vi,vi_end)=vertices(g);
 //initialize inputs
 std::queue<Graph::vertex_descriptor> q; 
 fGraph fgi(g,keep_all(),vertex_subgraph(0,get(&Neuron::subgraph,g)));
 fGraph fgo(g,keep_all(),vertex_subgraph(1,get(&Neuron::subgraph,g)));
 fGraph::vertex_iterator fvi,fvi_end;

 for (int i=1;i<=calculationNumber;++i){
  // add input neurons to the visit 
  int j;
  for (boost::tie(fvi,fvi_end)=vertices(fgi),j=0;fvi!=fvi_end;++fvi,++j){
   ++fgi[*fvi].visits;
   q.push(*fvi);
   if (i==1)
    fgi[*fvi].value = in[j];
   else
    UpdateValue(*fvi,q);
  }
  while(!q.empty()){
   Graph::vertex_descriptor vd = q.front();
   q.pop();
   UpdateValue(vd,q);
  }
 } 
 //give answer;
 vector<float> ans;
 boost::tie(fvi,fvi_end)=vertices(fgo);
 for (;fvi!=fvi_end;++fvi){
  ans.push_back(g[*vi].value);
 }
 return ans;
}

void Brain2::UpdateValue(const graph_traits<Graph>::vertex_descriptor & vd, std::queue<Graph::vertex_descriptor>& q){
 int i= g[vd].visits;
 // compute the value for all destination target then
 // add all destination vertex not already in the queue
 Graph::out_edge_iterator oei,oei_end;
 boost::tie(oei,oei_end) = out_edges(vd,g);
 for(;oei!=oei_end;++oei){
  if (g[oei->m_target].visits<i){
   ++g[oei->m_target].visits;
   q.push(oei->m_target);
   Graph::vertex_descriptor nvd = oei->m_target;
   Graph::in_edge_iterator iei,iei_end;
   float result = 0;
   float test = 0;
   for (boost::tie(iei,iei_end) = in_edges(nvd,g);iei!=iei_end;++iei){
    result += g[*iei].strength.first * g[iei->m_source].value;
   }
   switch (g[nvd].type.first){
   case THRESHOLD:
    result = (result>g[nvd].threshold.first);
	   break;
   case LINEAR:
	   result = ((result-g[nvd].threshold.first)*g[nvd].coef.first);
	   break;
   case PW_LINEAR:
	   test = (result-g[nvd].threshold.first)*g[nvd].coef.first;
	   result = (test>-1)*(test*(test<=1)+1*(test>1));
	   break;
   case SIGMOID:
	   result = tanh((result-g[nvd].threshold.first)*g[nvd].coef.first);
	   break;
   default:
	   result = -1;
   }
   g[nvd].value = result;
  }
 }
}


void Brain2::WriteGraph(ostream& out){
UpdateIndices();
out<<"digraph G {\n";
set<Graph::edge_descriptor> allEdges;
Graph::edge_iterator ei, ei_end;
for (boost::tie(ei,ei_end)=edges(g);ei!=ei_end;++ei){
 allEdges.insert(*ei);
}
for (set<unsigned int>::iterator it = subgraph_indices.begin();it!=subgraph_indices.end();++it){
 out<<"subgraph cluster_"<<*it<<" {\n";
 fGraph fg(g,keep_all(),vertex_subgraph(*it,get(&Neuron::subgraph,g)));
 fGraph::edge_iterator fei, fei_end;
 fGraph::vertex_iterator fvi,fvi_end;
 for (boost::tie(fvi,fvi_end)=vertices(fg);fvi!=fvi_end;++fvi){
  out<<get(vertex_index,fg,*fvi)<<";\n";
 }
 for (boost::tie(fei,fei_end) = edges(fg);fei!=fei_end;++fei){
  allEdges.erase(*fei);
  out<<get(vertex_index,fg,source(*fei,fg))<<" -> "<< get(vertex_index,fg,target(*fei,fg))<<";\n";
 }
 out<<"}\n";
}
for (set<Graph::edge_descriptor>::iterator it = allEdges.begin();it!=allEdges.end();++it){
 out<<get(vertex_index,g,source(*it,g))<<" -> "<<get(vertex_index,g,target(*it,g))<<'\n';
}
out<<"}\n";
}

void Brain2::UpdateIndices(){
 Graph::vertex_iterator vi,vi_end;
 size_t i=0;
 for (boost::tie(vi,vi_end) = vertices(g);vi!=vi_end;++vi){
  put(vertex_index,g,*vi,i++);
 }
 i=0;
 Graph::edge_iterator ei,ei_end;
 for (boost::tie(ei,ei_end)=edges(g);ei!=ei_end;++ei){
  put(edge_index,g,*ei,i++);
 }
}