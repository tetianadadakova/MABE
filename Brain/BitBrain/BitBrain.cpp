//  MABE is a product of The Hintze Lab @ MSU

//     for general research information:
//         hintzelab.msu.edu
//     for MABE documentation:
//         github.com/Hintzelab/MABE/wiki
//
//  Copyright (c) 2015 Michigan State University. All rights reserved.
//     to view the full license, visit:
//         github.com/Hintzelab/MABE/wiki/License

#include "../BitBrain/BitBrain.h"

shared_ptr<ParameterLink<string>> BitBrain::genomeNamePL =
  Parameters::register_parameter("BRAIN_BIT_NAMES-genomeName",
                                 (string) "root::",
                                 "root:: is default value");
shared_ptr<ParameterLink<int>> BitBrain::nrOfHiddenNodesPL =
  Parameters::register_parameter(
    "BRAIN_BIT-nrOfHiddenNodes",
    0,
    "number of hidden nodes. used to allow recurrence between brain updates");
shared_ptr<ParameterLink<int>> BitBrain::nrOfLayersPL =
  Parameters::register_parameter(
    "BRAIN_BIT-nrOfLayers",
    0,
    "number of \"hidden\" layers. used to add depth to the brain's topology");
shared_ptr<ParameterLink<int>> BitBrain::nrOfGateInsPL =
  Parameters::register_parameter("BRAIN_BIT-nrOfGateIns",
                                 2,
                                 "number of inputs each gate has");
shared_ptr<ParameterLink<string>> BitBrain::gateTrackingSetPL =
  Parameters::register_parameter("BRAIN_BIT-gateTrackingSet",
                                 (string) "-1",
                                 "list of output nodes that will be recursively tested for gate usage. -1 signals no tracking");

BitBrain::BitBrain(int nrInNodes,
                   int nrOutNodes,
                   shared_ptr<ParametersTable> PT)
  : AbstractBrain(nrInNodes, nrOutNodes, PT)
{
  genomeName = genomeNamePL->get(PT);
  nrOfLayers = nrOfLayersPL->get(PT);
  nrOfGateIns = nrOfGateInsPL->get(PT);
  H = nrOfHiddenNodesPL->get(PT);
  I = nrInNodes;
  O = nrOutNodes;
  auto gateTrackingSetString = gateTrackingSetPL->get(PT);
  if (gateTrackingSetString != "-1"){
    gateTrackingSet = seq(gateTrackingSetString, I + O + H);
  }
}

shared_ptr<AbstractBrain>
BitBrain::makeBrain(unordered_map<string, shared_ptr<AbstractGenome>>& genomes)
{
  shared_ptr<BitBrain> newBrain =
    make_shared<BitBrain>(nrInputValues, nrOutputValues, PT);
  auto genomeHandler =
    genomes[genomeName]->newHandler(genomes[genomeName], true);
  newBrain->H = H;
  newBrain->I = I;
  newBrain->O = O;
  newBrain->nrOfLayers = nrOfLayers;

  newBrain->nodes.resize(nrOfLayers + 2);

  for (int i = 0; i < nrOfLayers + 2; i++) {
    newBrain->nodes[i].resize(I + O + H);
  }

  newBrain->gates.resize(nrOfLayers + 1);

  for (int i = 0; i < nrOfLayers + 1; i++) {
    newBrain->gates[i].resize(I + O + H);

    for (int j = 0; j < (I + O + H); j++) {
      newBrain->gates[i][j] =
        make_shared<Gate>(genomeHandler, I + O + H, nrOfGateIns, i, j);
    }
  }
  //cout << I + O + H << endl;
  newBrain->markGatesUsed();
  return newBrain;
}


void
BitBrain::markGateUsed(shared_ptr<Gate> gate)
{
  if (! gate->used){
    //cout << "unvisited gate " << gate->node << " in layer " << gate->layer << endl;
    gate->used = true;
    if (gate->layer > 0){
      //cout << "Recursing up a layer..." << endl;
      for (auto nodeID:gate->ins){
        //cout << "Visiting node " << nodeID << " as a child of " << gate->node << endl;
        markGateUsed(gates[gate->layer -1][nodeID]);
        //cout << "Returned from recursing on gate " << nodeID << endl;
      }
    }
    else{
      for (auto nodeID:gate->ins){
        if (nodeID >= I+O){
          //cout << "Hidden node " << nodeID << " in layer 0 is a child of gate " << gate->node << ". Recursing on hidden node " << nodeID << " at output layer..." << endl;
          markGateUsed(gates[gates.size() -1][nodeID]);
        }
      }
    }
  }
}


void
BitBrain::markGatesUsed()
{
  if (!gateTrackingSet.empty()){
    for (auto nodeID:gateTrackingSet){
      markGateUsed(gates[gates.size() -1][nodeID]);
      //cout << "Returned from recursing on gate " << nodeID << endl;
    }
  }
  else{
    for (auto gateLayer:gates){
      for (auto gate:gateLayer){
        gate->used = true; 
      }
    }
  }
}


void
BitBrain::resetBrain()
{
  for (auto N : nodes) {
    for (int i = 0; i < (int)N.size(); i++) {
      N[i] = 0.0;
    }
  }
}

void
BitBrain::setInput(const int& inputAddress, const double& value)
{
  nodes[0][inputAddress] = value;
}

double
BitBrain::readInput(const int& inputAddress)
{
  return nodes[0][inputAddress];
}

void
BitBrain::setOutput(const int& outputAddress, const double& value)
{
  nodes[(int)nodes.size() - 1][outputAddress] = value;
}

double
BitBrain::readOutput(const int& outputAddress)
{
  return nodes[(int)nodes.size() - 1][outputAddress];
}

void
BitBrain::update()
{
  for (int i = 0; i < nrOfLayers + 1; i++) {
    for (int j = 0; j < gates[i].size(); j++) {
      nodes[i + 1][j] = gates[i][j]->update(nodes[i]);
    }
  }

  for (int i = 0; i < H; i++) {
    nodes[0][I + O + i] = nodes[nodes.size() - 1][I + O + i];
  }
}

vector<int>
BitBrain::getHiddenNodes()
{
  vector<int> temp = {};
  for (int i = 0; i < H; i++) {
    temp.push_back(Bit(nodes[nodes.size() - 1][I + O + i]));
  }
  return temp;
}

void inline BitBrain::resetOutputs()
{
  for (int o = 0; o < O; o++) {
    nodes[(int)nodes.size() - 1][o] = 0.0;
  }
}

vector<int>
BitBrain::reportComponents()
{
  vector<int> retrn = vector<int>((1<<(1<<nrOfGateIns)) + 1 ,0);
  //1<<(1<<nrOfGateIns))+1 = (2^(2^x))+1
  for (int i = 0; i < nrOfLayers + 1; i++){
    for (auto gate:gates[i]){
      if (gate->used){
        int logicID = 0;
        for (int j = 0; j < gate->output.size(); j++){
          logicID += gate->output[j] << j; 
        }
        retrn[logicID]++;
      }
    }
  }
  return retrn;
}

string
BitBrain::description()
{
  string S = "Bit Brain";

  return S;
}

DataMap
BitBrain::getStats(string& prefix)
{
  DataMap dataMap;

  return dataMap;
}

void
BitBrain::initializeGenomes(
  unordered_map<string, shared_ptr<AbstractGenome>>& genomes)
{
  genomes[genomeName]->fillRandom();
}

shared_ptr<AbstractBrain>
BitBrain::makeCopy(shared_ptr<ParametersTable> PT_)
{
  if (PT_ == nullptr) {
    PT_ = PT;
  }
  auto newBrain = make_shared<BitBrain>(nrInputValues, nrOutputValues, PT_);
  newBrain->H = H;
  newBrain->I = I;
  newBrain->O = O;
  newBrain->nodes = nodes;
  newBrain->gates = gates;
  return newBrain;
}

void
BitBrain::showBrain()
{
  // EMPTY
}