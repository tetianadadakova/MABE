//  MABE is a product of The Hintze Lab @ MSU
//     for general research information:
//         hintzelab.msu.edu
//     for MABE documentation:
//         github.com/Hintzelab/MABE/wiki
//
//  Copyright (c) 2015 Michigan State University. All rights reserved.
//     to view the full license, visit:
//         github.com/Hintzelab/MABE/wiki/License

// Evaluates agents in an NK landscape
// Landscape can be changing periodically (treadmilling) or static

#include "NKWorld.h"
#include "../../Utilities/Random.h"
#include <vector>
#include <map>

std::shared_ptr<ParameterLink<int>> NKWorld::nPL =
    Parameters::register_parameter("WORLD_NK-n", 4,
                                   "number of outputs (e.g. traits, loci)");
std::shared_ptr<ParameterLink<int>> NKWorld::kPL =
    Parameters::register_parameter("WORLD_NK-k", 2,
                                   "range of values each number can take");
std::shared_ptr<ParameterLink<bool>> NKWorld::treadmillPL =
    Parameters::register_parameter("WORLD_NK-treadmill", false,
                                   "whether landscape should treadmill over time. 0 = static landscape, 1 = treadmilling landscape");
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

  // localize N & K parameters
  N = nPL->get(PT);
  K = kPL->get(PT);

  // columns to be added to ave file
  popFileColumns.clear();
  popFileColumns.push_back("score");
  popFileColumns.push_back("score_VAR"); // specifies to also record the
                                         // variance (performed automatically
                                         // because _VAR)
}

// generate NK lookup table
// dimensions: N x 2^K
// each value is a randomly generated pair of doubles, each in [-1.0,1.0]
// represents weighting on the fitness fcn
std::vector<std::vector<std::pair<double,double>>> getNKTable(int N, int K) {
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

void NKWorld::evaluateSolo(std::shared_ptr<Organism> org, int analyze,
                             int visualize, int debug) {
  auto brain = org->brains[brainNamePL->get(PT)];
  for (int r = 0; r < evaluationsPerGenerationPL->get(PT); r++) {
    brain->resetBrain();
    brain->setInput(0, 1); // give the brain a constant 1 (for wire brain)
    brain->update();
    double score = 0.0;
    if (score < 0.0)
      score = 0.0;
    org->dataMap.append("score", score);
    if (visualize)
      std::cout << "organism with ID " << org->ID << " scored " << score
                << std::endl;
  }
}

