#pragma once

#include <sstream>
#include <string>
#include <vector>

class Lexer {

public:
    static std::vector<std::string> lexer(std::string& str){
        std::vector<std::string> token;
        std::istringstream strsteam(str);
        std::string tokens;
        while (strsteam>>tokens) {
            token.push_back(tokens);
        }
        return token;
    }
};