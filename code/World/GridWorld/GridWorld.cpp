//  MABE is a product of The Hintze Lab @ MSU
//     for general research information:
//         hintzelab.msu.edu
//     for MABE documentation:
//         github.com/Hintzelab/MABE/wiki
//
//  Copyright (c) 2015 Michigan State University. All rights reserved.
//     to view the full license, visit:
//         github.com/Hintzelab/MABE/wiki/License

#include "GridWorld.h"

// this is how you setup a parameter in MABE, the function Parameters::register_parameter()takes the
// name of the parameter (catagory-name), default value (which must conform with the type), a the useage message
shared_ptr<ParameterLink<int>> GridWorld::evaluationsPerGenerationPL =
    Parameters::register_parameter("WORLD_CLEAN-evaluationsPerGeneration", 30,
    "how many times should each organism be tested in each generation?");

// the constructor gets called once when MABE starts up. use this to set things up
GridWorld::GridWorld(shared_ptr<ParametersTable> PT_) : AbstractWorld(PT_) {
    
    //localize a parameter value for faster access
    evaluationsPerGeneration = evaluationsPerGenerationPL->get(PT);
    
    // popFileColumns tell MABE what data should be saved to pop.csv files
	popFileColumns.clear();
    popFileColumns.push_back("score");
    //popFileColumns.push_back("count_up");
    //popFileColumns.push_back("count_down");
    //popFileColumns.push_back("count_left");
    //popFileColumns.push_back("count_right");
    popFileColumns.push_back("out0");
    popFileColumns.push_back("out1");
    popFileColumns.push_back("out2");
    popFileColumns.push_back("out3");
    popFileColumns.push_back("out4");
    popFileColumns.push_back("out5");
    popFileColumns.push_back("out6");
    popFileColumns.push_back("out7");
    popFileColumns.push_back("out8");
    popFileColumns.push_back("outs");
}

// the evaluate function gets called every generation. evaluate should set values on organisms datamaps
// that will be used by other parts of MABE for things like reproduction and archiving
auto GridWorld::evaluate(map<string, shared_ptr<Group>>& groups, int analyze, int visualize, int debug) -> void {

    int popSize = groups[groupName]->population.size(); 
    
    // in this world, organisms donot interact, so we can just iterate over the population
    for (int i = 0; i < popSize; i++) {

        // create a shortcut to access the organism and organisms brain
        auto org = groups[groupName]->population[i];
        auto brain = org->brains[brainName]; 
        
        double score = 0;
        int count_up = 0;
        int count_down = 0;
        int count_left = 0;
        int count_right = 0;
        
        //double score_curr_max = std::numeric_limits<double>::min();
        const int rows = 3;
        const int cols = 3;
        int total_cells = rows * cols;
        //std::vector<std::vector<bool>> best_grid (rows, std::vector<bool>(cols, false));
         
        // evaluate this organism some number of times based on evaluationsPerGeneration
        for (int t = 0; t < evaluationsPerGeneration; t++) {
            
            // clear the brain - resets brain state including memory
            brain->resetBrain();

            //int in0 = t % 2;                // this will switch this input on and off
            //int in1 = Random::getInt(0,1);  // get a random number in the range [0,1]
            
            /*
              grid:
                  c0 c1 c2
              r0  1  0  0
              r1  1  0  0
              r2  1  0  0
             
            out:
              0 1 2 1 0 3 2 1 1
              0 - up, 1 - down, 2 - left, 3 - right
            */
            
            std::vector<std::vector<bool>> grid (rows, std::vector<bool>(cols, false));
            
            // set input based on agent and world state (can be int or double)
            //brain->setInput(0, in0); // set input 0 to in0
            //brain->setInput(1, in1); // set input 1 to in1

            // run a brain update (i.e. ask the brain to convert it's inputs into outputs)
            brain->update();

            // read brain output 0
            //double out0 = brain->readOutput(0);
            std::vector<int> out;
            const int steps = 9;
            for (int s = 0; s < steps; ++s) {
                out.push_back((int)brain->readOutput(s)); // 0 to 4?
                org->dataMap.append("outs", out.back());
            }

            /*
            std::cout << "out0 = " << out[0] << std::endl;
            std::cout << "out1 = " << out[1] << std::endl;
            std::cout << "out2 = " << out[2] << std::endl;
            std::cout << "out3 = " << out[3] << std::endl;
            std::cout << "out4 = " << out[4] << std::endl;
            std::cout << "out5 = " << out[5] << std::endl;
            std::cout << "out6 = " << out[6] << std::endl;
            std::cout << "out7 = " << out[7] << std::endl;
            std::cout << "out8 = " << out[8] << std::endl;
            */
            
            // update agent or world state here
            // brain should output 1 if in1 == in2, else output 0
            //if ( (out0 > 0) == (in0 == in1) ) {
            //    score += 1;
            //}
            int row = 0;
            int col = 0;
            for (auto dir : out) {
                if (row < 0)
                    row = 0;
                if (col < 0)
                    col = 0;
                if (row > grid.size() - 1)
                    row = grid.size() - 1;
                if (col > grid[0].size() - 1)
                    col = grid[0].size() - 1;
                
                if (grid[row][col] == false) {
                    //score++;
                    grid[row][col] = true;
                } else {
                    //score--;
                }
                
                // e.g. 0 - up, 1 - down, 2 - left, 3 - right
                if (dir == 0)
                    row--; // up
                else if (dir == 1)
                    row++; // down
                else if (dir == 2)
                    col--; // left
                else // dir == 3
                    col++; // right
            }
            
            int num_visited = 0;
            for (auto vec : grid)
                for (auto c : vec)
                    if (c == true) num_visited++;

            //std::cout << "total_cells = " << total_cells << std::endl;
            //std::cout << "num_visited = " << num_visited << std::endl;
            //score -= (total_cells - num_visited);
            score = num_visited;
            
            // add output behavior to organisms data
            // this will create a list of out0 values in the dataMap the average of this list will be
            // used to create the pop file in other files, the list will be saved along with the average
            org->dataMap.append("out0", out[0]);
            org->dataMap.append("out1", out[1]);
            org->dataMap.append("out2", out[2]);
            org->dataMap.append("out3", out[3]);
            org->dataMap.append("out4", out[4]);
            org->dataMap.append("out5", out[5]);
            org->dataMap.append("out6", out[6]);
            org->dataMap.append("out7", out[7]);
            org->dataMap.append("out8", out[8]);
            
            count_up = std::count (out.begin(), out.end(), 0);
            count_down = std::count (out.begin(), out.end(), 1);
            count_left = std::count (out.begin(), out.end(), 2);
            count_right = std::count (out.begin(), out.end(), 3);
            
            // save best grid
            //if (score > score_curr_max) {
            //    best_grid = grid;
            //}
        }
        // add score to organisms data
        // it can be expensive to access dataMap too often. also, here we want score to be the sum of the correct answers
        org->dataMap.append("score", score);
        /*org->dataMap.append("count_up", count_up);
        org->dataMap.append("count_down", count_down);
        org->dataMap.append("count_left", count_left);
        org->dataMap.append("count_right", count_right);*/
        
        //show best grid
        /*std::cout << "grid:\n";
        for (auto vec : best_grid) {
            for (auto c : vec) {
                std::cout << c << " ";
            }
            std::cout << "\n";
        }
        std::cout << std::endl;*/
         

    } // end of population loop
}

// the requiredGroups function lets MABE know how to set up populations of organisms that this world needs
auto GridWorld::requiredGroups() -> unordered_map<string,unordered_set<string>> {
	return { { groupName, { "B:"+brainName+",0,9" } } };
        
        // this tells MABE to make a group called "root::" with a brain called "root::" that takes 2 inputs and has 1 output
        // "root::" here also indicates the namespace for the parameters used to define these elements.
        // "root::" is the default namespace, so parameters defined without a namespace are "root::" parameters
}
