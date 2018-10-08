//  MABE is a product of The Hintze Lab @ MSU
//     for general research information:
//         hintzelab.msu.edu
//     for MABE documentation:
//         github.com/Hintzelab/MABE/wiki
//
//  Copyright (c) 2015 Michigan State University. All rights reserved.
//     to view the full license, visit:
//         github.com/Hintzelab/MABE/wiki/License



// GardenWorld is a MABE implementation of tea-garden by Cara Reedy
// see:
//      https://github.com/Lekyn/tea-garden

#include "GardenWorld.h"

//////////////////////// PARAMETERS ///////////////////////////////

// WORLD_GARDEN parameters

std::shared_ptr<ParameterLink<int>> GardenWorld::evaluationsPerGenerationPL = 
    Parameters::register_parameter("WORLD_GARDEN-evaluationsPerGeneration", 10,
                                   "Number of evaluations per generation.");

std::shared_ptr<ParameterLink<int>> GardenWorld::gardenSizePL =
    Parameters::register_parameter("WORLD_GARDEN-gardenSize", 13,
                                   "Height and width of the world.");

std::shared_ptr<ParameterLink<int>> GardenWorld::maxPopPL =
    Parameters::register_parameter("WORLD_GARDEN-maxPop", 500,
                                   "Maximum population allowed in the world. "
                                   "When maxPop is reached, no new organisms will be born until some die.");

std::shared_ptr<ParameterLink<int>> GardenWorld::maxGenPL =
    Parameters::register_parameter("WORLD_GARDEN-maxGen", 10,
                                   "Number of concurrent generations allowed in the world. "
                                   "When maxGen is reached for a lineage, no more children will be born until the eldest generation dies.");
// WORLD_GARDEN_FOOD parameters

std::shared_ptr<ParameterLink<std::string>> GardenWorld::gardenModePL =
    Parameters::register_parameter("WORLD_GARDEN_FOOD-gardenMode", (std::string) "Static", 
                                   "Decide whether the food sources are static (constant nutritive value) "
                                   "or dynamic (changing nutritive values).");

std::shared_ptr<ParameterLink<int>> GardenWorld::switchTimePL =
    Parameters::register_parameter("WORLD_GARDEN_FOOD-switchTime", 40,
                                   "If dynamic, number of ticks before nutrition values switch.");

std::shared_ptr<ParameterLink<double>> GardenWorld::valFood1PL =
    Parameters::register_parameter("WORLD_GARDEN_FOOD-valFood1", 1.0,
                                   "Starting nutritional value of Food1 [0 to 1.0].");

std::shared_ptr<ParameterLink<double>> GardenWorld::valFood2PL =
    Parameters::register_parameter("WORLD_GARDEN_FOOD-valFood2", 0.3,
                                   "Starting nutritional value of Food2 [0 to 1.0].");

// WORLD_GARDEN_OBJECTS parameters

std::shared_ptr<ParameterLink<double>> GardenWorld::pctFood1PL=
    Parameters::register_parameter("WORLD_GARDEN_OBJECTS-pctFood1", 0.2,
                                   "Percentage of the world taken up by Food 1 [0 to 1.0].");

std::shared_ptr<ParameterLink<double>> GardenWorld::pctFood2PL=
    Parameters::register_parameter("WORLD_GARDEN_OBJECTS-pctFood2", 0.0,
                                   "Percentage of the world taken up by Food 2 [0 to 1.0]. Overriden to 0.0 if mode is Static.");

std::shared_ptr<ParameterLink<double>> GardenWorld::pctToyPL=
    Parameters::register_parameter("WORLD_GARDEN_OBJECTS-pctToyPL", 0.04,
                                   "Percentage of the world taken up by Toys [0 to 1.0].");

std::shared_ptr<ParameterLink<double>> GardenWorld::pctRockPL=
    Parameters::register_parameter("WORLD_GARDEN_OBJECTS-pctRockPL", 0.02,
                                   "Percentage of the world taken up by Rocks [0 to 1.0].");

// WORLD_GARDEN_DRIVES parameters

std::shared_ptr<ParameterLink<double>> GardenWorld::initFullnessPL =
    Parameters::register_parameter("WORLD_GARDEN_DRIVES-initFullness", 150.0,
                                   "Starting value of fullness drive.");

std::shared_ptr<ParameterLink<double>> GardenWorld::initAmusementPL =
    Parameters::register_parameter("WORLD_GARDEN_DRIVES-initAmusement", 150.0,
                                   "Starting value of amusement drive.");

std::shared_ptr<ParameterLink<double>> GardenWorld::initPainPL =
    Parameters::register_parameter("WORLD_GARDEN_DRIVES-initPain", 0.0,
                                   "Starting value of pain drive.");

std::shared_ptr<ParameterLink<double>> GardenWorld::initDesirePL =
    Parameters::register_parameter("WORLD_GARDEN_DRIVES-initDesire", 0.0,
                                   "Starting value of desire drive.");


// WORLD_GARDEN_NAMES parameters
std::shared_ptr<ParameterLink<std::string>> GardenWorld::groupNamePL =
    Parameters::register_parameter("WORLD_GARDEN_NAMES-groupNameSpace",
                                   (std::string) "root::",
                                   "namespace of group to be evaluated");

std::shared_ptr<ParameterLink<std::string>> GardenWorld::brainNamePL =
    Parameters::register_parameter("WORLD_GARDEN_NAMES-brainNameSpace", 
                                   (std::string) "root::",
                                   "namespace for parameters used to define brain");

///////////////////////// I/O NODES ///////////////////////////////////

// Input Nodes: Sensory
const int GardenWorld::nodeDirt = 0;
const int GardenWorld::nodeFood1 = 1;
const int GardenWorld::nodeFood2 = 2;
const int GardenWorld::nodeRock = 3;
const int GardenWorld::nodeToy = 4;
const int GardenWorld::nodeOrg = 5;

const int GardenWorld::numObjects = 6;

// Input Nodes: Drives
const int GardenWorld::nodeAmusement = 6;
const int GardenWorld::nodeDesire = 7;
const int GardenWorld::nodeFullness = 8;
const int GardenWorld::nodePain = 9;

const int GardenWorld::numDrives = 4;

// Output Nodes

const int GardenWorld::nodeForward = 0;
const int GardenWorld::nodeLeft = 1;
const int GardenWorld::nodeRight = 2;
const int GardenWorld::nodeEat = 3;
const int GardenWorld::nodePlay = 4;
const int GardenWorld::nodeMate = 5;

const int GardenWorld::numOutputs = 6;

////////////////////////// WORLD FUNCTIONS ///////////////////////////


// Score Recording 
GardenWorld::GardenWorld(std::shared_ptr<ParametersTable> PT_)
    : AbstractWorld(PT_) {

  // Localize parameters
  evaluationsPerGeneration = evaluationsPerGenerationPL->get(PT);
  gardenSize = gardenSizePL->get(PT);
  maxPop = maxPopPL->get(PT);
  maxGen = maxGenPL->get(PT);
    
  gardenMode = gardenModePL->get(PT);
  switchTime = switchTimePL->get(PT);
  valFood1 = valFood1PL->get(PT);
  valFood2 = valFood2PL->get(PT);

  pctRock = pctRockPL->get(PT);
  pctToy = pctToyPL->get(PT);
  pctFood1 = pctFood1PL->get(PT);
  pctFood2 = pctFood2PL->get(PT);

  initFullness = initFullnessPL->get(PT);
  initAmusement = initAmusementPL->get(PT);
  initPain = initPainPL->get(PT);
  initDesire = initDesirePL->get(PT);

  groupName = groupNamePL->get(PT);
  brainName = brainNamePL->get(PT);


  // Clean out population columns
  popFileColumns.clear();

  // Specify which columns to add to population file
  popFileColumns.push_back("score");
  popFileColumns.push_back("score_VAR");
  popFileColumns.push_back("steps");
  popFileColumns.push_back("turns");
  popFileColumns.push_back("eats");

}
// Initialize the map and return its available locations
std::pair< Vector2d<char>,std::vector<Point2d> > GardenWorld::initializeMapAndLocations(int gardenSize, double pctRock, double pctToy, double pctFood1, double pctFood2) {
    
    Vector2d<char> gardenMap(gardenSize,gardenSize);
    int gardenArea = gardenSize*gardenSize;

    // Check if fill percentages are >100% 
    if (pctRock + pctToy + pctFood1 + pctFood2 > 1.0) {
        std::cout << "ERROR! In GardenWorld:: Garden object initialization adds up to greater than 100%." << std::endl;
        std::cout << "Please reinitialize percentages." <<std::endl;
        std::cout << "Exiting." <<std::endl;
        exit(1);
    } 

    // Create vector of garden locations (flattened version of gardenMap w/ coordinates only)
    std::vector<Point2d> availableLocations(gardenArea);
    
    // Fill the garden with charDirt and create the elements of availableLocations
    for (int x = 0; x < gardenSize; x++) {
        for (int y = 0; y < gardenSize; y++) {
            gardenMap(x,y) = charDirt;
            availableLocations[x + y*gardenSize].set(x,y); // flattens the grid
        }
    }

    // Shuffle the availableLocations vector
    // This is our way to randomly distribute resources throughout the garden
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    auto rng = std::default_random_engine(seed);
    std::shuffle(std::begin(availableLocations), std::end(availableLocations), rng);
    
    // Determine counts of resources from percents and garden area
    int ctFood1 = std::floor(pctFood1*gardenArea);
    int ctFood2 = std::floor(pctFood2*gardenArea);
    int ctToy = std::floor(pctToy*gardenArea);
    int ctRock = std::floor(pctRock*gardenArea); 

    // Distribute the items in the garden using helper function addItemsToGarden
    addItemsToGarden(gardenMap, availableLocations, charFood1, ctFood1);
    addItemsToGarden(gardenMap, availableLocations, charFood2, ctFood2);
    addItemsToGarden(gardenMap, availableLocations, charToy, ctToy);
    addItemsToGarden(gardenMap, availableLocations, charRock, ctRock);

    std::pair< Vector2d<char>, std::vector<Point2d> > gardenMapAndLocations(gardenMap, availableLocations);
    return gardenMapAndLocations;
}

// Helper function for initializeGardenMap
// Inserts appropriate char (representing the item) to gardenMap ctItem times, given randomly sorted locations vector
void GardenWorld::addItemsToGarden(Vector2d<char> &gardenMap, std::vector<Point2d> &availableLocations, char item, int ctItem) {
    
    int xLoc, yLoc;
    // we iterate backwards through the vector because backwards vector travel is easier than forwards
    // requires less rearranging (following indices don't need to move up)
    for (int ct = 0; ct < ctItem; ct++) {
        xLoc = availableLocations.back().x;
        yLoc = availableLocations.back().y;
        gardenMap(xLoc,yLoc) = item;
        availableLocations.pop_back();
    }
}

// Helper function for initializeGardenMap
// Inserts a char representing the initial location of each organism (as in addItemsToGarden)
// Returns a vector where each entry is a vector which contains the location of each organism and its ID.

std::vector< std::vector<int> > GardenWorld::initializeOrganismLocations(Vector2d<char> &gardenMap, std::vector<Point2d> &availableLocations, char org, int orgPopSize) {
    
    size_t numAvailableLocations = availableLocations.size();
    std::cout << "Locations: " << numAvailableLocations << std::endl;
    if (orgPopSize > numAvailableLocations) {
        std::cout << "ERROR! In GardenWorld:: the initial population exceeds the number of available garden squares." << std::endl;
        std::cout << "Please increase the garden size, reduce the number of garden objects, reduce the initial population, or some combination of these options." <<std::endl;
        std::cout << "Exiting." << std::endl;
        exit(1); 
    }
    // a vector of vectors
    // the inner vectors are <xLoc, yLoc, orgID>
    std::vector< std::vector<int> > orgLocations;

    // we iterate backwards through the vector because backwards vector travel is easier than forwards
    // requires less rearranging (following indices don't need to move up)
    for (int orgID = 0; orgID < orgPopSize; orgID++) {
        
        // adds organism to garden
        int xLoc = availableLocations.back().x;
        int yLoc = availableLocations.back().y;
        gardenMap(xLoc,yLoc) = charOrg;
        availableLocations.pop_back();
        
        // adds organism location to orgLocation
        std::vector<int> thisOrg = {xLoc,yLoc,orgID};
        orgLocations.push_back(thisOrg);
    }

    return orgLocations;
} 

// TODO
//
// Organism I/O
// - Give each organism a location
// - Create input vector 
// - Create output vector of length 6?
// 
// Implement Reproduction
// 
// Implement Archiving

// Evaluation Functions
void GardenWorld::evaluateSolo(std::shared_ptr<Organism> org, int analyze, int visualize, int debug) {
  
  // Initialize the garden map and the available locations 
  std::pair< Vector2d<char> , std::vector<Point2d> > gardenMapAndLocations = initializeMapAndLocations(gardenSize, pctRock, pctToy, pctFood1, pctFood2);
  Vector2d<char> gardenMap = gardenMapAndLocations.first;
  std::vector<Point2d> availableLocations = gardenMapAndLocations.second;
  //std::vector< std::vector<int> > organismLocations = initializeOrganismLocations(&gardenMap, &availableLocations, org, orgPopSize);
  
  

  // Specify brain
  auto brain = org->brains[brainNamePL->get(PT)];

  // Set initial drive values
  
  brain->setInput(nodeAmusement, initAmusement);
  brain->setInput(nodeDesire, initDesire);
  brain->setInput(nodeFullness, initFullness);
  brain->setInput(nodePain, initPain);

  Point2d orgPosition(0,0);
  int orgFacing = 0;
  
  double score = 0.0;
  
  // Get x and y coordinates of the point in front of the organism 
  Point2d orgFront(orgPosition.x + dx[orgFacing], orgPosition.y + dy[orgFacing]);
  
  int steps = 0;
  int turns = 0; 
  int eats = 0;

  for (int r = 0; r < evaluationsPerGeneration; r++) {
    
    brain->resetBrain();
    
    for (int node = 0; node < numObjects; node++) {
        brain->setInput(node, 0); 
    }
    
    
    // Set input node based on what the item at orgFront is
    
    int nodeInput;

    switch(gardenMap(orgFront)) {
        case charDirt: nodeInput = nodeDirt; break;
        case charFood1: nodeInput = nodeFood1; break;
        case charFood2: nodeInput = nodeFood2; break;
        case charRock: nodeInput = nodeRock; break;
        case charToy: nodeInput = nodeToy; break;
        case charOrg: nodeInput = nodeOrg; break;

    }

    // TODO: Amusement, Desire, Fullness, Pain drive updates
    // Amusement, Fullness, Pain should go down over time
    // Desire should modulate on its own
    //
    // For now, for tests, increase the organism's score if fullness is > 75.0

    brain->setInput(nodeInput, 1); 

    brain->update();
    
    std::vector<double> brainOutputs = brain->outputValues;

    // Returns the index of the node with the highest output
    double outputMax = *std::max_element(brainOutputs.begin(), brainOutputs.end());
    int nodeMax = std::distance(brainOutputs.begin(), std::max_element(brainOutputs.begin(), brainOutputs.end()));

    // Get the current values of the drives
    double amusement = brain->readInput(nodeAmusement);
    double desire = brain->readInput(nodeDesire);
    double fullness = brain->readInput(nodeFullness);
    double pain = brain->readInput(nodePain);

    // Determine action based upon the highest scoring output node
    switch (nodeMax) {
        case nodeForward: 
            orgPosition.x += dx[orgFacing];
            orgPosition.y += dy[orgFacing];
            steps++;
            break;
        case nodeLeft:
            orgFacing--;
            if (orgFacing < 0) {
                orgFacing = 3;
            }
            turns++;
            break;
        case nodeRight:
            orgFacing++;
            if (orgFacing > 3) {
                orgFacing = 0;
            }
            turns++;
            break;
        case nodeEat:
            if (gardenMap(orgFront) == charFood1) {
                gardenMap(orgFront) = charDirt;
                brain->setInput(nodeFullness, fullness + valFood1);
            } else if (gardenMap(orgFront) == charFood2) {
                gardenMap(orgFront) = charDirt;
                brain->setInput(nodeFullness, fullness + valFood2);
            } else if (gardenMap(orgFront) == charRock) {
                pain += 30.0;
                brain->setInput(nodePain, pain);
            }
            eats++;
            break;
        case nodePlay:
            if (gardenMap(orgFront) == charToy) {
                amusement += 30.0;
                brain->setInput(nodeAmusement, amusement); 
            } else if (gardenMap(orgFront) == charRock) {
                amusement -= 10.0;
                brain->setInput(nodeAmusement, amusement);
            }
            break;
        case nodeMate: //TODO: Implement Mating
            break;
    }

    fullness -= 0.5;
    brain->setInput(nodeFullness, fullness);
    
    // Wrap arounds
    if (orgPosition.x > gardenSize - 1) {
        orgPosition.x = 0;
    } else if (orgPosition.x < 0) {
        orgPosition.x = gardenSize - 1;
    } 

    if (orgPosition.y > gardenSize - 1) {
        orgPosition.y = 0; 
    } else if (orgPosition.y < 0) {
        orgPosition.y = gardenSize - 1;
    } 

    score = brain->readInput(nodeFullness);
  }

  
  org->dataMap.append("score", score);
  org->dataMap.append("steps", steps);
  org->dataMap.append("turns", turns);
  org->dataMap.append("eats", eats);
  if (visualize) {
    std::cout << "organism with ID " << org->ID << " scored " << score << std::endl;
    std::cout << "organism with ID " << org->ID << " ended at " << orgPosition.x << "," << orgPosition.y << std::endl;
  }
}
