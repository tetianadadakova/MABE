#pragma once

#include <World/AbstractWorld.h>

#include <cstdlib>
#include <thread>
#include <vector>

class NumGuesserWorld : public AbstractWorld {

public:
  static std::shared_ptr<ParameterLink<int>> modePL;
  static std::shared_ptr<ParameterLink<int>> numberOfOutputsPL;
  static std::shared_ptr<ParameterLink<int>> evaluationsPerGenerationPL;

  // int mode;
  // int numberOfOutputs;
  // int evaluationsPerGeneration;

  static std::shared_ptr<ParameterLink<std::string>> groupNamePL;
  static std::shared_ptr<ParameterLink<std::string>> brainNamePL;
  // string groupName;
  // string brainName;

  NumGuesserWorld(std::shared_ptr<ParametersTable> PT_ = nullptr);
  virtual ~NumGuesserWorld() = default;

  void evaluateSolo(std::shared_ptr<Organism> org, int analyze,
                            int visualize, int debug);
  void evaluate(std::map<std::string, std::shared_ptr<Group>> &groups,
                int analyze, int visualize, int debug);

  virtual std::unordered_map<std::string, std::unordered_set<std::string>>
    requiredGroups() override;
};

