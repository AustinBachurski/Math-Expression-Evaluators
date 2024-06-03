#ifndef OPERATOR_TOKEN_HPP
#define OPERATOR_TOKEN_HPP

enum class Prescedence
{
    Addition_Subtraction,
    Multiplication_Division,
    Negation,
    Parenthesis,
};

enum class Operator
{
    Parenthesis,
    Negation,
    Multiplication,
    Division,
    Addition,
    Subtraction,
};

struct OperatorToken
{
    Operator m_operator;
    Prescedence m_prescedence;
};

#endif
