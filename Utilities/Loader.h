// author : cgnitash
// Loader.h contains decl of Loader class

#pragma once

#include <map>
#include <regex>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

// implements the functionality specified in
// https://github.com/Hintzelab/MABE/wiki/Population-Loading
//
class Loader {

public:
  // public method returns a vector of pairs
  // each pair is a long (where a value < 1 indicates whether that this is a
  // random organim, and a positive number indicates it is actually loaded from
  // file) and a map of (attribute-value) pairs for the particular organism
  std::vector<std::pair<long, std::unordered_map<std::string, std::string>>>
  loadPopulation(const std::string &);

private:
  // helper struct that stores all of the organisms info pulled from
  // organisms_files and data_files
  struct organism {
    std::unordered_map<std::string, std::string> attributes;
    std::string
        from_file; // name of organism file this organism was pulled from
    long orig_ID;  // ID in the original file
  };

  std::vector<organism> all_organisms; // literally

  // (invisibly) reserved token names for temporaries
  // this needs a unique name guarantee - enforced
  // by requiring user names to NOT begin with __
  const std::string tk_name = "__TK42PL__";

  // used to create unique temporary token names
  long tk_counter;

  //  std::vector<std::string>
  //      all_possible_file_names; // literally (see constructor)

  std::map<std::string, std::vector<std::vector<long>>>
      collection_org_lists; // key: collection name (user-defined or
                            // temporary token) - value: list of orgs in
                            // each population of the collection

  // *** 	methods   *** 

  // load population specified in a .plf file
  std::string loadFromFile(const std::string &);

  // expand user inputted wildcards into list
  // of all matching filenames
  std::vector<std::string> expandFiles(const std::string &);

  std::pair<long, long> generatePopulation(const std::string &);

  // parse all assignments of collection-expressions to user-defined variables
  void parseAllCommands(std::string);

  // unpack and parse entire collection-expression
  std::vector<std::vector<long>> parseExpression(std::string);

  // parse single collection
  std::vector<std::vector<long>>
  parseCollection(const std::string &);

  // clean comments and check for invalid syntax
  std::string cleanLines(std::ifstream &);

  // a level of indirection so that all possible files the user might
  // need are parsed exactly once
  std::string findAndGenerateAllFiles(std::string all_lines);

  // simple syntactic check for nexted braces to avoid parsing issues
  bool balancedBraces(std::string);

  // for interactive screen display
  void showFinalPopulation(std::vector<long>);

  // methods for all keywords
  std::vector<std::vector<long>> keywordCollapse(const std::string &);
  std::vector<std::vector<long>> keywordRandom(long number);
  std::vector<std::vector<long>> keywordDefault(long number);
  std::vector<std::vector<long>> keywordGreatest(size_t, std::string,
                                                 const std::string &);
  std::vector<std::vector<long>> keywordLeast(size_t, std::string,
                                              const std::string &);
  std::vector<std::vector<long>> keywordAny(size_t, const std::string &);
  std::vector<std::vector<long>> keywordDuplicate(size_t, const std::string &);
  std::vector<std::vector<long>> keywordMatch(std::string, std::string,
                                              const std::string &);

  // make sure user-defined variable names actually exist  
  void checkTokenExistence(const std::string &);

  // strictly to debug all_organisms entries
  void printOrganism(long);
};
