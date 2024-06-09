#include "ffl.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstring>


void help()
{
    std::cout << "USAGE:\n";
    std::cout << "./ffl_calculator file_with_grammar grammar_start_symbol\n";
}

int main(int argc, const char* argv[])
{
    if (argc == 1)
    {
        help();
        return 0;
    }

    else if (argc == 2 && strcmp(argv[1], "--help") == 0)
    {
        help();
        return 0;
    }

    else if (argc > 3)
    {
        std::cerr << "Invalid number of arguments!\n";
        return 1;
    }

    std::string grammar;
    std::stringstream in;
    std::ifstream file(argv[1]);
    if (!file.is_open())
    {
        std::cerr << "Can't open file: " << argv[1] << '\n';
        return 1;
    }
    
    in << file.rdbuf();
    file.close();
    grammar = in.str();

    Set fset;
    first_set(grammar, fset);
    std::cout << "First-set:\n";
    print_set(fset);
    std::cout << std::endl;

    Set flwset;
    follow_set(grammar, fset, flwset, argv[2]);
    std::cout << "Follow-set:\n";
    print_set(flwset);

    return 0;
}

