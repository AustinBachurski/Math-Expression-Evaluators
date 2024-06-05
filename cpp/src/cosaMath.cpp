#include "cosaMath.hpp"

#include <algorithm>
#include <string_view>


CosaMath::CosaMath(const std::filesystem::path& pathToTraceFile) noexcept
  : m_traceFile{ pathToTraceFile, std::ios::out | std::ios::trunc }
{
    initializeTraceFile();
    populateLogicTable();
    populateTraceTable();
}

// Methods
long double CosaMath::evaluate()
{
    m_timeIndex = toSize_t(Time::finalEvaluation);
    m_engineRunning = true;
    while (m_engineRunning && m_programRunning)
    {
        if ((this->*(m_logic[m_timeIndex].state))())
        {
            (this->*(m_traceTrue))();
            (this->*(m_logic[m_timeIndex].trueProcedure))();
            m_timeIndex = m_logic[m_timeIndex].trueNext;
        }
        else
        {
            (this->*(m_traceFalse))();
            (this->*(m_logic[m_timeIndex].falseProcedure))();
            m_timeIndex = m_logic[m_timeIndex].falseNext;
        }
    }
    return m_operandStack.back();
}

void CosaMath::parseExpressionComponent(const char c)
{
    m_inputChar = c;
    m_engineRunning = true;
    while (m_engineRunning && m_programRunning)
    {
        if ((this->*(m_logic[m_timeIndex].state))())
        {
            (this->*(m_traceTrue))();
            (this->*(m_logic[m_timeIndex].trueProcedure))();
            m_timeIndex = m_logic[m_timeIndex].trueNext;
        }
        else
        {
            (this->*(m_traceFalse))();
            (this->*(m_logic[m_timeIndex].falseProcedure))();
            m_timeIndex = m_logic[m_timeIndex].falseNext;
        }
    }
}

bool CosaMath::running()
{
    return m_programRunning;
}

void CosaMath::quit()
{
    m_programRunning = false;
}

// Functions
bool CosaMath::isWhitespace() const
{
    return m_inputChar == ' ';
}

bool CosaMath::isUnaryOperator() const
{
    return std::ranges::contains(m_unaryOperators, m_inputChar);
}

bool CosaMath::isBinaryOperator() const
{
    return std::ranges::contains(m_binaryOperators, m_inputChar);
}

bool CosaMath::isNumberComponent() const
{
    return std::isdigit(m_inputChar) || m_inputChar == '.';
}

bool CosaMath::parsingOperand() const
{
    return m_generatingOperand;
}

bool CosaMath::checkPrecedence() const
{
    return m_operatorStack.back() >= m_newOperator;
}

bool CosaMath::sufficientOperands() const
{
    return  m_operandStack.size() >= m_existingOperator.m_operands;
}

bool CosaMath::unaryOperator() const
{
    return m_existingOperator.m_operands == 1;
}

bool CosaMath::multiplication() const
{
    return m_existingOperator.m_operator == Operator::multiplication;
}

bool CosaMath::division() const
{
    return m_existingOperator.m_operator == Operator::division;
}

bool CosaMath::addition() const
{
    return m_existingOperator.m_operator == Operator::addition;
}

bool CosaMath::subtraction() const
{
    return m_existingOperator.m_operator == Operator::subtraction;
}

bool CosaMath::negation() const
{
    return m_existingOperator.m_operator == Operator::negation;
}

bool CosaMath::isParenthesis() const
{
    return m_existingOperator.m_operator == Operator::parenthesis;
}

bool CosaMath::operatorsRemain() const
{
    return m_operandStack.size();
}

bool CosaMath::singleOperandRemains() const
{
    return m_operandStack.size() == 1;
}

// Procedures
void CosaMath::ignore()
{
    return; // Intentional nop
}

void CosaMath::skip()
{
    stopEngine();
    return;
}

OperatorToken CosaMath::makeUnaryOperator() const
{
    if (m_inputChar == '(')
    {
        return { Operator::parenthesis, Prescedence::parenthesis, 0 };
    }
    else if (m_inputChar == '-')
    {
        return { Operator::negation, Prescedence::negation, 0 };
    }
    else
    {
        std::unreachable();
    }
}

OperatorToken CosaMath::makeBinaryOperator() const
{
    switch (m_inputChar)
    {
        case ')':
            return { 
                Operator::parenthesis,
                Prescedence::parenthesis,
                0 };

        case '*':
            return { 
                Operator::multiplication,
                Prescedence::multiplicationDivision,
                0 };

        case '/':
            return { 
                Operator::division,
                Prescedence::multiplicationDivision,
                0 };

        case '+':
            return { 
                Operator::addition,
                Prescedence::additionSubtraction,
                0 };

        case '-':
            return { 
                Operator::subtraction,
                Prescedence::additionSubtraction,
                0 };
    }
    std::unreachable(); // If we hit this our parsing is incorrect.
}

void CosaMath::generateAndPushUnaryOperator()
{
    m_operatorStack.push_back(makeUnaryOperator());
    stopEngine();
}

void CosaMath::parseNumberComponent()
{
    m_generatingOperand = true;
    m_operandString += m_inputChar;
    stopEngine();
}

void CosaMath::resetState()
{
    m_operandString.clear();
    m_generatingOperand = false;
    m_timeIndex = 0;
    m_existingOperator = {};
    m_newOperator = {};
    m_operatorStack.clear();
    m_operandStack.clear();
}

void CosaMath::invalidExpression()
{
    // TODO - From an error reporting perspective, should have multiple
    // ways to throw, did we actually get an invalid character, or did
    // we encounter a number followed by a '('?  That sort of thing...
    resetState();
    throw std::invalid_argument(
            std::format("Character {} is not valid.", m_inputChar));
}

void CosaMath::pushOperand()
{
    // m_operandString is digits or '.' so parsing should never fail.
    m_operandStack.push_back(std::stold(m_operandString));
    m_generatingOperand = false;
    // Engine remains ON.
}

void CosaMath::generateBinaryOperator()
{
    m_newOperator = makeBinaryOperator();
    // Engine remains ON.
}

void CosaMath::popAndSetOperator()
{
    m_existingOperator = m_operatorStack.back();
    m_operatorStack.pop_back();
    // Engine remains ON.
}

void CosaMath::popAndSetSingleOperand()
{
    m_rightOperand = m_operandStack.back();
    m_operandStack.pop_back();
    // Engine remains ON.
}

void CosaMath::popAndSetTwoOperands()
{
    m_rightOperand = m_operandStack.back();
    m_operandStack.pop_back();
    m_leftOperand = m_operandStack.back();
    m_operandStack.pop_back();
    // Engine remains ON.
}

void CosaMath::pushOperator()
{
    m_operatorStack.push_back(m_newOperator);
    stopEngine();
}

void CosaMath::performMultiplication()
{
    m_operandStack.emplace_back(m_leftOperand * m_rightOperand);
    // Engine remains ON.
}

void CosaMath::performDivision()
{
    m_operandStack.emplace_back(m_leftOperand / m_rightOperand);
    // Engine remains ON.
}

void CosaMath::performAddition()
{
    m_operandStack.emplace_back(m_leftOperand + m_rightOperand);
    // Engine remains ON.
}

void CosaMath::performSubtraction()
{
    m_operandStack.emplace_back(m_leftOperand - m_rightOperand);
    // Engine remains ON.
}

void CosaMath::performNegation()
{
    m_operandStack.emplace_back(-m_leftOperand);
    // Engine remains ON.
}

// Trace
void CosaMath::traceFalse()
{
    m_traceFile << "False     " << m_trace[m_timeIndex] << '\n';
    m_traceFile.flush();
}

void CosaMath::traceTrue()
{
    m_traceFile << "True      " << m_trace[m_timeIndex] << '\n';
    m_traceFile.flush();
}

void CosaMath::initializeTraceFile()
{
    if (!m_traceFile.is_open())
    {
        m_traceTrue = &CosaMath::ignore;
        m_traceFalse = &CosaMath::ignore;
        return;
    }

    m_traceFile <<
        "Introspective Trace for Cosa Math Expression Evaluator\n"
        "                                                                                  True                            True                                False                               False\n"
        "Evaluation       Rule                             State                           Behavior                        Next                                Behavior                            Next                                Trace\n"
        "-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n";
    m_traceFile.flush();
}

// Logic Table
void CosaMath::stopEngine()
{
    m_engineRunning = false;
}

constexpr size_t CosaMath::toSize_t(const Time value) const
{
    return toSize_t(value);
}

void CosaMath::insertRule(
        Time index,
        function state,
        procedure trueProcedure,
        Time trueNext,
        procedure falseProcedure,
        Time falseNext,
        unsigned int trace
        )
{
    m_logic.at(toSize_t(index)).state = state;
    m_logic.at(toSize_t(index)).trueProcedure = trueProcedure;
    m_logic.at(toSize_t(index)).trueNext = toSize_t(trueNext);
    m_logic.at(toSize_t(index)).falseProcedure = falseProcedure;
    m_logic.at(toSize_t(index)).falseNext = toSize_t(falseNext);
    m_logic.at(toSize_t(index)).trace = trace;
}

void CosaMath::populateLogicTable()
{/*
    Cosa Matrix
                                                                                            True                                        True                                    False                               False
                Rule                                    State                               Behavior                                    Next                                    Behavior                            Next                                   Trace  */

    insertRule( Time::checkForUnaryWhitespace,          &CosaMath::isWhitespace,            &CosaMath::skip,                            Time::checkForUnaryWhitespace,          &CosaMath::ignore,                  Time::checkForUnaryOperator,           101 );
    insertRule( Time::checkForUnaryOperator,            &CosaMath::isUnaryOperator,         &CosaMath::generateAndPushUnaryOperator,    Time::checkForUnaryWhitespace,          &CosaMath::ignore,                  Time::checkForUnaryNumberComponent,    102 );
    insertRule( Time::checkForUnaryNumberComponent,     &CosaMath::isNumberComponent,       &CosaMath::parseNumberComponent,            Time::checkForBinaryWhitespace,         &CosaMath::invalidExpression,       Time::checkForUnaryWhitespace,         103 );
    insertRule( Time::checkForBinaryWhitespace,         &CosaMath::isWhitespace,            &CosaMath::skip,                            Time::checkForBinaryWhitespace,         &CosaMath::ignore,                  Time::checkForBinaryOperator,          104 );
    insertRule( Time::checkForBinaryNumberComponent,    &CosaMath::isNumberComponent,       &CosaMath::parseNumberComponent,            Time::checkForBinaryNumberComponent,    &CosaMath::ignore,                  Time::checkIfParsingNumber,            105 );
    insertRule( Time::checkIfParsingNumber,             &CosaMath::parsingOperand,          &CosaMath::pushOperand,                     Time::checkForBinaryOperator,           &CosaMath::ignore,                  Time::checkForBinaryOperator,          106 );
    insertRule( Time::checkForBinaryOperator,           &CosaMath::isBinaryOperator,        &CosaMath::generateBinaryOperator,          Time::checkOperatorPrecedence,          &CosaMath::invalidExpression,       Time::checkForUnaryWhitespace,         107 );            
    insertRule( Time::checkIsParenthesis,               &CosaMath::isParenthesis,           &CosaMath::ignore,                          Time::evaluateSubexpression,            &CosaMath::ignore,                  Time::checkOperatorPrecedence,         108 );
    insertRule( Time::checkOperatorPrecedence,          &CosaMath::checkPrecedence,         &CosaMath::popAndSetOperator,               Time::ensureSufficientOperands,         &CosaMath::pushOperator,            Time::checkForUnaryWhitespace,         109 );
    insertRule( Time::ensureSufficientOperands,         &CosaMath::sufficientOperands,      &CosaMath::ignore,                          Time::checkForUnaryOperator,            &CosaMath::invalidExpression,       Time::checkForUnaryWhitespace,         110 );
    insertRule( Time::checkForUnaryOperator,            &CosaMath::isUnaryOperator,         &CosaMath::popAndSetSingleOperand,          Time::operatorIsNegate,                 &CosaMath::popAndSetTwoOperands,    Time::operatorIsMultiply,              111 );
    insertRule( Time::operatorIsNegate,                 &CosaMath::negation,                &CosaMath::performNegation,                 Time::checkOperatorPrecedence,          &CosaMath::invalidExpression,       Time::checkForUnaryWhitespace,         112 );
    insertRule( Time::operatorIsMultiply,               &CosaMath::multiplication,          &CosaMath::performMultiplication,           Time::checkOperatorPrecedence,          &CosaMath::ignore,                  Time::operatorIsDivide,                113 );
    insertRule( Time::operatorIsDivide,                 &CosaMath::division,                &CosaMath::performDivision,                 Time::checkOperatorPrecedence,          &CosaMath::ignore,                  Time::operatorIsAdd,                   114 );
    insertRule( Time::operatorIsAdd,                    &CosaMath::addition,                &CosaMath::performAddition,                 Time::checkOperatorPrecedence,          &CosaMath::ignore,                  Time::operatorIsSubtract,              115 );
    insertRule( Time::operatorIsSubtract,               &CosaMath::subtraction,             &CosaMath::performSubtraction,              Time::checkOperatorPrecedence,          &CosaMath::invalidExpression,       Time::checkForUnaryWhitespace,         116 );

    insertRule( Time::evaluateSubexpression,            &CosaMath::operatorsRemain,         &CosaMath::popAndSetOperator,               Time::subEnsureSufficientOperands,      &CosaMath::invalidExpression,       Time::checkForUnaryWhitespace,         201 );
    insertRule( Time::subCheckIsParenthesis,            &CosaMath::isParenthesis,           &CosaMath::ignore,                          Time::checkForUnaryWhitespace,          &CosaMath::ignore,                  Time::subEnsureSufficientOperands,     202 );
    insertRule( Time::subEnsureSufficientOperands,      &CosaMath::sufficientOperands,      &CosaMath::ignore,                          Time::subCheckForUnaryOperator,         &CosaMath::invalidExpression,       Time::checkForUnaryWhitespace,         203 );
    insertRule( Time::subCheckForUnaryOperator,         &CosaMath::isUnaryOperator,         &CosaMath::popAndSetSingleOperand,          Time::subOperatorIsNegate,              &CosaMath::popAndSetTwoOperands,    Time::subOperatorIsMultiply,           204 );
    insertRule( Time::subOperatorIsNegate,              &CosaMath::negation,                &CosaMath::performNegation,                 Time::evaluateSubexpression,            &CosaMath::invalidExpression,       Time::evaluateSubexpression,           205 );
    insertRule( Time::subOperatorIsMultiply,            &CosaMath::multiplication,          &CosaMath::performMultiplication,           Time::evaluateSubexpression,            &CosaMath::ignore,                  Time::subOperatorIsDivide,             206 );
    insertRule( Time::subOperatorIsDivide,              &CosaMath::division,                &CosaMath::performDivision,                 Time::evaluateSubexpression,            &CosaMath::ignore,                  Time::subOperatorIsAdd,                207 );
    insertRule( Time::subOperatorIsAdd,                 &CosaMath::addition,                &CosaMath::performAddition,                 Time::evaluateSubexpression,            &CosaMath::ignore,                  Time::subOperatorIsSubtract,           208 );
    insertRule( Time::subOperatorIsSubtract,            &CosaMath::subtraction,             &CosaMath::performSubtraction,              Time::evaluateSubexpression,            &CosaMath::ignore,                  Time::subOperatorIsNegate,             209 );

    insertRule( Time::finalEvaluation,                  &CosaMath::operatorsRemain,         &CosaMath::popAndSetOperator,               Time::evalEnsureSufficientOperands,     &CosaMath::ignore,                  Time::evalSingleOperandRemains,        301 );
    insertRule( Time::evalEnsureSufficientOperands,     &CosaMath::sufficientOperands,      &CosaMath::ignore,                          Time::evalCheckForUnaryOperator,        &CosaMath::invalidExpression,       Time::checkForUnaryWhitespace,         302 );
    insertRule( Time::evalCheckForUnaryOperator,        &CosaMath::isUnaryOperator,         &CosaMath::popAndSetSingleOperand,          Time::evalOperatorIsNegate,             &CosaMath::popAndSetTwoOperands,    Time::evalOperatorIsMultiply,          303 );
    insertRule( Time::evalOperatorIsNegate,             &CosaMath::negation,                &CosaMath::performNegation,                 Time::finalEvaluation,                  &CosaMath::invalidExpression,       Time::checkForUnaryWhitespace,         304 );
    insertRule( Time::evalOperatorIsMultiply,           &CosaMath::multiplication,          &CosaMath::performMultiplication,           Time::finalEvaluation,                  &CosaMath::ignore,                  Time::evalOperatorIsDivide,            305 );
    insertRule( Time::evalOperatorIsDivide,             &CosaMath::division,                &CosaMath::performDivision,                 Time::finalEvaluation,                  &CosaMath::ignore,                  Time::evalOperatorIsAdd,               306 );
    insertRule( Time::evalOperatorIsAdd,                &CosaMath::addition,                &CosaMath::performAddition,                 Time::finalEvaluation,                  &CosaMath::ignore,                  Time::evalOperatorIsSubtract,          307 );
    insertRule( Time::evalOperatorIsSubtract,           &CosaMath::subtraction,             &CosaMath::performSubtraction,              Time::finalEvaluation,                  &CosaMath::invalidExpression,       Time::checkForUnaryWhitespace,         308 );
    insertRule( Time::evalSingleOperandRemains,         &CosaMath::singleOperandRemains,    &CosaMath::stopEngine,                      Time::checkForUnaryWhitespace,          &CosaMath::invalidExpression,       Time::checkForUnaryWhitespace,         309 );
}

void CosaMath::populateTraceTable()
{/*
                                                                                            True                                        True                                    False                               False
                Rule                                    State                               Behavior                                    Next                                    Behavior                            Next                                   Trace  */

    m_trace.at(toSize_t(Time::checkForUnaryWhitespace)) =
        "       Time::checkForUnaryWhitespace,          &CosaMath::isWhitespace,            &CosaMath::skip,                            Time::checkForUnaryWhitespace,          &CosaMath::ignore,                  Time::checkForUnaryOperator,           101";
    m_trace.at(toSize_t(Time::checkForUnaryOperator)) =
        "       Time::checkForUnaryOperator,            &CosaMath::isUnaryOperator,         &CosaMath::generateAndPushUnaryOperator,    Time::checkForUnaryWhitespace,          &CosaMath::ignore,                  Time::checkForUnaryNumberComponent,    102";
    m_trace.at(toSize_t(Time::checkForUnaryNumberComponent)) =
        "       Time::checkForUnaryNumberComponent,     &CosaMath::isNumberComponent,       &CosaMath::parseNumberComponent,            Time::checkForBinaryWhitespace,         &CosaMath::invalidExpression,       Time::checkForUnaryWhitespace,         103";
    m_trace.at(toSize_t(Time::checkForBinaryWhitespace)) =
        "       Time::checkForBinaryWhitespace,         &CosaMath::isWhitespace,            &CosaMath::skip,                            Time::checkForBinaryWhitespace,         &CosaMath::ignore,                  Time::checkForBinaryOperator,          104";
    m_trace.at(toSize_t(Time::checkForBinaryNumberComponent)) =
        "       Time::checkForBinaryNumberComponent,    &CosaMath::isNumberComponent,       &CosaMath::parseNumberComponent,            Time::checkForBinaryNumberComponent,    &CosaMath::ignore,                  Time::checkIfParsingNumber,            105";
    m_trace.at(toSize_t(Time::checkIfParsingNumber)) =
        "       Time::checkIfParsingNumber,             &CosaMath::parsingOperand,          &CosaMath::pushOperand,                     Time::checkForBinaryOperator,           &CosaMath::ignore,                  Time::checkForBinaryOperator,          106";
    m_trace.at(toSize_t(Time::checkForBinaryOperator)) =
        "       Time::checkForBinaryOperator,           &CosaMath::isBinaryOperator,        &CosaMath::generateBinaryOperator,          Time::checkOperatorPrecedence,          &CosaMath::invalidExpression,       Time::checkForUnaryWhitespace,         107";            
    m_trace.at(toSize_t(Time::checkIsParenthesis)) =
        "       Time::checkIsParenthesis,               &CosaMath::isParenthesis,           &CosaMath::ignore,                          Time::evaluateSubexpression,            &CosaMath::ignore,                  Time::checkOperatorPrecedence,         108";
    m_trace.at(toSize_t(Time::checkOperatorPrecedence)) =
        "       Time::checkOperatorPrecedence,          &CosaMath::checkPrecedence,         &CosaMath::popAndSetOperator,               Time::ensureSufficientOperands,         &CosaMath::pushOperator,            Time::checkForUnaryWhitespace,         109";
    m_trace.at(toSize_t(Time::ensureSufficientOperands)) =
        "       Time::ensureSufficientOperands,         &CosaMath::sufficientOperands,      &CosaMath::ignore,                          Time::checkForUnaryOperator,            &CosaMath::invalidExpression,       Time::checkForUnaryWhitespace,         110";
    m_trace.at(toSize_t(Time::checkForUnaryOperator)) =
        "       Time::checkForUnaryOperator,            &CosaMath::isUnaryOperator,         &CosaMath::popAndSetSingleOperand,          Time::operatorIsNegate,                 &CosaMath::popAndSetTwoOperands,    Time::operatorIsMultiply,              111";
    m_trace.at(toSize_t(Time::operatorIsNegate)) =
        "       Time::operatorIsNegate,                 &CosaMath::negation,                &CosaMath::performNegation,                 Time::checkOperatorPrecedence,          &CosaMath::invalidExpression,       Time::checkForUnaryWhitespace,         112";
    m_trace.at(toSize_t(Time::operatorIsMultiply)) =
        "       Time::operatorIsMultiply,               &CosaMath::multiplication,          &CosaMath::performMultiplication,           Time::checkOperatorPrecedence,          &CosaMath::ignore,                  Time::operatorIsDivide,                113";
    m_trace.at(toSize_t(Time::operatorIsDivide)) =
        "       Time::operatorIsDivide,                 &CosaMath::division,                &CosaMath::performDivision,                 Time::checkOperatorPrecedence,          &CosaMath::ignore,                  Time::operatorIsAdd,                   114";
    m_trace.at(toSize_t(Time::operatorIsAdd)) =
        "       Time::operatorIsAdd,                    &CosaMath::addition,                &CosaMath::performAddition,                 Time::checkOperatorPrecedence,          &CosaMath::ignore,                  Time::operatorIsSubtract,              115";
    m_trace.at(toSize_t(Time::operatorIsSubtract)) =
        "       Time::operatorIsSubtract,               &CosaMath::subtraction,             &CosaMath::performSubtraction,              Time::checkOperatorPrecedence,          &CosaMath::invalidExpression,       Time::checkForUnaryWhitespace,         116";

    m_trace.at(toSize_t(Time::evaluateSubexpression)) =
        "       Time::evaluateSubexpression,            &CosaMath::operatorsRemain,         &CosaMath::popAndSetOperator,               Time::subEnsureSufficientOperands,      &CosaMath::invalidExpression,       Time::checkForUnaryWhitespace,         201";
    m_trace.at(toSize_t(Time::subCheckIsParenthesis)) =
        "       Time::subCheckIsParenthesis,            &CosaMath::isParenthesis,           &CosaMath::ignore,                          Time::checkForUnaryWhitespace,          &CosaMath::ignore,                  Time::subEnsureSufficientOperands,     202";
    m_trace.at(toSize_t(Time::subEnsureSufficientOperands)) =
        "       Time::subEnsureSufficientOperands,      &CosaMath::sufficientOperands,      &CosaMath::ignore,                          Time::subCheckForUnaryOperator,         &CosaMath::invalidExpression,       Time::checkForUnaryWhitespace,         203";
    m_trace.at(toSize_t(Time::subCheckForUnaryOperator)) =
        "       Time::subCheckForUnaryOperator,         &CosaMath::isUnaryOperator,         &CosaMath::popAndSetSingleOperand,          Time::subOperatorIsNegate,              &CosaMath::popAndSetTwoOperands,    Time::subOperatorIsMultiply,           204";
    m_trace.at(toSize_t(Time::subOperatorIsNegate)) =
        "       Time::subOperatorIsNegate,              &CosaMath::negation,                &CosaMath::performNegation,                 Time::evaluateSubexpression,            &CosaMath::invalidExpression,       Time::evaluateSubexpression,           205";
    m_trace.at(toSize_t(Time::subOperatorIsMultiply)) =
        "       Time::subOperatorIsMultiply,            &CosaMath::multiplication,          &CosaMath::performMultiplication,           Time::evaluateSubexpression,            &CosaMath::ignore,                  Time::subOperatorIsDivide,             206";
    m_trace.at(toSize_t(Time::subOperatorIsDivide)) =
        "       Time::subOperatorIsDivide,              &CosaMath::division,                &CosaMath::performDivision,                 Time::evaluateSubexpression,            &CosaMath::ignore,                  Time::subOperatorIsAdd,                207";
    m_trace.at(toSize_t(Time::subOperatorIsAdd)) =
        "       Time::subOperatorIsAdd,                 &CosaMath::addition,                &CosaMath::performAddition,                 Time::evaluateSubexpression,            &CosaMath::ignore,                  Time::subOperatorIsSubtract,           208";
    m_trace.at(toSize_t(Time::subOperatorIsSubtract)) =
        "       Time::subOperatorIsSubtract,            &CosaMath::subtraction,             &CosaMath::performSubtraction,              Time::evaluateSubexpression,            &CosaMath::ignore,                  Time::subOperatorIsNegate,             209";

    m_trace.at(toSize_t(Time::finalEvaluation)) =
        "       Time::finalEvaluation,                  &CosaMath::operatorsRemain,         &CosaMath::popAndSetOperator,               Time::evalEnsureSufficientOperands,     &CosaMath::ignore,                  Time::evalSingleOperandRemains,        301";
    m_trace.at(toSize_t(Time::evalEnsureSufficientOperands)) =
        "       Time::evalEnsureSufficientOperands,     &CosaMath::sufficientOperands,      &CosaMath::ignore,                          Time::evalCheckForUnaryOperator,        &CosaMath::invalidExpression,       Time::checkForUnaryWhitespace,         302";
    m_trace.at(toSize_t(Time::evalCheckForUnaryOperator)) =
        "       Time::evalCheckForUnaryOperator,        &CosaMath::isUnaryOperator,         &CosaMath::popAndSetSingleOperand,          Time::evalOperatorIsNegate,             &CosaMath::popAndSetTwoOperands,    Time::evalOperatorIsMultiply,          303";
    m_trace.at(toSize_t(Time::evalOperatorIsNegate)) =
        "       Time::evalOperatorIsNegate,             &CosaMath::negation,                &CosaMath::performNegation,                 Time::finalEvaluation,                  &CosaMath::invalidExpression,       Time::checkForUnaryWhitespace,         304";
    m_trace.at(toSize_t(Time::evalOperatorIsMultiply)) =
        "       Time::evalOperatorIsMultiply,           &CosaMath::multiplication,          &CosaMath::performMultiplication,           Time::finalEvaluation,                  &CosaMath::ignore,                  Time::evalOperatorIsDivide,            305";
    m_trace.at(toSize_t(Time::evalOperatorIsDivide)) =
        "       Time::evalOperatorIsDivide,             &CosaMath::division,                &CosaMath::performDivision,                 Time::finalEvaluation,                  &CosaMath::ignore,                  Time::evalOperatorIsAdd,               306";
    m_trace.at(toSize_t(Time::evalOperatorIsAdd)) =
        "       Time::evalOperatorIsAdd,                &CosaMath::addition,                &CosaMath::performAddition,                 Time::finalEvaluation,                  &CosaMath::ignore,                  Time::evalOperatorIsSubtract,          307";
    m_trace.at(toSize_t(Time::evalOperatorIsSubtract)) =
        "       Time::evalOperatorIsSubtract,           &CosaMath::subtraction,             &CosaMath::performSubtraction,              Time::finalEvaluation,                  &CosaMath::invalidExpression,       Time::checkForUnaryWhitespace,         308";
    m_trace.at(toSize_t(Time::evalSingleOperandRemains)) =
        "       Time::evalSingleOperandRemains,         &CosaMath::singleOperandRemains,    &CosaMath::stopEngine,                      Time::checkForUnaryWhitespace,          &CosaMath::invalidExpression,       Time::checkForUnaryWhitespace,         309";
}
