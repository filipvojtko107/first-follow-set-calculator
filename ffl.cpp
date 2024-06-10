#include "ffl.h"
#include <sstream>
#include <iostream>
#include <cctype>
#include <algorithm>
//#define DEBUG
static const std::string EPSILON = "epsilon";


bool is_term(const std::string& term, const std::set<std::string>& terms)
{
    return (term == EPSILON || std::find(terms.begin(), terms.end(), term) != terms.end());
}

bool is_nonterm(const std::string& nonterm, const std::set<std::string>& nonterms)
{
    return (std::find(nonterms.begin(), nonterms.end(), nonterm) != nonterms.end());
}


int parse_nonterms(const std::string& grammar, std::set<std::string>& nonterms)
{
    std::istringstream g(grammar);
    std::string rule;
    std::string nonterm;
    size_t pos;

    while (std::getline(g, rule))
    {
        pos = rule.find("->");
        if (pos == std::string::npos)
        {
            std::cerr << "Invalid grammar format!\n";
            return -1;
        }

        nonterm = rule.substr(0, pos - 1);
        nonterms.insert(std::move(nonterm));
    }

    return 0;
}


int parse_grammar(const std::string& grammar, Rules& rules, std::set<std::string>& terms, std::set<std::string>& nonterms)
{
    std::istringstream g(grammar);
    std::string rule;
    std::string str;
    std::string current_nonterm;
    std::string nonterm;
    size_t rules_index;

    if (parse_nonterms(grammar, nonterms) == -1) {
        return -1;
    }

    while (std::getline(g, rule))
    {
        // Pridam neterminal
        size_t pos = rule.find("->");
        nonterm = rule.substr(0, pos - 1);

        if (current_nonterm != nonterm) 
        {
            current_nonterm = nonterm;
            rules_index = 0;
        }
        else {
            rules_index++;
        }

        pos += 3;
        bool ok = false;

        while (true)
        {
            size_t temp_pos = rule.find(' ', pos);
            if (temp_pos == std::string::npos) {
                str = rule.substr(pos);
                ok = true;
            }
            else {
                str = rule.substr(pos, temp_pos - pos);
                pos = temp_pos + 1;
            }

            if (rules[current_nonterm].size() < (rules_index + 1)) {
                rules[current_nonterm].push_back(std::vector<std::string>());
            }

            rules[current_nonterm].at(rules_index).push_back(str);
            if (!is_nonterm(str, nonterms)) {
                terms.insert(std::move(str));
            }

            if (ok) { break; }
        }
    }

    return 0;
}


void fst(bool& is_epsilon, std::vector<std::string>& stack, const Rules& rules, std::set<std::string>& fset, const std::set<std::string>& terms, const std::set<std::string>& nonterms)
{
#ifdef DEBUG
    for (const std::string& s : stack) {
        std::cout << s << ' ';
    }
    std::cout << "\n\n";
#endif

    if (stack.empty()) 
    {
        is_epsilon = true;
        return;
    }

    if (is_term(*stack.begin(), terms))
    {
        if (*stack.begin() == EPSILON)
        {
            stack.erase(stack.begin(), stack.begin() + 1);
            return fst(is_epsilon, stack, rules, fset, terms, nonterms);
        }

        else
        {
            fset.insert(*stack.begin());
            return;
        }
    }

    else
    {
        const std::string& nonterm = *stack.begin();
        const std::vector<std::vector<std::string>>& rls = rules.at(nonterm);

        for (const std::vector<std::string>& rl : rls)
        {
            std::vector<std::string> stack_temp = stack;
            stack_temp.erase(stack_temp.begin(), stack_temp.begin() + 1);
            size_t i = 0;
            for (const std::string& r : rl) 
            {
                stack_temp.insert(stack_temp.begin() + i, r);
                i++;
            }

            fst(is_epsilon, stack_temp, rules, fset, terms, nonterms);
        }
    }
}

int first_set(const std::string& grammar, Set& set)
{
    Rules rules;
    std::set<std::string> terms;
    std::set<std::string> nonterms;

    if (parse_grammar(grammar, rules, terms, nonterms) == -1) {
        return -1;
    }

#ifdef DEBUG
    std::cout << "Terms:\n";
    for (const auto& t : terms) {
        std::cout << t << ' ';
    }
    std::cout << std::endl << std::endl;

    std::cout << "Non-terms:\n";
    for (const auto& t : nonterms) {
        std::cout << t << ' ';
    }
    std::cout << std::endl << std::endl;

    std::cout << "Rules:\n";
    for (const auto& r1 : rules)
    {
        std::cout << r1.first << " = {";
        for (const auto& r2 : r1.second)
        {
            std::cout << "{";
            for (const std::string& s : r2) {
                std::cout << s << ", ";
            }
            std::cout << "}, ";
        }
        std::cout << "}\n";
    }
    std::cout << std::endl;
#endif

    std::vector<std::string> stack;
    std::string nonterm;

    for (const auto& r1 : rules)
    {
        nonterm = r1.first;
        for (const std::vector<std::string>& rule : r1.second)
        {
            stack = rule;
            bool is_epsilon = false;
            fst(is_epsilon, stack, rules, set[nonterm], terms, nonterms);
            if (is_epsilon) {
                set[nonterm].insert(EPSILON);
            }
        }
    }

    return 0;
}


void add_to_flw_set(const std::set<std::string>& fset, std::set<std::string>& flwset)
{
    for (const std::string& fst : fset) {
        flwset.insert(fst);
    }
}

std::set<std::string> flw(const std::string& nonterm, const Rules& rules, const Set& fstset, const std::set<std::string>& terms, std::set<std::string>& flwset)
{
    std::set<std::string> most_righthand_side;

    // Najdu vsechna pravidla s vyskytem daneho "nonterm"
    for (const auto& r1 : rules)
    {
        for (const std::vector<std::string>& r2 : r1.second)
        {
            // Nalezeno pravidlo
            auto pos = r2.end();
            if ((pos = std::find(r2.begin(), r2.end(), nonterm)) != r2.end()) 
            {
                // Posledni prvek v pravidle
                if ((pos + 1) == r2.end())
                {
                    if (r1.first != nonterm) {
                        most_righthand_side.insert(r1.first);
                    }
                }
                else
                {
                    auto temp_pos = pos + 1;
                    if (is_term(*temp_pos, terms))
                    {
                        if (*temp_pos != EPSILON) { flwset.insert(*temp_pos); }
                        continue;
                    } 

                    const std::set<std::string>* fset = &fstset.at(*temp_pos);
                    // Projizdim dokud dostavam epsilon
                    while (std::find(fset->begin(), fset->end(), EPSILON) != fset->end())
                    {
                        add_to_flw_set(*fset, flwset);
                        temp_pos++;
                        if (temp_pos == r2.end())
                        {
                            most_righthand_side.insert(r1.first); 
                            break;
                        }
                        else 
                        {
                            if (is_term(*temp_pos, terms))
                            {
                                if (*temp_pos != EPSILON) { flwset.insert(*temp_pos); }
                                break;
                            } 
                            else { 
                                fset = &fstset.at(*temp_pos); 
                            }
                        }
                    }

                    if (temp_pos != r2.end()) {
                        add_to_flw_set(*fset, flwset);
                    }
                    flwset.erase(EPSILON);
                }
            }
        }
    }

    return most_righthand_side;
}

int follow_set(const std::string& grammar, const Set& fstset, Set& flwset, const std::string& start_nonterm)
{
    Rules rules;
    std::set<std::string> terms;
    std::set<std::string> nonterms;

    if (parse_grammar(grammar, rules, terms, nonterms) == -1) {
        return -1;
    }

    // Vypocet follow mnozin
    std::map<std::string, std::set<std::string>> most_righthand_sides;
    for (const auto& r1 : rules)
    {
        const std::string& nonterm = r1.first;
        most_righthand_sides[nonterm] = flw(nonterm, rules, fstset, terms, flwset[nonterm]);
    }

    // Doplneni startovaciho symbolu
    flwset[start_nonterm].insert(EPSILON);

    // Dopocitat the most righthand sides
    for (const auto& rh : most_righthand_sides)
    {
        const std::string& nonterm = rh.first;
        for (const std::string& rhnt : rh.second)
        {
            // Kontrola provazanosti
            const auto& mrs = most_righthand_sides[rhnt];
            if (std::find(mrs.begin(), mrs.end(), nonterm) == mrs.end())
            {
                add_to_flw_set(flwset[rhnt], flwset[nonterm]);
            }
        }
    }

    return 0;
}


void print_set(const Set& set)
{
    for (const auto& s : set)
    {
        std::cout << s.first << " = {";
        if (!s.second.empty())
        {
            auto ss = s.second.cbegin();
            auto it = ss;
            std::advance(it, s.second.size() - 1);
            for (; ss != it; ++ss) {
                std::cout << *ss << ", ";
            }
            std::cout << *ss;
        }
        std::cout << "}\n";
    }
}
