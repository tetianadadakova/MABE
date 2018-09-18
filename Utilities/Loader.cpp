// author : cgnitash, joryschossau
// Loader.cpp contains implementation of population loading scripting language

#include "Loader.h"
#include "CSV.h"
#include "Filesystem.h"

#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <random>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

std::string dataVersionOfFilename(const std::string &filename) {
  std::string ext(filename.substr(filename.rfind(".")));
  int orgposIfExist = filename.rfind("organisms");
  if (orgposIfExist != -1)
    return filename.substr(0, orgposIfExist) + "data" +
           filename.substr(orgposIfExist + 9);
  else
    return filename.substr(0, filename.rfind(".")) + "_data" + ext;
}

std::string Loader::loadFromFile(const std::string &loader_file_name) {
  std::ifstream flines(loader_file_name);
  if (!flines.is_open()) {
    std::cout << " error: population loader file " << loader_file_name
              << " does not exist " << std::endl
              << " Run MABE with -l to generate population loader file"
              << std::endl;
    exit(1);
  }
  std::cout << "Creating population from " << loader_file_name << "\n";
  return cleanLines(flines);
}

// load population specified in settings.cfg
std::vector<std::pair<long, std::unordered_map<std::string, std::string>>>
Loader::loadPopulation(const std::string &loader_option) {

  static const std::regex plf_file(R"(.*\.plf)");
  auto all_lines =
      std::regex_match(loader_option, plf_file)
          ? loadFromFile(loader_option) // if config specifies a .plf file
          : loader_option;              // else config has in-place .plf syntax

  // replace all filenames (expanded or otherwise) with temporary tokens
  all_lines = findAndGenerateAllFiles(all_lines);

  tk_counter = 0; // initialize temporary token counter

  parseAllCommands(all_lines);

  // .plf must contain a variable called MASTER
  if (collection_org_lists.find("MASTER") == collection_org_lists.end()) {
    std::cout << "error: Must load from variable named MASTER" << std::endl;
    exit(1);
  }
  // and it must be a collection with exactly 1 population
  if (collection_org_lists.at("MASTER").size() != 1) {
    std::cout
        << "error: variable named MASTER must contain exactly one population"
        << std::endl;
    exit(1);
  }

  // store the final population of organisms
  auto final_orgs = collection_org_lists.at("MASTER")[0];

  // inform the user which organisms have been loaded
  showFinalPopulation(final_orgs);

  // convert the organims attributes into a form palatable? to main.cpp
  std::vector<std::pair<long, std::unordered_map<std::string, std::string>>>
      final_population;
  std::transform(
      final_orgs.begin(), final_orgs.end(),
      std::back_inserter(final_population), [this](long org) {
        return org < 0 // default organism
                   ? make_pair(org,
                               std::unordered_map<std::string, std::string>())
                   // loaded organism
                   : make_pair(org, all_organisms.at(org).attributes);
      });

  return final_population;
} // end Loader::loadPopulation

void Loader::showFinalPopulation(std::vector<long> orgs) {
  std::cout << "\n"
            << "Loading "
            << std::accumulate(orgs.begin(), orgs.end(), 0,
                               [](long acc, long i) { return acc + (i == -1); })
            << " Random organisms\n";
  std::cout << "Loading "
            << std::accumulate(orgs.begin(), orgs.end(), 0,
                               [](long acc, long i) { return acc + (i == -2); })
            << " Default organisms\n";
  std::map<std::string, std::vector<long>> orgs_from_files;
  for (auto i : orgs)
    if (i > -1)
      orgs_from_files[all_organisms.at(i).from_file].push_back(
          all_organisms.at(i).orig_ID);
  for (auto &f : orgs_from_files) {
    std::cout << "From file " << f.first << "\n  Loading organisms with IDs : ";
    for (auto k : f.second)
      std::cout << k << " ";
    std::cout << "\n";
  }
}

void Loader::parseAllCommands(std::string all_lines) {
  // split into commands. These are all collection to variable assignments
  static const std::regex command(R"((?:(\w+)\s*=)(.*?)(?=(?:(?:\w+\s*=))|$))");
  // match one of the collection-assignments but don't capture
  // lookahead for another collection-assignment (without capturing) or end of
  // line
  for (std::sregex_iterator end,
       i = std::sregex_iterator(all_lines.begin(), all_lines.end(), command);
       i != end; i++) {
    std::smatch m = *i;
    auto expr = m[2].str();
    auto name = m[1].str();
    collection_org_lists[name] = parseExpression(expr);
  }
}

std::string Loader::cleanLines(std::ifstream &flines) {

  std::string all_lines = " ", line;

  // read lines, strip out all trailing comments, and create one long string.
  static const std::regex comments("#.*");
  while (getline(flines, line)) {
    if (!line.empty()) {
      std::string clean_line = std::regex_replace(line, comments, "");
      all_lines += " " + clean_line;
    }
  }

  // check that file must start with an assignment
  static const std::regex garbage(R"(^\s*\w+\s*=.*$)");
  if (!std::regex_match(all_lines, garbage)) {
    std::cout << " error : Loader file contains unrezognized text at beginning "
              << std::endl;
    exit(1);
  }
  return all_lines;
}

std::string Loader::findAndGenerateAllFiles(std::string all_lines) {

  // stores all file names that correspond to each collection
  std::map<std::string, std::vector<std::string>> collection_of_files;
  // every file the user might refer to
  std::set<std::string> actual_files;

  // get all file names . These must all be in single-qoutes and can be
  // wildcarded
  static const std::regex quoted_files("'([^']*)'");
  std::smatch m;
  while (std::regex_search(all_lines, m, quoted_files)) {
    // create a new token, and bump the token counter
    std::string new_tk = tk_name + std::to_string(tk_counter++);
    // replace quoted file names with temporary token name
    all_lines = m.prefix().str() + " " + new_tk + " " + m.suffix().str();
    // expand the filename in case there are wildcards
    auto exp_files = expandFiles(m[1].str());
    //  update the local list of files associated with collection name
    collection_of_files[new_tk] = exp_files;
    // add list of files to global file list
    actual_files.insert(exp_files.begin(), exp_files.end());
  }

  // simple map from filename -> (start index of org in all_organisms, number of
  // sequential organisms from start) efficiently keeps track of organisms
  std::map<std::string, std::pair<long, long>> file_contents;

  // each file is read once and each organism from every file is
  // assigned a unique ID
  // this creates a SINGLE list of organisms that can be efficiently accessed
  for (const auto &file : actual_files) {
    std::cout << "Parsing file " << file << "..." << std::endl;
    file_contents[file] = generatePopulation(file);
  }

  // create all collections from file names
  // Note: collections internally only hold on to the unique index
  for (const auto &files : collection_of_files) {
    for (const auto &file : files.second) {
      // create all indices within the range generated for this file
      std::vector<long> sinf(file_contents.at(file).second);
      std::iota(sinf.begin(), sinf.end(), file_contents.at(file).first);
      // add this collection(indices) to global list
      collection_org_lists[files.first].push_back(sinf);
    }
  }
  // returns .plf script with all filenames removed
  return all_lines;
}

// implements a recursive parser, that stores sub-expressions in a stack, and
// evaluates them bottom-up
std::vector<std::vector<long>> Loader::parseExpression(std::string expr) {

  // a simple linear time check for balanced braces. Strictly speaking not
  // necessary, but it's a very cheap check that gives a much better error
  // message, than if caught during parsing
  if (!balancedBraces(expr)) {
    std::cout << " Expression " << expr << " does not have balanced braces "
              << std::endl;
    exit(1);
  }

  // make the top of local stack evaluate to the correct expression
  expr = "{" + expr + "}";

  // stores all sub-expressions into temporary tokens
  std::vector<std::pair<std::string, std::string>> local_tk_stack;
  // match lowest-level (non-decomposable) sub-expression
  static const std::regex braces(R"(\{([^\{\}]*)\})");
  std::smatch m;
  // for each nested sub-expression - resolve braces ...
  while (std::regex_search(expr, m, braces)) {
    // create a new token, and bump token counter
    std::string new_tk = tk_name + std::to_string(tk_counter++);
    // add token to local stack of tokens
    local_tk_stack.emplace_back(new_tk, m[1].str());
    // replace braced sub-expression with token
    expr = m.prefix().str() + " " + new_tk + " " + m.suffix().str();
  } // repeat for all braces ...

  // evaluate tokens in local stack
  // each sub-expression in a collection is separated by a :
  static const std::regex colon_sep(R"((.+?)(?:\:|$))");
  for (const auto &tk : local_tk_stack) {
    // collection that needs to be filled
    std::vector<std::vector<long>> coll;
    // expression corresponding to the token
    std::string tkn = tk.second;
    //	evaluate each sub expression
    for (std::sregex_iterator end,
         i = std::sregex_iterator(tkn.begin(), tkn.end(), colon_sep);
         i != end; i++) {
      // extract the expression text
      std::smatch sub_expr = *i;
      std::string token = sub_expr[1].str();
      // remove syntactic whitespace
      token.erase(std::remove_if(token.begin(), token.end(), ::isspace),
                  token.end());
      auto p = collection_org_lists.find(token) != collection_org_lists.end()
                   ? // if sub-expression is already evaluated (e.g. repeated
                     // filename)
                   collection_org_lists[token]
                   : // else it needs to be parsed
                   parseCollection(sub_expr[1].str());
      // insert the population corresponding to the expression into the
      // collection
      coll.insert(coll.end(), p.begin(), p.end());
    }
    // add the collection on the stack to global collections list
    collection_org_lists[tk.first] = coll;
  }

  // return collection at top of stack - this must be the complete RHS
  // expression from a user-defined variable assignment
  return collection_org_lists[local_tk_stack.back().first];
} // end  Loader::parseExpression

bool Loader::balancedBraces(std::string s) {
  long k = 0;
  for (auto &c : s)
    switch (c) {
    case '{':
      k++;
      break;
    case '}':
      k--;
      if (k < 0)
        return false;
      break;
    }
  return k == 0;
}

std::vector<std::vector<long>>
Loader::parseCollection(const std::string &expr) {

  // each sub-expression can have one of the following keyword commands
  {
    static const std::regex command_collapse(R"(^\s*collapse\s+(\w+)\s*$)");
    std::smatch match;
    if (std::regex_match(expr, match, command_collapse))
      return keywordCollapse(match[1].str());
  }

  {
    static const std::regex command_random(R"(^\s*random\s+(\d+)\s*$)");
    std::smatch match;
    if (std::regex_match(expr, match, command_random))
      return keywordRandom(stol(match[1].str()));
  }

  {
    static const std::regex command_default(R"(^\s*default\s+(\d+)\s*$)");
    std::smatch match;
    if (std::regex_match(expr, match, command_default))
      return keywordDefault(stol(match[1].str()));
  }

  {
    static const std::regex command_greatest(
        R"(^\s*greatest\s+(\d+)\s+by\s+(\w+)\s+from\s+(\w+)\s*$)");
    std::smatch match;
    if (std::regex_match(expr, match, command_greatest))
      return keywordGreatest(stoul(match[1].str()), match[2].str(),
                             match[3].str());
  }

  {
    static const std::regex command_least(
        R"(^\s*least\s+(\d+)\s+by\s+(\w+)\s+from\s+(\w+)\s*$)");
    std::smatch match;
    if (std::regex_match(expr, match, command_least))
      return keywordLeast(stol(match[1].str()), match[2].str(), match[3].str());
  }

  {
    static const std::regex command_any(
        R"(^\s*any\s+(\d+)\s+from\s+(\w+)\s*$)");
    std::smatch match;
    if (std::regex_match(expr, match, command_any))
      return keywordAny(stol(match[1].str()), match[2].str());
  }

  {
    static const std::regex command_match(
        R"(^\s*match\s+(\w+)\s+where\s+(\S+)\s+from\s+(\w+)\s*$)");
    std::smatch match;
    if (std::regex_match(expr, match, command_match))
      return keywordMatch(match[1].str(), match[2].str(), match[3].str());
  }

  {
    static const std::regex command_duplicate(R"(^\s*(\d+)\s*\*\s*(\w+)\s*$)");
    std::smatch match;
    if (std::regex_match(expr, match, command_duplicate))
      return keywordDuplicate(stoul(match[1].str()), match[2].str());
  }

  // if no keyword matches
  std::cout << " error: syntax error while trying to resolve " << std::endl
            << expr << std::endl;
  exit(1);

} // end Loader::parse_token

void Loader::checkTokenExistence(const std::string &token_name) {
  if (collection_org_lists.find(token_name) == collection_org_lists.end()) {
    std::cout << "Unrecognised token " << token_name << std::endl;
    exit(1);
  }
}
std::vector<std::vector<long>>
Loader::keywordDuplicate(size_t value, const std::string &resource) {

  checkTokenExistence(resource);

  std::vector<std::vector<long>> coll;
  // make 'value' number of copies for each population in the collection
  for (const auto &pop : collection_org_lists[resource]) {
    auto num = value;
    while (num--)
      coll.push_back(pop);
  }
  return coll;
}

std::vector<std::vector<long>>
Loader::keywordMatch(std::string attribute, std::string value,
                     const std::string &resource) {

  checkTokenExistence(resource);

  std::vector<std::vector<long>> coll;
  for (const auto &p : collection_org_lists.at(resource)) {
    const auto from_pop = p;
    for (const auto &o : from_pop)
      if (all_organisms.at(o).attributes.find(attribute) ==
          all_organisms.at(o).attributes.end()) {
        std::cout << "error: while trying to match " << value << "  "
                  << resource << " contains organisms without attribute "
                  << attribute << std::endl;
        exit(1);
      }
    std::vector<long> pop;
    std::copy_if(std::begin(from_pop), std::end(from_pop),
                 std::back_inserter(pop), [&](long index) {
                   return all_organisms.at(index).attributes.at(attribute) ==
                          value;
                 });
    coll.push_back(pop);
  }
  return coll;
}

std::vector<std::vector<long>>
Loader::keywordGreatest(size_t number, std::string attribute,
                        const std::string &resource) {

  checkTokenExistence(resource);

  std::vector<std::vector<long>> coll;
  for (const auto &p : collection_org_lists.at(resource)) {
    const auto from_pop = p;
    if (from_pop.size() < number) {
      std::cout << "error: trying to get greatest " << number
                << " from collection, but  " << resource
                << " contains a population that does not have sufficient "
                << " organisms" << std::endl;
      exit(1);
    }
    for (const auto &o : from_pop)
      if (all_organisms.at(o).attributes.find(attribute) ==
          all_organisms.at(o).attributes.end()) {
        std::cout << "error:  trying to get greatest " << number
                  << " from collection, but  " << resource
                  << " contains organisms without attribute " << attribute
                  << std::endl;
        exit(1);
      }
    std::vector<long> pop(number);
    std::partial_sort_copy(
        from_pop.begin(), from_pop.end(), pop.begin(), pop.end(),
        [&](long lhs, long rhs) {
          return std::stod(all_organisms.at(lhs).attributes.at(attribute)) >
                 std::stod(all_organisms.at(rhs).attributes.at(attribute));
        });
    coll.push_back(pop);
  }
  return coll;
}

std::vector<std::vector<long>>
Loader::keywordLeast(size_t number, std::string attribute,
                     const std::string &resource) {

  checkTokenExistence(resource);

  std::vector<std::vector<long>> coll;

  for (const auto &p : collection_org_lists.at(resource)) {
    const auto from_pop = p;
    if (from_pop.size() < number) {
      std::cout << "error: trying to get least" << number
                << " from collection, but  " << resource
                << " contains a population that does not have sufficient "
                << " organisms" << std::endl;
      exit(1);
    }
    for (const auto &o : from_pop)
      if (all_organisms.at(o).attributes.find(attribute) ==
          all_organisms.at(o).attributes.end()) {
        std::cout << "error:  trying to get least" << number
                  << " from collection, but  " << resource
                  << " contains organisms without attribute " << attribute
                  << std::endl;
        exit(1);
      }
    std::vector<long> pop(number);
    std::partial_sort_copy(
        from_pop.begin(), from_pop.end(), pop.begin(), pop.end(),
        [&](long lhs, long rhs) {
          return std::stod(all_organisms.at(lhs).attributes.at(attribute)) <
                 std::stod(all_organisms.at(rhs).attributes.at(attribute));
        });
    coll.push_back(pop);
  }
  return coll;
}

std::vector<std::vector<long>> Loader::keywordAny(size_t number,
                                                  const std::string &resource) {
  checkTokenExistence(resource);

  std::vector<std::vector<long>> coll;
  for (const auto &p : collection_org_lists.at(resource)) {
    auto from_pop = p;
    if (from_pop.size() < number) {
      std::cout << "error: trying to get any" << number
                << " from collection, but  " << resource
                << " contains a population that does not have sufficient "
                << " organisms" << std::endl;
      exit(1);
    }
    // Latent bug - not reproducible randomness :(
    std::shuffle(from_pop.begin(), from_pop.end(), std::random_device());
    std::vector<long> pop(from_pop.begin(), from_pop.begin() + number);
    coll.push_back(pop);
  }
  return coll;
}

std::vector<std::vector<long>>
Loader::keywordCollapse(const std::string &resource) {

  checkTokenExistence(resource);
  std::vector<std::vector<long>> coll;
  std::vector<long> pop;
  for (const auto &c : collection_org_lists.at(resource)) {
    pop.insert(pop.end(), c.begin(), c.end());
  }
  coll.push_back(pop);
  return coll;
}

// these 2 functions are intended to have different semantics, but may not be
// distinguished by main.cpp
std::vector<std::vector<long>> Loader::keywordRandom(long number) {
  std::vector<std::vector<long>> coll;
  std::vector<long> pop(number, -1);
  coll.push_back(pop);
  return coll;
}

std::vector<std::vector<long>> Loader::keywordDefault(long number) {
  std::vector<std::vector<long>> coll;
  std::vector<long> pop(number, -2);
  coll.push_back(pop);
  return coll;
}

std::vector<std::string> Loader::expandFiles(const std::string &file) {

  //	store file names after wildcard expansion
  std::vector<std::string> result;
  // expand wildcards
  getFilesMatchingRelativePattern(file, result);

  if (result.empty()) {
    std::cout << " error: " << file << " does not match any files" << std::endl;
    exit(1);
  }
  return result;
} // end Loader::expandFiles

std::pair<long, long> Loader::generatePopulation(const std::string &file_name) {

  // store organism file in memory-mapped CSV
  auto org_file_data = CSV(file_name);
  // setup the range of indices needed to identify all organisms from this file
  auto file_contents_pair = std::make_pair(long(all_organisms.size()),
                                           long(org_file_data.row_count()));

  /*
  static const std::regex valid_org_name(R"((.*)_organisms(_\d+)?.csv$)");
  std::smatch match_org;
  std::regex_match(file_name, match_org, valid_org_name);
  //auto data_file_name = std::regex_replace(file_name, valid_org_name,
  //                                         match_org[1].str() + "_data" +
  //                                             match_org[2].str() + ".csv");
*/
  //  std::map<long, std::map<std::string, std::string>> data_file_data;

  // search for the _data version of the organism file
  std::string data_file_name(dataVersionOfFilename(file_name));
  if (fileExists(data_file_name)) {
    // store _data file in memory_mapped CSV
    auto data_file_data = CSV(data_file_name);
    // merge the _data file data into the organism file data
    org_file_data.merge(data_file_data, "ID");
  }

  // for each ID in the organism+_data data
  for (auto const &id : org_file_data.singleColumn("ID")) {
    // create an internal organism
    organism org;
    // store the unique ID
    org.orig_ID = std::stol(id);
    // store the orginal file from which it was pulled
    org.from_file = file_name;
    // stick all the attributes into the organism
    for (const auto &attribute : org_file_data.columns()) {
      // making sure to use the per-file-unique-ID
      org.attributes.insert(
          make_pair(attribute, org_file_data.lookUp("ID", id, attribute)));
    }
    all_organisms.push_back(org);
  }

  return file_contents_pair;
} // end Loader::generatePopulation

/*
// reads organisms or data file. return key of ID to map of attributes to values
// attributes do NOT include ID
std::map<long, std::map<std::string, std::string>>
Loader::getAttributeMap(const std::string &file_name) {

  std::map<long, std::map<std::string, std::string>> result;

  // check if organsims or data file
  static const std::regex org_or_data(R"((.*)_(data|organisms)(_\d+)?.csv$)");
  std::smatch match_org;
  if (!std::regex_match(file_name, match_org, org_or_data)) {
    std::cout << " error: unrecognized file name format " << file_name
              << std::endl
              << " Was this file generated by MABE? " << std::endl;
    exit(1);
  }
  std::ifstream file(file_name);
  if (!file.is_open()) {
    std::cout << " error: unable to load" << file_name << std::endl;
    exit(1);
  }

  std::string attr_names;
  getline(file, attr_names);
  static const std::regex each_attribute(R"([\w|:]+)");
  std::vector<std::string> attribute_names;

  for (std::sregex_iterator end,
       i = std::sregex_iterator(attr_names.begin(), attr_names.end(),
                                each_attribute);
       i != end; i++) {
    attribute_names.push_back((*i).str());
  }

  if (std::find(attribute_names.begin(), attribute_names.end(), "ID") ==
      attribute_names.end()) {
    std::cout << " error: no ID for organisms in file " << file_name
              << std::endl;
    exit(1);
  }

  // checking for MABE csv-ness
  static const std::regex mabe_csv_regex(
      R"((([-\.\d]+)(?:,|$))|("\[)|(([-\.\d]+\]")(?:,|$)))");
  //	std::regex mabe_csv_regex(R"(("[^"]+"|[^,]+)(,|$))");  // does not work
  //because of
  //	https://gcc.gnu.org/bugzilla/show_bug.cgi?id=61582
  std::string org_details;
  while (getline(file, org_details)) {
    std::map<std::string, std::string> temp_result;
    long k = 0;
    auto in_quotes = false;
    std::string quote_str;
    for (std::sregex_iterator end,
         i = std::sregex_iterator(org_details.begin(), org_details.end(),
                                  mabe_csv_regex);
         i != end; i++) {
      std::smatch m = *i;

      if (m[1].length())
        if (in_quotes == false)
          temp_result[attribute_names.at(k++)] = m[2].str();
        else
          quote_str += m[1].str();
      else if (m[3].length()) {
        in_quotes = true;
        quote_str += m[3].str();
      } else if (m[5].length()) {
        quote_str += m[5].str();
        temp_result[attribute_names.at(k++)] = quote_str;
        in_quotes = false;
        quote_str = "";
      } else {
        std::cout << " error : something wrong with mabe csv-ness "
                  << std::endl;
        exit(1);
      }
    }
    auto orig_ID = std::stol(temp_result.at("ID"));
    temp_result.erase("ID");
    result[orig_ID] = temp_result;
  }
  file.close();
  return result;
} // end Loader::getAttributeMap
*/


void Loader::printOrganism(long i) {

  // strictly for debugging purposes 
  if (i != -1)
    std::cout << "\tID: " << all_organisms.at(i).orig_ID << " from file "
              << all_organisms.at(i).from_file << std::endl;
  else
    std::cout << "\trandom default organism" << std::endl;
} // end Loader::printOrganism
