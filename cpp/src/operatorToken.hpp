#ifndef OPERATOR_TOKEN_HPP
#define OPERATOR_TOKEN_HPP

#include <compare>

enum class Prescedence
{
    unassigned,
    parenthesis,
    additionSubtraction,
    multiplicationDivision,
    negation,
};

enum class Operator
{
    unassigned,
    parenthesis,
    negation,
    multiplication,
    division,
    addition,
    subtraction,
};

struct OperatorToken
{
    Operator m_operator;
    Prescedence m_prescedence;
    unsigned int m_operands;

    auto operator<=>(const OperatorToken& other) const
    {
        return m_prescedence <=> other.m_prescedence;
    }
};

#endif
