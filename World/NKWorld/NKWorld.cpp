//  MABE is a product of The Hintze Lab @ MSU
//     for general research information:
//         hintzelab.msu.edu
//     for MABE documentation:
//         github.com/Hintzelab/MABE/wiki
//
//  Copyright (c) 2015 Michigan State University. All rights reserved.
//     to view the full license, visit:
//         github.com/Hintzelab/MABE/wiki/License

// Evaluates agents on how many '1's they can output. This is a purely fixed
// task
// that requires to reactivity to stimuli.
// Each correct '1' confers 1.0 point to score, or the decimal output determined
// by 'mode'.

#include "NKWorld.h"
#include "../../Utilities/Random.h"
#include <vector>
#include <map>

std::shared_ptr<ParameterLink<int>> NKWorld::numberOfOutputsPL =
    Parameters::register_parameter("WORLD_TEST-numberOfOutputs", 10,
                                   "number of outputs in this world");
std::shared_ptr<ParameterLink<int>> NKWorld::evaluationsPerGenerationPL =
    Parameters::register_parameter("WORLD_TEST-evaluationsPerGeneration", 1,
                                   "Number of times to test each Genome per "
                                   "generation (useful with non-deterministic "
                                   "brains)");
std::shared_ptr<ParameterLink<std::string>> NKWorld::groupNamePL =
    Parameters::register_parameter("WORLD_TEST_NAMES-groupNameSpace",
                                   (std::string) "root::",
                                   "namespace of group to be evaluated");
std::shared_ptr<ParameterLink<std::string>> NKWorld::brainNamePL =
    Parameters::register_parameter(
        "WORLD_TEST_NAMES-brainNameSpace", (std::string) "root::",
        "namespace for parameters used to define brain");

NKWorld::NKWorld(std::shared_ptr<ParametersTable> PT_)
    : AbstractWorld(PT_) {

  // columns to be added to ave file
  popFileColumns.clear();
  popFileColumns.push_back("score");
  popFileColumns.push_back("score_VAR"); // specifies to also record the
                                         // variance (performed automatically
                                         // because _VAR)
}

std::vector<std::vector<std::pair<double,double>>> getNKTable(int N,int K){
    std::vector<std::vector<std::pair<double,double>>> NKTable;
    NKTable.clear();
    NKTable.resize(N);
    for(int n=0;n<N;n++){
        NKTable[n].resize(1<<K);
        for(int k=0;k<(1<<K);k++){
            NKTable[n][k]= std::pair<double,double>(Random::getDouble(-1.0,1.0),Random::getDouble(-1.0,1.0));
//            printf("NK %i %i %f\n",n,k,fitnessFunction(NKTable[n][k],1.0));
        }
    }
    return NKTable;
}

void NKWorld::evaluateSolo(std::shared_ptr<Organism> org, int analyze,
                             int visualize, int debug) {
  auto brain = org->brains[brainNamePL->get(PT)];
  for (int r = 0; r < evaluationsPerGenerationPL->get(PT); r++) {
    brain->resetBrain();
    brain->setInput(0, 1); // give the brain a constant 1 (for wire brain)
    brain->update();
    double score = 0.0;
    for (int i = 0; i < brain->nrOutputValues; i++) {
      if (modePL->get(PT) == 0)
        score += Bit(brain->readOutput(i));
      else
        score += brain->readOutput(i);
    }
    if (score < 0.0)
      score = 0.0;
    org->dataMap.append("score", score);
    if (visualize)
      std::cout << "organism with ID " << org->ID << " scored " << score
                << std::endl;
  }
}

