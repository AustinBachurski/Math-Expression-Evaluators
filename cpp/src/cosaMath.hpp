#ifndef COSA_MATH_HPP
#define COSA_MATH_HPP

#include "operatorToken.hpp"

#include <array>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

class CosaMath
{
public:
    long double evaluate();
    void parseExpressionComponent(char c);

private:
    char m_inputChar{};
    long double m_outputValue;
    std::string m_operandString;
    bool m_generatingOperand;
    long double m_leftOperand;
    long double m_rightOperand;
    OperatorToken m_activeOperator;
    OperatorToken m_comparisonOperator;
    std::vector<OperatorToken> m_operatorStack;
    std::vector<long double> m_operandStack;
    static constexpr std::array<char, 2> m_unaryOperators { '-', '(' };
    static constexpr std::array<char, 5> m_binaryOperators { 
        ')', '*', '/', '+', '-' };


// COSA Logic
private:
    static constexpr unsigned int m_matrixRows { 8 };
    std::ofstream m_traceFile;
    bool m_engineRunning { true };

    using function = bool (CosaMath::*)(void) const;
    using procedure = void (CosaMath::*)(void);
    
    struct matrixRow
    {
        function state;
        procedure trueProcedure;
        size_t trueNext;
        procedure falseProcedure;
        size_t falseNext;
        std::string_view trace;
    };
    
    enum class Time
    {
    };

    std::array<matrixRow, m_matrixRows> m_logic;
    std::array<std::string_view, m_matrixRows> m_trace;

    constexpr void insertRule(
        Time index,
        function state,
        procedure trueProcedure,
        Time trueNext,
        procedure falseProcedure,
        Time falseNext,
        std::string_view trace
    );

    constexpr void populateRulesTables();
};

#endif
