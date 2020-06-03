#include "NumGuesserWorld.h"

std::shared_ptr<ParameterLink<int>> NumGuesserWorld::modePL =
    Parameters::register_parameter(
        "WORLD_NumGuesser-mode", 0, "0 = bit outputs before adding, 1 = add outputs");
std::shared_ptr<ParameterLink<int>> NumGuesserWorld::numberOfOutputsPL =
    Parameters::register_parameter("WORLD_NumGuesser-numberOfOutputs", 10,
                                   "number of outputs in this world");
std::shared_ptr<ParameterLink<int>> NumGuesserWorld::evaluationsPerGenerationPL =
    Parameters::register_parameter("WORLD_NumGuesser-evaluationsPerGeneration", 1,
                                   "Number of times to test each Genome per "
                                   "generation (useful with non-deterministic "
                                   "brains)");
std::shared_ptr<ParameterLink<std::string>> NumGuesserWorld::groupNamePL =
    Parameters::register_parameter("WORLD_NumGuesser_NAMES-groupNameSpace",
                                   (std::string) "root::",
                                   "namespace of group to be evaluated");
std::shared_ptr<ParameterLink<std::string>> NumGuesserWorld::brainNamePL =
    Parameters::register_parameter(
        "WORLD_NumGuesser_NAMES-brainNameSpace", (std::string) "root::",
        "namespace for parameters used to define brain");

NumGuesserWorld::NumGuesserWorld(std::shared_ptr<ParametersTable> PT_)
    : AbstractWorld(PT_) {

  // columns to be added to ave file
  popFileColumns.clear();
  popFileColumns.push_back("score");
  popFileColumns.push_back("score_VAR"); // specifies to also record the
                                         // variance (performed automatically
                                         // because _VAR)
}

void NumGuesserWorld::evaluateSolo(std::shared_ptr<Organism> org, int analyze,
                             int visualize, int debug) {
  auto brain = org->brains[brainNamePL->get(PT)];
  for (int r = 0; r < evaluationsPerGenerationPL->get(PT); r++) {
    brain->resetBrain();
    brain->setInput(0, 1);
    brain->update();
      
    double score = 0.0;
    double num_to_guess = 12;
          
    for (int i = 0; i < brain->nrOutputValues; i++) {
          //std::cout << "i: " << i << std::endl;
          //std::cout << "brain->readOutput(i) " << brain->readOutput(i) << std::endl;
        if (brain->readOutput(i) == num_to_guess)
          score += 1;
    }
      
    if (score < 0.0)
      score = 0.0;
    org->dataMap.append("score", score);
    if (visualize)
      std::cout << "organism with ID " << org->ID << " scored " << score
                << std::endl;
  }
}

void NumGuesserWorld::evaluate(std::map<std::string, std::shared_ptr<Group>> &groups,
                      int analyze, int visualize, int debug) {
  int popSize = groups[groupNamePL->get(PT)]->population.size();
  for (int i = 0; i < popSize; i++) {
    evaluateSolo(groups[groupNamePL->get(PT)]->population[i], analyze,
                 visualize, debug);
  }
}

std::unordered_map<std::string, std::unordered_set<std::string>>
NumGuesserWorld::requiredGroups() {
  return {{groupNamePL->get(PT),
        {"B:" + brainNamePL->get(PT) + ",1," +
            std::to_string(numberOfOutputsPL->get(PT))}}};
  // requires a root group and a brain (in root namespace) and no addtional
  // genome,
  // the brain must have 1 input, and the variable numberOfOutputs outputs
}





