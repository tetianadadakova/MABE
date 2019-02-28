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

#include <cmath>
#include <memory>
#include <iostream>
#include <set>
#include <vector>
#include <map>

#include "../../Genome/AbstractGenome.h"
#include "../../Utilities/Random.h"
#include "../AbstractBrain.h"
#include "../../Utilities/Parameters.h"


class Gate {
public:
    std::vector<int> I,O;
    std::vector<std::vector<double>> table;
	int c = 0;
	int g = 0;
	Gate(std::vector<int> &inAddr, std::vector<int> &outAddr, std::vector<std::vector<double>> &theTable, int xx, int yy);
    void update(std::vector<double> &nodes, std::vector<double> &nextNodes);
};


class CorticalColumnBrain : public AbstractBrain {
    std::vector<std::shared_ptr<Gate>> Gates;
    std::map<int,std::vector<int>> wiresFromInputs,wiresToOutputs;
    std::vector<double> nodes,nextNodes;
	
	int regCount = 0;
	int inCount = 0;
	int outCount = 0;
	int crossCount = 0;
	std::string brainMap = "";
	bool recordBrainMap = 0;
	int brainMapInterval = 100;

private:
	static int firstInGen;	//used to indicate if the organism is the first to be constructed in its generation

public:
    static std::shared_ptr<ParameterLink<std::string>> genomeNamePL;
    static std::shared_ptr<ParameterLink<int>> nrOfColumnsPL;
    static std::shared_ptr<ParameterLink<int>> widthOfColumnsPL;
	static std::shared_ptr<ParameterLink<bool>> recordBrainMapPL;
	static std::shared_ptr<ParameterLink<int>> brainMapIntervalPL;
	static std::shared_ptr<ParameterLink<int>> maxNodeInputsPL;
	static std::shared_ptr<ParameterLink<int>> maxNodeOutputsPL;
	static std::shared_ptr<ParameterLink<int>> initialCountColumnNodesPL;
	static std::shared_ptr<ParameterLink<int>> initialCountCrossNodesPL;
	static std::shared_ptr<ParameterLink<int>> initialCountInConnxnsPL;
	static std::shared_ptr<ParameterLink<int>> initialCountOutConnxnsPL;
	static std::shared_ptr<ParameterLink<bool>> makeCrossNodesPL;
    int nrOfHiddenStates,nrOfColumns,widthOfColumns;
    
    CorticalColumnBrain() = default;
    CorticalColumnBrain(int _nrInNodes, int _nrOutNodes, std::shared_ptr<ParametersTable> PT_);
    
    virtual void update() override;
    
    virtual std::shared_ptr<AbstractBrain> makeBrain(std::unordered_map<std::string, std::shared_ptr<AbstractGenome>> &_genomes) override;
    virtual std::shared_ptr<AbstractBrain> makeCopy(std::shared_ptr<ParametersTable> PT_ = nullptr) override;
    
    // virtual shared_ptr<AbstractBrain>
    // makeMutatedBrainFrom(shared_ptr<AbstractBrain> parent) override {
    //    //cout << "  in makeMutatedBrainFrom" << endl;
    //    return make_shared<HumanBrain>(nrInputValues, nrOutputValues, PT);
    //}
    
    // virtual shared_ptr<AbstractBrain>
    // makeMutatedBrainFromMany(vector<shared_ptr<AbstractBrain>> parents)
    // override {
    //    //cout << "  in makeMutatedBrainFromMany" << endl;
    //    return make_shared<HumanBrain>(nrInputValues, nrOutputValues, PT);
    //}
    
    virtual std::string description() override;
    virtual DataMap getStats(std::string &prefix) override;
    virtual std::string getType() override { return "Human"; }
    
    virtual void resetBrain() override;
    
    virtual void initializeGenomes( std::unordered_map<std::string, std::shared_ptr<AbstractGenome>> &_genomes);
    
    virtual std::unordered_set<std::string> requiredGenomes() override {
        return {genomeNamePL->get(PT)};
    }
};

inline std::shared_ptr<AbstractBrain> CorticalColumnBrain_brainFactory(int ins, int outs, std::shared_ptr<ParametersTable> PT = Parameters::root) {
    return std::make_shared<CorticalColumnBrain>(ins, outs, PT);
}

