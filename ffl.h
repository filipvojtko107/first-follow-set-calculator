#ifndef __FFL__H__
#define __FFL__H__
#include <map>
#include <vector>
#include <set>
#include <string>

typedef std::map<std::string, std::vector<std::vector<std::string>>> Rules;
typedef std::map<std::string, std::set<std::string>> Set;

int first_set(const std::string& grammar, Set& set);
int follow_set(const std::string& grammar, const Set& fstset, Set& flwset, const std::string& start_nonterm);
void print_set(const Set& set);

#endif