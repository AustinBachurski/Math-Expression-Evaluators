#include "cosaMath.hpp"

#include <print>
#include <iostream>

int main()
{
    CosaMath math;

    std::system("clear");
    const auto welcomeMessage = 
        "*********************************************\n"
        "*  COSA Powered Math Experession Evaluator  *\n"
        "*********************************************\n\n"
        "This evaluator supports the following operators:\n\n"
        "\t  *************************\n"
        "\t  *                       *\n"
        "\t  *  ( ) Parenthesis      *\n"
        "\t  *   -  Negation         *\n"
        "\t  *   *  Multiplication   *\n"
        "\t  *   /  Division         *\n"
        "\t  *   +  Addition         *\n"
        "\t  *   -  Subtraction      *\n"
        "\t  *                       *\n"
        "\t  *************************\n\n"
        "Please enter an expression:> ";

    std::print("{}", welcomeMessage);

    std::string inputString;
    std::cin >> inputString;

    return 0;
}

