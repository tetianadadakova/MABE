//  MABE is a product of The Hintze Lab @ MSU
//     for general research information:
//         hintzelab.msu.edu
//     for MABE documentation:
//         github.com/Hintzelab/MABE/wiki
//
//  Copyright (c) 2015 Michigan State University. All rights reserved.
//     to view the full license, visit:
//         github.com/Hintzelab/MABE/wiki/License

#pragma once

#include "../AbstractWorld.h"

#include <cstdlib>
#include <thread>
#include <vector>

class NKWorld : public AbstractWorld {

public:

  // Parameters
  static std::shared_ptr<ParameterLink<int>> nPL;
  static std::shared_ptr<ParameterLink<int>> kPL;
  static std::shared_ptr<ParameterLink<int>> evaluationsPerGenerationPL;

  static std::shared_ptr<ParameterLink<bool>> treadmillPL; 

  static std::shared_ptr<ParameterLink<std::string>> groupNamePL;
  static std::shared_ptr<ParameterLink<std::string>> brainNamePL;

  int N;
  int K;
  
  NKWorld(std::shared_ptr<ParametersTable> PT_ = nullptr);
  virtual ~NKWorld() = default;

  // NK-specific functions
  std::vector<std::vector<std::pair<double,double>>> getNKTable(int N, int K);
  double fitnessFunction(std::pair<double,double> &value,double t);

  void evaluateSolo(std::shared_ptr<Organism> org, int analyze,
                            int visualize, int debug);
  
  virtual void evaluate(std::map<std::string, std::shared_ptr<Group>> &groups,
                        int analyze, int visualize, int debug) {
    int popSize = groups[groupNamePL->get(PT)]->population.size();
    for (int i = 0; i < popSize; i++) {
      evaluateSolo(groups[groupNamePL->get(PT)]->population[i], analyze,
                   visualize, debug);
    }
  }

  // generate NK lookup table
  // dimensions: N x 2^K
  // each value is a randomly generated pair of doubles, each in [-1.0,1.0]
  // represents weighting on the fitness fcn
  std::vector<std::vector<std::pair<double,double>>> NKWorld::getNKTable(int N, int K) {
      std::vector<std::vector<std::pair<double,double>>> NKTable;
      NKTable.clear();
      NKTable.resize(N);
      for(int n=0;n<N;n++){
          NKTable[n].resize(1<<K);
          for(int k=0;k<(1<<K);k++){
              NKTable[n][k]= std::pair<double,double>(Random::getDouble(-1.0,1.0),Random::getDouble(-1.0,1.0));
          }
      }
      return NKTable;
  } 


  virtual std::unordered_map<std::string, std::unordered_set<std::string>>
  requiredGroups() override {
    return {{groupNamePL->get(PT),
             {"B:" + brainNamePL->get(PT) + ",1," +
              std::to_string(nPL->get(PT))}}};
    // requires a root group and a brain (in root namespace) and no addtional
    // genome,
    // the brain must have 1 input, and the variable numberOfOutputs outputs
  }
};

