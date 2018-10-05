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
#include "Utilities/VectorNd.h"
#include "Utilities/PointNd.h"

#include <cstdlib>
#include <thread>
#include <vector>
#include <iterator>

class GardenWorld : public AbstractWorld {

public:

// see GardenWorld.cpp for parameter descriptions

  //////////// PARAMETERS ///////////

  // Overall Parameters
  static std::shared_ptr<ParameterLink<int>> evaluationsPerGenerationPL;
  static std::shared_ptr<ParameterLink<int>> gardenSizePL;
  static std::shared_ptr<ParameterLink<int>> maxPopPL;
  static std::shared_ptr<ParameterLink<int>> maxGenPL;
  int evaluationsPerGeneration;
  int gardenSize;
  int maxPop;
  int maxGen; 

  // Food Parameters
  static std::shared_ptr<ParameterLink<std::string>> gardenModePL;
  static std::shared_ptr<ParameterLink<int>> switchTimePL;
  static std::shared_ptr<ParameterLink<double>> valFood1PL;
  static std::shared_ptr<ParameterLink<double>> valFood2PL;
  std::string gardenMode;
  int switchTime;
  double valFood1;
  double valFood2;

  // Object Parameters
  static std::shared_ptr<ParameterLink<double>> pctRockPL;
  static std::shared_ptr<ParameterLink<double>> pctToyPL;
  static std::shared_ptr<ParameterLink<double>> pctFood1PL;
  static std::shared_ptr<ParameterLink<double>> pctFood2PL;
  double pctRock;
  double pctToy;
  double pctFood1;
  double pctFood2;

  // Drive Parameters
  static std::shared_ptr<ParameterLink<double>> initFullnessPL;
  static std::shared_ptr<ParameterLink<double>> initAmusementPL;
  static std::shared_ptr<ParameterLink<double>> initPainPL;
  static std::shared_ptr<ParameterLink<double>> initDesirePL;
  double initFullness;
  double initAmusement;
  double initPain;
  double initDesire;

  // Namespace Parameters
  static std::shared_ptr<ParameterLink<std::string>> groupNamePL;
  static std::shared_ptr<ParameterLink<std::string>> brainNamePL;
  std::string groupName;
  std::string brainName;

  ///// CONSTRUCTORS & EVALUATORS //////

  // Declare Constructor & Destructor
  GardenWorld(std::shared_ptr<ParametersTable> PT_ = nullptr);
  virtual ~GardenWorld() = default;

  // Declare Solo Evaluator
  void evaluateSolo(std::shared_ptr<Organism> org, int analyze,int visualize, int debug);

  // Declare Group Evaluator
  virtual void evaluate(std::map<std::string, std::shared_ptr<Group>> &groups, int analyze, int visualize, int debug) {
    int popSize = groups[groupNamePL->get(PT)]->population.size();
    for (int i = 0; i < popSize; i++) {
      evaluateSolo(groups[groupNamePL->get(PT)]->population[i], analyze, visualize, debug);
    }
  }

  ////////// DIRECTIONAL MOVEMENT ////////

  // Each entry represents a direction to move
  // 0: Move Right
  // 1: Move Down
  // 2: Move Left
  // 3: Move Up

  // Numerical values of x and y change correspond to the coordinate system established in Vector2d
  // Follows conventions of computer graphics (top left = 0,0 ; x increases right ; y increases down)
  
  const int dx[4] = {1, 0, -1, 0};
  const int dy[4] = {0, 1, 0, -1};


  ////////// MAP VARIABLES //////////
  
  static const char charDirt = 'd', charFood1 = 'f', charFood2 = 'g', charRock = 'r', charToy = 't', charOrg = 'o';

  ////////// INPUT/OUTPUT NODES ////////

  // Input Nodes
  
  //// Senses
  
  static const int numObjects = 6;
  static const int nodeDirt = 0;
  static const int nodeFood1 = 1;
  static const int nodeFood2 = 2;
  static const int nodeRock = 3;
  static const int nodeToy = 4;
  static const int nodeOrg = 5;

  //// Drives
  
  const int numDrives = 4;
  const int nodeAmusement = 6;
  const int nodeDesire = 7;
  const int nodeFullness = 8;
  const int nodePain = 9;
 
  // Output Nodes
  static const int nodeForward = 0;
  static const int nodeLeft = 1;
  static const int nodeRight = 2;
  static const int nodeEat = 3;
  static const int nodePlay = 4;
  static const int nodeMate = 5; 


  ////////// WORLD FUNCTIONS ////////
  
  std::pair <Vector2d<char> , std::vector<Point2d> > initializeMapAndLocations(int gardenSize, double pctRock, double pctToy, double pctFood1, double pctFood2);
  
  void addItemsToGarden(Vector2d<char> &gardenMap, std::vector<Point2d> &availableLocations, char item, int ctItem);
  
  std::vector< std::vector<int> > initializeOrganismLocations(Vector2d<char> &gardenMap, std::vector<Point2d> &availableLocations, char org, int initPop);
  

  
          
  ////////// BRAIN REQUIREMENT //////

  virtual std::unordered_map<std::string, std::unordered_set<std::string>>
  requiredGroups() override {
    return {{groupNamePL->get(PT),{"B:" + brainNamePL->get(PT) + ",10,6"}}}; // 10 inputs, 6 outputs
  }

};
