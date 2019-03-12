//  MABE is a product of The Hintze Lab @ MSU
//     for general research information:
//         hintzelab.msu.edu
//     for MABE documentation:
//         github.com/Hintzelab/MABE/wiki
//
//  Copyright (c) 2015 Michigan State University. All rights reserved.
//     to view the full license, visit:
//         github.com/Hintzelab/MABE/wiki/License

#include "FeatureBrain.h"

std::shared_ptr<ParameterLink<std::string>> FeatureBrain::genomeNamePL = Parameters::register_parameter("BRAIN_FEATURE-genomeNameSpace", (std::string) "root::", "namespace used to set parameters for genome used to encode this brain");
std::shared_ptr<ParameterLink<int>> FeatureBrain::nrOfColumnsPL = Parameters::register_parameter("BRAIN_FEATURE-nrOfColumns", 8, "number of columns");
std::shared_ptr<ParameterLink<int>> FeatureBrain::widthOfColumnsPL = Parameters::register_parameter("BRAIN_FEATURE-widthOfColumns", 8, "width of each columns");
std::shared_ptr<ParameterLink<bool>> FeatureBrain::recordBrainMapPL = Parameters::register_parameter("BRAIN_FEATURE-recordBrainMap", false, "do you really want to make brain maps?");
std::shared_ptr<ParameterLink<int>> FeatureBrain::brainMapIntervalPL = Parameters::register_parameter("BRAIN_FEATURE-brainMapInterval", 100, "generations between creating brain maps");
std::shared_ptr<ParameterLink<int>> FeatureBrain::maxNodeInputsPL = Parameters::register_parameter("BRAIN_FEATURE-maxNodeInputs", 4, "maximum number of inputs a node can have");
std::shared_ptr<ParameterLink<int>> FeatureBrain::maxNodeOutputsPL = Parameters::register_parameter("BRAIN_FEATURE-maxNodeOutputs", 4, "maximum number of outputs a node can have");
std::shared_ptr<ParameterLink<int>> FeatureBrain::initialCountColumnNodesPL = Parameters::register_parameter("BRAIN_FEATURE-initialCountCountColumnNodes", 0, "initial number of added nodes per column");
std::shared_ptr<ParameterLink<int>> FeatureBrain::initialCountInConnxnsPL = Parameters::register_parameter("BRAIN_FEATURE-initialCountCountInConnxns", 0, "initial number of added in connections");
std::shared_ptr<ParameterLink<int>> FeatureBrain::initialCountOutConnxnsPL = Parameters::register_parameter("BRAIN_FEATURE-initialCountCountOutConnxns", 0, "initial number of added out connections");
std::shared_ptr<ParameterLink<bool>> FeatureBrain::matchColumnIOPL = Parameters::register_parameter("BRAIN_FEATURE-matchColumnIO", true, "will match each brain input to a column and each corresponding output to the same column. will overwrite nrOfColumns. WORLD MUST HAVE SAME nrOfInputs AND nrOfOutputs!!!");
std::shared_ptr<ParameterLink<double>> FeatureBrain::adjacentColumnReachPL = Parameters::register_parameter("BRAIN_FEATURE-adjacentColumnReach", 0.5, "the proportion of adjacent columns each column can write into");


int FeatureBrain::firstInGen = -1;

FeatureBrain::FeatureBrain(int _nrInNodes, int _nrOutNodes, std::shared_ptr<ParametersTable> PT_) : AbstractBrain(_nrInNodes, _nrOutNodes, PT_) {
	matchColumnIO = matchColumnIOPL->get(PT);
	if (matchColumnIO) {
		nrOfColumns = nrInputValues;
	} else {
		nrOfColumns = nrOfColumnsPL->get(PT);
	}
	std::cout << nrInputValues << " " << nrOfColumns << std::endl;
	
	widthOfColumns = widthOfColumnsPL->get(PT);
	nrOfHiddenStates = nrOfColumns*widthOfColumns;

	// columns to be added to ave file
	popFileColumns.clear();
	popFileColumns.push_back("featureBrainRegGates");
	popFileColumns.push_back("featureBrainInGates");
	popFileColumns.push_back("featureBrainOutGates");
}

std::shared_ptr<AbstractBrain> FeatureBrain::makeBrain(std::unordered_map<std::string, std::shared_ptr<AbstractGenome>>&_genomes) {
	std::shared_ptr<FeatureBrain> newBrain = std::make_shared<FeatureBrain>(nrInputValues, nrOutputValues, PT);
	auto genomeHandler = _genomes[genomeNamePL->get(PT)]->newHandler(_genomes[genomeNamePL->get(PT)]);
	newBrain->Gates.clear();
	newBrain->wiresFromInputs.clear();
	newBrain->wiresToOutputs.clear();
	newBrain->nodes.resize(nrOfHiddenStates);
	newBrain->nextNodes.resize(nrOfHiddenStates);
	int currentSite, nextSite;


	currentSite = genomeHandler->readInt(0, 255);
	while (!genomeHandler->atEOG()) {
		//no overlapping genes
		nextSite = genomeHandler->readInt(0, 255);
		if ((currentSite = 42) && (nextSite == 213)) {
			//regular gene
			int nrI = genomeHandler->readInt(1, maxNodeInputsPL->get(PT));
			int nrO = genomeHandler->readInt(1, maxNodeOutputsPL->get(PT));
			std::vector<int> theIs, theOs, theOs2;
			std::vector<std::vector<double>> theTable;
			int column;
			int gateID;

			for (int i = 0; i < maxNodeInputsPL->get(PT); i++) {
				int value = genomeHandler->readInt(0, widthOfColumns - 1);
				if (i < nrI) {
					theIs.push_back(value);
				}
			}
			for (int i = 0; i < maxNodeOutputsPL->get(PT); i++) {
				int value = genomeHandler->readInt((adjacentColumnReachPL->get(PT)*(-1))*widthOfColumns, ((adjacentColumnReachPL->get(PT)+1)*widthOfColumns) - 1);
				if (i < nrO) {
					theOs.push_back(value);
				}
			}

			//create table for the node
			for (int i = 0; i < (1 << nrI); i++) {
				std::vector<double> row;
				for (int o = 0; o < nrO; o++) {
					row.push_back(Bit(genomeHandler->readDouble(-1.0, 1.0)));
				}
				theTable.push_back(row);
			}
			//copy the node
			theOs2.resize(theOs.size());
			for (column = 0; column < nrOfColumns; column++) {
				for (int i = 0; i < theOs.size(); i++) {
					if (theOs[i] < 0) {
						theOs2[i] = (theOs[i] + (widthOfColumns*nrOfColumns));
					} else if (theOs[i] > (widthOfColumns*nrOfColumns - 1)) {
						theOs2[i] = (theOs[i] - (widthOfColumns*nrOfColumns));
					} else {
						theOs2[i] = theOs[i];
					}
				}
                newBrain->Gates.push_back(std::make_shared<FeatureGate>(theIs,theOs2,theTable,column,newBrain->regCount));
				for (int i = 0; i < (int)theIs.size(); i++) {
					theIs[i] += widthOfColumns;
				}
				for (int o = 0; o < (int)theOs.size(); o++) {
					theOs[o] += widthOfColumns;
				}
			}
			newBrain->regCount++;
			nextSite = genomeHandler->readInt(0, 255);
		}
		else if ((currentSite = 43) && (nextSite == 212)) {
			//in connector
			if (!matchColumnIO) {
				int from = genomeHandler->readInt(0, nrInputValues - 1);
				int to = genomeHandler->readInt(0, nrOfHiddenStates - 1);
				newBrain->wiresFromInputs[from].push_back(to);
				newBrain->inCount++;
			} else if ((matchColumnIO)/* && (newBrain->inCount = 0)*/){
				int to = genomeHandler->readInt(0, widthOfColumns - 1);
				for (newBrain->inCount = 0; newBrain->inCount < nrInputValues; newBrain->inCount++) {
					int toActual = to + (newBrain->inCount*widthOfColumns);
					newBrain->wiresFromInputs[newBrain->inCount].push_back(toActual);
					newBrain->inCount++;
				}
			}
			nextSite = genomeHandler->readInt(0, 255);
		}
		else if ((currentSite = 44) && (nextSite == 211)) {
			//out connector
			if (!matchColumnIO) {
				int from = genomeHandler->readInt(0, nrOfHiddenStates - 1);
				int to = genomeHandler->readInt(0, nrOutputValues - 1);
				newBrain->wiresToOutputs[from].push_back(to);
				newBrain->outCount++;
			} else if ((matchColumnIO) && (newBrain->outCount = 0)) {
				int from = genomeHandler->readInt(0, widthOfColumns - 1);
				for (newBrain->outCount = 0; newBrain->outCount < nrInputValues; newBrain->outCount++) {
					int fromActual = from + (newBrain->outCount*widthOfColumns);
					newBrain->wiresFromInputs[fromActual].push_back(newBrain->outCount);
					newBrain->outCount++;
				}
			}
			nextSite = genomeHandler->readInt(0, 255);
		}
		/*else if ((currentSite = 45) && (nextSite == 210) && (makeCrossNodesPL->get(PT) == true)) {
			//cross connectors are gates!
			int nrI = genomeHandler->readInt(1, maxNodeInputsPL->get(PT));
			int nrO = genomeHandler->readInt(1, maxNodeOutputsPL->get(PT));
			std::vector<int> theIs, theOs;
			std::vector<std::vector<double>> theTable;
			for (int i = 0; i < maxNodeInputsPL->get(PT); i++) {
				int value = genomeHandler->readInt(0, nrOfHiddenStates - 1);
				if (i < nrI) {
					theIs.push_back(value);
				}
			}
			for (int i = 0; i < maxNodeOutputsPL->get(PT); i++) {
				int value = genomeHandler->readInt(0, nrOfHiddenStates - 1);
				if (i < nrO) {
					theOs.push_back(value);
				}
			}

			for (int i = 0; i < (1 << nrI); i++) {
				std::vector<double> row;
				for (int o = 0; o < nrO; o++) {
					row.push_back(Bit(genomeHandler->readDouble(-1.0, 1.0)));
				}
				theTable.push_back(row);
			}
			newBrain->Gates.push_back(std::make_shared<Gate>(theIs, theOs, theTable, -1, newBrain->crossCount));
			nextSite = genomeHandler->readInt(0, 255);
			newBrain->crossCount++;
		}*/
		currentSite = nextSite;
	}

	recordBrainMap = recordBrainMapPL->get(PT);
	brainMapInterval = brainMapIntervalPL->get(PT);
	if (recordBrainMap == true && firstInGen != Global::update && Global::update % brainMapInterval == 0) {
		newBrain->brainMap = "diGraph G {ranksep=3.0;";
		for (auto G : Gates) {
			/*if (G->c == -1) {
				newBrain->brainMap += "{";
				for (int i = 0; i < (int)G->I.size(); i++) {
					newBrain->brainMap += "H" + std::to_string(G->I[i]) + ";";
				}
				newBrain->brainMap += "} -> C" + std::to_string(G->g) + " -> {";
				for (int o = 0; o < (int)G->O.size(); o++) {
					newBrain->brainMap += "Hx" + std::to_string(G->O[o]) + ";";
				}
				newBrain->brainMap += "};";
			}*/
			newBrain->brainMap += "{";
			for (int i = 0; i < (int)G->I.size(); i++) {
				newBrain->brainMap += "H" + std::to_string(G->I[i]) + ";";
			}
			newBrain->brainMap += "} -> R" + std::to_string(G->g) + "inC" + std::to_string(G->c) + " -> {";
			for (int o = 0; o < (int)G->O.size(); o++) {
				newBrain->brainMap += "Hx" + std::to_string(G->O[o]) + ";";
				//std::cout << G->O[o] << std::endl;
			}
			newBrain->brainMap += "};";
			newBrain->brainMap += "R" + std::to_string(G->g) + "inC" + std::to_string(G->c) + " [style=filled,color=\"" + std::to_string(1 / (double(G->c) + 1)) + " .5 .8\"];";

		}
		for (int i = 0; i < nrInputValues; i++) {
			for (int j = 0; j < newBrain->wiresFromInputs[i].size(); j++) {
				newBrain->brainMap += "i" + std::to_string(i) + " -> H" + std::to_string(newBrain->wiresFromInputs[i][j]) + ";";
				newBrain->brainMap += "i" + std::to_string(i) + " [shape=invtriangle,style=filled,color=\"0.0 .5 .8\"]";
			}
		}
		for (int o = 0; o < nrOfHiddenStates; o++) {
			for (int k = 0; k < newBrain->wiresToOutputs[o].size(); k++) {
				newBrain->brainMap += "Hx" + std::to_string(o) + " -> o" + std::to_string(newBrain->wiresToOutputs[o][k]) + ";";
				newBrain->brainMap += "o" + std::to_string(newBrain->wiresToOutputs[o][k]) + " [shape=box,style=filled,color=\".33 .5 .8\"]";
			}
		}
		for (int i = 0; i < nrOfColumns; i++) {
			newBrain->brainMap += "subgraph \"cluster_column" + std::to_string(i) + "\" {label=\"Column " + std::to_string(i) + "\";";
			for (int j = 0; j < newBrain->regCount; j++) {
				newBrain->brainMap += "R" + std::to_string(j) + "inC" + std::to_string(i) + ";";
			}
			for (int j = 0; j < widthOfColumns; j++) {
				newBrain->brainMap += "H" + std::to_string(j + (widthOfColumns*i)) + ";Hx" + std::to_string(j + (widthOfColumns*i)) + ";";
			}
			newBrain->brainMap += "}";
		}

		newBrain->brainMap += "}";
		FileManager::writeToFile("featureBrainMapsUpdate" + std::to_string(Global::update) + ".dot", newBrain->brainMap, "");
		firstInGen = Global::update;
	}

    return newBrain;
}

void FeatureBrain::resetBrain() {
    AbstractBrain::resetBrain();
    for(int i=0;i<(int)nextNodes.size();i++){
        nextNodes[i]=0.0;
        nodes[i]=0.0;
    }
}

void FeatureBrain::update() {
    //do the inputs
    for(std::map<int,std::vector<int>>::iterator I=wiresFromInputs.begin();I!=wiresFromInputs.end();I++){
        for(auto to:I->second){
            nodes[to]=inputValues[I->first];
        }
    }
    //reset next nodes
    nextNodes.assign(nrOfHiddenStates, 0.0);
    //do all the gate updates
	for (auto G : Gates) {
		G->update(nodes, nextNodes);
	}
    //swap the buffers
    swap(nodes, nextNodes);
    //get all the info from the hidden states to the outputs
    for(std::map<int,std::vector<int>>::iterator O=wiresToOutputs.begin();O!=wiresToOutputs.end();O++){
        for(auto to:O->second){
            outputValues[to]=nodes[O->first];
            //printf("out %i %i\n",O->first,to);
        }
    }

}

std::string FeatureBrain::description() {
    return "FeatureBrain\n";
    
}

DataMap FeatureBrain::getStats(std::string &prefix) { 
	DataMap dataMap;
	dataMap.set(prefix + "featureBrainRegGates", regCount);
	dataMap.set(prefix + "featureBrainInGates", inCount);
	dataMap.set(prefix + "featureBrainOutGates", outCount);
	/*std::cout << "\nregCount: " << regCount;
	std::cout << "\ninCount: " << inCount;
	std::cout << "\noutCount: " << outCount;
	std::cout << "\ncrossCount: " << crossCount << std::endl;*/
	return dataMap; 
}

void FeatureBrain::initializeGenomes(std::unordered_map<std::string, std::shared_ptr<AbstractGenome>> &_genomes) {
    _genomes[genomeNamePL->get()]->fillRandom();
    auto genomeHandler = _genomes[genomeNamePL->get()]->newHandler(_genomes[genomeNamePL->get()]);
	for (int i = 0; i < initialCountColumnNodesPL->get(PT); i++) {
		genomeHandler->randomize();
		genomeHandler->writeInt(42, 0, 255);
		genomeHandler->writeInt(213, 0, 255);
	}
	for (int i = 0; i < initialCountInConnxnsPL->get(PT); i++) {
		genomeHandler->randomize();
		genomeHandler->writeInt(43, 0, 255);
		genomeHandler->writeInt(212, 0, 255);
	}
	for (int i = 0; i < initialCountOutConnxnsPL->get(PT); i++) {
		genomeHandler->randomize();
		genomeHandler->writeInt(44, 0, 255);
		genomeHandler->writeInt(211, 0, 255);
	}
}

std::shared_ptr<AbstractBrain> FeatureBrain::makeCopy(std::shared_ptr<ParametersTable> PT_) {
    if (PT_ == nullptr) {
        PT_ = PT;
    }
    auto newBrain = std::make_shared<FeatureBrain>(nrInputValues, nrOutputValues, PT_);
    newBrain->Gates=Gates;
    newBrain->wiresFromInputs=wiresFromInputs;
    newBrain->wiresToOutputs=wiresToOutputs;
    newBrain->nodes=nodes;
    newBrain->nextNodes=nextNodes;
    return newBrain;
}


/*** GATE defition below here***/
FeatureGate::FeatureGate(std::vector<int> &inAddr, std::vector<int> &outAddr,std::vector<std::vector<double>> &theTable,int xx, int yy){
    I=inAddr;
    O=outAddr;
    table=theTable;
	c = xx;
	g = yy;
}

void FeatureGate::update(std::vector<double> &nodes, std::vector<double> &nextNodes){
    int input=0;
    for(auto i:this->I)
        input=(input<<1)+Bit(nodes[i]);
    for(int o=0;o<(int)table[input].size();o++)
        nextNodes[O[o]]+=Bit(table[input][o]);
}
