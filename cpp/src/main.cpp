#include "cosaMath.hpp"

#include <iostream>
#include <print>
#include <string_view>

void displayWelcome()
{
    std::system("clear");
    const auto welcomeMessage = 
        "*********************************************\n"
        "*  COSA Powered Math Experession Evaluator  *\n"
        "*********************************************\n\n"
        "This evaluator supports the following operators:\n\n"
        "\t  ****************************************\n"
        "\t  *                                      *\n"
        "\t  *  ( ) Parenthesis                     *\n"
        "\t  *   -  Negation                        *\n"
        "\t  *   *  Multiplication                  *\n"
        "\t  *   /  Division                        *\n"
        "\t  *   +  Addition                        *\n"
        "\t  *   -  Subtraction                     *\n"
        "\t  *  Whitespace Insensive                *\n"
        "\t  *  Type <exit> to quit.                *\n"
        "\t  *  Type <help> to display this again.  *\n"
        "\t  *                                      *\n"
        "\t  ****************************************\n\n"
        "Please enter an expression:> ";

    std::print("{}", welcomeMessage);
}

void checkForHelpOrQuit(const std::string_view input, CosaMath& program)
{
    if (input.contains("help"))
    {
        displayWelcome();
    }
            
    if (input.contains("exit"))
    {
        program.quit();
    }
}


int main()
{
    CosaMath math("./trace.txt");

    displayWelcome();

    std::string inputString;

    while (math.running())
    {
        std::cin >> inputString;
        checkForHelpOrQuit(inputString, math);

        try
        {
            for (const char c : inputString)
            {
                math.parseExpressionComponent(c);
            }
            std::print("Result: {}\n\nPlease enter an expression:> ", 
                    math.evaluate());
        }
        catch (const std::invalid_argument& error)
        {
            std::println("{}", error.what());
        }

    }
    return 0;
}
