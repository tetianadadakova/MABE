//  MABE is a product of The Hintze Lab @ MSU
//     for general research information:
//         hintzelab.msu.edu
//     for MABE documentation:
//         github.com/Hintzelab/MABE/wiki
//
//  Copyright (c) 2015 Michigan State University. All rights reserved.
//     to view the full license, visit:
//         github.com/Hintzelab/MABE/wiki/License

#include "CorticalColumnBrain.h"

std::shared_ptr<ParameterLink<std::string>> CorticalColumnBrain::genomeNamePL = Parameters::register_parameter("BRAIN_CORTICALCOLUMN-genomeNameSpace", (std::string) "root::", "namespace used to set parameters for genome used to encode this brain");
std::shared_ptr<ParameterLink<int>> CorticalColumnBrain::nrOfColumnsPL = Parameters::register_parameter("BRAIN_CORTICALCOLUMN-nrOfColumns", 8 , "number of columns");
std::shared_ptr<ParameterLink<int>> CorticalColumnBrain::widthOfColumnsPL = Parameters::register_parameter("BRAIN_CORTICALCOLUMN-widthOfColumns", 8 , "width of each columns");
std::shared_ptr<ParameterLink<bool>> CorticalColumnBrain::recordBrainMapPL = Parameters::register_parameter("BRAIN_CORTICALCOLUMN-recordBrainMap", false, "do you really want to make brain maps?");
std::shared_ptr<ParameterLink<int>> CorticalColumnBrain::brainMapIntervalPL = Parameters::register_parameter("BRAIN_CORTICALCOLUMN-brainMapInterval", 100, "generations between creating brain maps");
std::shared_ptr<ParameterLink<int>> CorticalColumnBrain::maxNodeInputsPL = Parameters::register_parameter("BRAIN_CORTICALCOLUMN-maxNodeInputs", 4, "maximum number of inputs a node can have");
std::shared_ptr<ParameterLink<int>> CorticalColumnBrain::maxNodeOutputsPL = Parameters::register_parameter("BRAIN_CORTICALCOLUMN-maxNodeOutputs", 4, "maximum number of outputs a node can have");
std::shared_ptr<ParameterLink<int>> CorticalColumnBrain::initialCountColumnNodesPL = Parameters::register_parameter("BRAIN_CORTICALCOLUMN-initialCountCountColumnNodes", 4, "initial number of nodes per column");
std::shared_ptr<ParameterLink<int>> CorticalColumnBrain::initialCountCrossNodesPL = Parameters::register_parameter("BRAIN_CORTICALCOLUMN-initialCountCountCrossNodes", 4, "initial number of cross connecting nodes");
std::shared_ptr<ParameterLink<int>> CorticalColumnBrain::initialCountInConnxnsPL = Parameters::register_parameter("BRAIN_CORTICALCOLUMN-initialCountCountInConnxns", 4, "initial number of in connections");
std::shared_ptr<ParameterLink<int>> CorticalColumnBrain::initialCountOutConnxnsPL = Parameters::register_parameter("BRAIN_CORTICALCOLUMN-initialCountCountOutConnxns", 4, "initial number of out connections");
std::shared_ptr<ParameterLink<bool>> CorticalColumnBrain::makeCrossNodesPL = Parameters::register_parameter("BRAIN_CORTICALCOLUMN-makeCrossNodes", false, "Determines if the brain will have nodes connecting between columns");


int CorticalColumnBrain::firstInGen = -1;

CorticalColumnBrain::CorticalColumnBrain(int _nrInNodes, int _nrOutNodes, std::shared_ptr<ParametersTable> PT_): AbstractBrain(_nrInNodes, _nrOutNodes, PT_) {
    nrOfColumns=nrOfColumnsPL->get( PT);
    widthOfColumns=widthOfColumnsPL->get( PT);
    nrOfHiddenStates=nrOfColumns*widthOfColumns;

    // columns to be added to ave file
    popFileColumns.clear();
	popFileColumns.push_back("corticalColumnBrainRegGates");
	popFileColumns.push_back("corticalColumnBrainInGates");
	popFileColumns.push_back("corticalColumnBrainOutGates");
	popFileColumns.push_back("corticalColumnBrainCrossGates");
}

std::shared_ptr<AbstractBrain> CorticalColumnBrain::makeBrain( std::unordered_map<std::string, std::shared_ptr<AbstractGenome>>&_genomes) {
    std::shared_ptr<CorticalColumnBrain> newBrain = std::make_shared<CorticalColumnBrain>(nrInputValues, nrOutputValues, PT);
    auto genomeHandler = _genomes[genomeNamePL->get(PT)]->newHandler(_genomes[genomeNamePL->get(PT)]);
    newBrain->Gates.clear();
    newBrain->wiresFromInputs.clear();
    newBrain->wiresToOutputs.clear();
    newBrain->nodes.resize(nrOfHiddenStates);
    newBrain->nextNodes.resize(nrOfHiddenStates);
    int currentSite,nextSite;

	newBrain->brainMap = "digraph G {ranksep=1.2;";
    
	currentSite=genomeHandler->readInt(0, 255);
    while(!genomeHandler->atEOG()){
		//no overlapping genes
        nextSite=genomeHandler->readInt(0,255);
        if((currentSite=42)&&(nextSite==213)){
            //regular gene
            int nrI=genomeHandler->readInt(1,maxNodeInputsPL->get(PT));
            int nrO=genomeHandler->readInt(1,maxNodeOutputsPL->get(PT));
            std::vector<int> theIs,theOs;
            std::vector<std::vector<double>> theTable;
			int column;
			int gateID;

            for(int i=0;i<maxNodeInputsPL->get(PT);i++){
                int value=genomeHandler->readInt(0,widthOfColumns-1);
				if (i < nrI) {
					theIs.push_back(value);
				}
            }
            for(int i=0;i<maxNodeOutputsPL->get(PT);i++){
                int value=genomeHandler->readInt(0,widthOfColumns-1);
				if (i < nrO) {
					theOs.push_back(value);
				}
            }

			//create table for the node
            for(int i=0;i<(1<<nrI);i++){
                std::vector<double> row;
                for(int o=0;o<nrO;o++){
                    row.push_back(Bit(genomeHandler->readDouble(-1.0, 1.0)));
                }
                theTable.push_back(row);
            }
			//copy the node
			for (column = 0; column < nrOfColumns; column++) {
                newBrain->Gates.push_back(std::make_shared<Gate>(theIs,theOs,theTable,column,newBrain->regCount));
				//newBrain->brainMap += "{";
				for (int i = 0; i < (int)theIs.size(); i++) {
					//newBrain->brainMap += "H" + std::to_string(theIs[i]) + ";";
					theIs[i] += widthOfColumns;
				}
				//newBrain->brainMap += "} -> R" + std::to_string(newBrain->regCount) + "inC" + std::to_string(column) + " -> {";
				for (int o = 0; o < (int)theOs.size(); o++) {
					//newBrain->brainMap += "Hx" + std::to_string(theOs[o]) + ";";
					theOs[o] += widthOfColumns;
				}
				//newBrain->brainMap += "};";
				//newBrain->brainMap += "R" + std::to_string(newBrain->regCount) + "inC" + std::to_string(column) + " [style=filled,color=\"" + std::to_string(1/(double(column)+1)) + " .5 .8\"];";
			}
			newBrain->regCount++;
			nextSite = genomeHandler->readInt(0, 255);
		}
		else if ((currentSite = 43) && (nextSite == 212)) {
			//in connector
			int from = genomeHandler->readInt(0, nrInputValues - 1);
			int to = genomeHandler->readInt(0, nrOfHiddenStates - 1);
			newBrain->wiresFromInputs[from].push_back(to);
			nextSite = genomeHandler->readInt(0, 255);
			newBrain->inCount++;
		}
		else if ((currentSite = 44) && (nextSite == 211)) {
			//out connector
			int from = genomeHandler->readInt(0, nrOfHiddenStates - 1);
			int to = genomeHandler->readInt(0, nrOutputValues - 1);
			newBrain->wiresToOutputs[from].push_back(to);
			nextSite = genomeHandler->readInt(0, 255);
			newBrain->outCount++;
		}
		else if ((currentSite = 45) && (nextSite == 210) && (makeCrossNodesPL->get(PT) == true)) {
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
		}
		currentSite = nextSite;

	}

	recordBrainMap = recordBrainMapPL->get(PT);
	brainMapInterval = brainMapIntervalPL->get(PT);
	if (recordBrainMap == true && firstInGen != Global::update && Global::update % brainMapInterval == 0) {
		newBrain->brainMap = "diGraph G {ranksep=1.2;";
		for (auto G : Gates) {
			if (G->c == -1) {
				newBrain->brainMap += "{";
				for (int i = 0; i < (int)G->I.size(); i++) {
					newBrain->brainMap += "H" + std::to_string(G->I[i]) + ";";
				}
				newBrain->brainMap += "} -> C" + std::to_string(G->g) + " -> {";
				for (int o = 0; o < (int)G->O.size(); o++) {
					newBrain->brainMap += "Hx" + std::to_string(G->O[o]) + ";";
				}
				newBrain->brainMap += "};";
			}
			else {
				newBrain->brainMap += "{";
				for (int i = 0; i < (int)G->I.size(); i++) {
					newBrain->brainMap += "H" + std::to_string(G->I[i]) + ";";
				}
				newBrain->brainMap += "} -> R" + std::to_string(G->g) + "inC" + std::to_string(G->c) + " -> {";
				for (int o = 0; o < (int)G->O.size(); o++) {
					newBrain->brainMap += "Hx" + std::to_string(G->O[o]) + ";";
				}
				newBrain->brainMap += "};";
				newBrain->brainMap += "R" + std::to_string(G->g) + "inC" + std::to_string(G->c) + " [style=filled,color=\"" + std::to_string(1 / (double(G->c) + 1)) + " .5 .8\"];";
			}

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
			newBrain->brainMap += "}";
		}

		newBrain->brainMap += "}";
		FileManager::writeToFile("brainMapsUpdate" + std::to_string(Global::update) + ".dot", newBrain->brainMap, "");
		firstInGen = Global::update;
	}

    return newBrain;
}

void CorticalColumnBrain::resetBrain() {
    AbstractBrain::resetBrain();
    for(int i=0;i<(int)nextNodes.size();i++){
        nextNodes[i]=0.0;
        nodes[i]=0.0;
    }
}

void CorticalColumnBrain::update() {
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

std::string CorticalColumnBrain::description() {
    return "CorticalColumnBrain\n";
    
}

DataMap CorticalColumnBrain::getStats(std::string &prefix) { 
	DataMap dataMap;
	dataMap.set(prefix + "corticalColumnBrainRegGates", regCount);
	dataMap.set(prefix + "corticalColumnBrainInGates", inCount);
	dataMap.set(prefix + "corticalColumnBrainOutGates", outCount);
	dataMap.set(prefix + "corticalColumnBrainCrossGates", crossCount);
	/*std::cout << "\nregCount: " << regCount;
	std::cout << "\ninCount: " << inCount;
	std::cout << "\noutCount: " << outCount;
	std::cout << "\ncrossCount: " << crossCount << std::endl;*/
	return dataMap; 
}

void CorticalColumnBrain::initializeGenomes(std::unordered_map<std::string, std::shared_ptr<AbstractGenome>> &_genomes) {
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
	for (int i = 0; i < initialCountCrossNodesPL->get(PT); i++) {
		genomeHandler->randomize();
		genomeHandler->writeInt(45, 0, 255);
		genomeHandler->writeInt(210, 0, 255);
	}
}

std::shared_ptr<AbstractBrain> CorticalColumnBrain::makeCopy(std::shared_ptr<ParametersTable> PT_) {
    if (PT_ == nullptr) {
        PT_ = PT;
    }
    auto newBrain = std::make_shared<CorticalColumnBrain>(nrInputValues, nrOutputValues, PT_);
    newBrain->Gates=Gates;
    newBrain->wiresFromInputs=wiresFromInputs;
    newBrain->wiresToOutputs=wiresToOutputs;
    newBrain->nodes=nodes;
    newBrain->nextNodes=nextNodes;
    return newBrain;
}


/*** GATE defition below here***/
Gate::Gate(std::vector<int> &inAddr, std::vector<int> &outAddr,std::vector<std::vector<double>> &theTable,int xx, int yy){
    I=inAddr;
    O=outAddr;
    table=theTable;
	c = xx;
	g = yy;
}

void Gate::update(std::vector<double> &nodes, std::vector<double> &nextNodes){
    int input=0;
    for(auto i:this->I)
        input=(input<<1)+Bit(nodes[i]);
    for(int o=0;o<(int)table[input].size();o++)
        nextNodes[O[o]]+=Bit(table[input][o]);
}
