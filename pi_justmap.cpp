#include <vector>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <chrono>
#include <mpi.h>
#include <mapreduce.h>
#include <keyvalue.h>
#include <fstream>

template<typename Key, typename Value>
void printKVpair (char* key, int keybytes,
		  char* value, int valuebytes,
		  void*) {
  std::cout<<"(";
  //print keys
  {
    Key* base = (Key*)key;
    for (uint64_t i = 0; i<keybytes/sizeof(Key); i++) { //be careful of divisibility in malformed data
      if (i != 0)
	std::cout<<", ";
      std::cout<<base[i];
    }
  }

  std::cout<<", "; 
  //print values
  {
    Value* base = (Value*)value;
    for (uint64_t i = 0; i<valuebytes/sizeof(Value); i++) { //be careful of divisibility in malformed data
      if (i != 0)
	std::cout<<", ";
      std::cout<<base[i];
    }
  }

  std::cout<<")\n";
}


void onepi (int id, MAPREDUCE_NS::KeyValue * kvout, void * nbpt_ptr) {
  uint64_t nbpt = *((uint64_t*)nbpt_ptr);
  //It is necessary to use a reentrant drand48 to avoid race condition
  //in random number generator. We initialize it based on the map task
  //id to provide different numbers through different map task.
  //(This may not be the best way of doing this.)
  struct drand48_data rdata; 
  srand48_r(id, &rdata); 
  
  std::pair<uint64_t, uint64_t> count; 
  count.first = 0;  //inrange
  count.second = 0; //outrange

  for (uint64_t i = 0; i<nbpt; ++i) {
    //generating a uniformly random point in the (0,0), (1,1) square
    double x, y;
    drand48_r(&rdata, &x);
    drand48_r(&rdata, &y);
    
    // is (x,y) in the quarter unit circle
    if (x*x + y*y <= 1.) 
      count.first ++;    
    else 
      count.second ++;
  }

  int dummy = 0;
  //generating output
  kvout->add ((char*)&dummy, sizeof(dummy), (char*)&count, sizeof(count));    
}


int main (int argc, char* argv[]) {
  MPI_Init(&argc, &argv);
  
  if (argc < 2) {
    std::cerr<<"usage: "<<argv[0]<<" <nbmap> <nbptpermap>"<<std::endl;
    return -1;
  }

  uint64_t nbmap = std::atol(argv[1]);
  uint64_t nbptpermap = std::atol(argv[2]);
  
  MAPREDUCE_NS::MapReduce mr;
  
  mr.map(nbmap, onepi, &nbptpermap); //generate (dummy, inrange, outrange) pairs

  //for inspection
  mr.scan(printKVpair<int, uint64_t>, nullptr);
  
  MPI_Finalize();
  return 0;
}

