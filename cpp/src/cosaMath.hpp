#ifndef COSA_MATH_HPP
#define COSA_MATH_HPP

#include "operatorToken.hpp"

#include <array>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

// TODO - Doxygen Comments!
class CosaMath
{
    using function = bool (CosaMath::*)(void) const;
    using procedure = void (CosaMath::*)(void);

public:
    CosaMath(const std::filesystem::path& pathToTraceFile) noexcept; 

    // Both of these throw: std::invalid_argument or std::runtime_error if an
    // invalid character is encountered, or if a divide by zero error occurs.
    long double evaluate();
    void parseExpressionComponent(const char c);
    bool running();
    void quit();

private:
    char m_inputChar{ ' ' };
    std::string m_operandString;
    bool m_generatingOperand{ false };
    bool m_engineRunning{ true };
    bool m_programRunning{ true };
    size_t m_timeIndex{ 0 };
    std::ofstream m_traceFile;
    procedure m_traceFalse = nullptr;
    procedure m_traceTrue = nullptr;
    long double m_leftOperand{ 0.0 };
    long double m_rightOperand{ 0.0 };
    OperatorToken m_existingOperator;
    OperatorToken m_newOperator;
    std::vector<OperatorToken> m_operatorStack;
    std::vector<long double> m_operandStack;
    static constexpr std::array m_unaryOperators { '-', '(' };
    static constexpr std::array m_binaryOperators { ')', '*', '/', '+', '-' };


// COSA Functions
private:
    [[nodiscard]] bool isWhitespace() const;
    [[nodiscard]] bool isUnaryOperator() const;
    [[nodiscard]] bool isNumberComponent() const;
    [[nodiscard]] bool parsingOperand() const;
    [[nodiscard]] bool isBinaryOperator() const;
    [[nodiscard]] bool checkPrecedence() const;
    [[nodiscard]] bool sufficientOperands() const;
    [[nodiscard]] bool unaryOperator() const;
    [[nodiscard]] bool multiplication() const;
    [[nodiscard]] bool division() const;
    [[nodiscard]] bool addition() const;
    [[nodiscard]] bool subtraction() const;
    [[nodiscard]] bool negation() const;
    [[nodiscard]] bool isParenthesis() const;
    [[nodiscard]] bool finalizeEvaluation() const;
    [[nodiscard]] bool operatorsRemain() const;
    [[nodiscard]] bool singleOperandRemains() const;


// COSA Procedures
private:
    void ignore();
    void skip();
    void generateAndPushUnaryOperator();
    void parseNumberComponent();
    void invalidExpression();
    void pushOperand();
    void generateUnaryOperator();
    void generateBinaryOperator();
    void popAndSetOperator();
    void popAndSetSingleOperand();
    void popAndSetTwoOperands();
    void pushOperator();
    void performMultiplication();
    void performDivision();
    void performAddition();
    void performSubtraction();
    void performNegation();

// COSA Trace
private:
    void initializeTraceFile();
    void traceFalse();
    void traceTrue();

// For indexing into m_logic array.
private:
    enum class Time : size_t
    {
        // Evaluation during expression parsing.
        checkForUnaryWhitespace,
        checkForBinaryWhitespace,
        checkForUnaryOperator,
        checkForUnaryNumberComponent,
        checkForBinaryNumberComponent,
        checkIfParsingNumber,
        checkForBinaryOperator,
        checkOperatorPrecedence,
        checkIsParenthesis,
        ensureSufficientOperands,
        operatorIsMultiply,
        operatorIsDivide,
        operatorIsAdd,
        operatorIsSubtract,
        operatorIsNegate,
        operatorIsParenthesis,
        compareOperatorPrecedence,
        checkForMatchingParenthesis,
        checkForRemainingOperators,
        ensureSingleOperandRemains,

        // Subexpression - evaluation within matched parenthesis.
        evaluateSubexpression,
        subCheckIsParenthesis,
        subEnsureSufficientOperands,
        subCheckForUnaryOperator,
        subOperatorIsNegate,
        subOperatorIsMultiply,
        subOperatorIsDivide,
        subOperatorIsAdd,
        subOperatorIsSubtract,
        subOperatorIsParenthesis,

        // Expression parsing complete finish evaluation.
        // Parenthesis should not exist at this point.
        finalEvaluation,
        evalEnsureSufficientOperands,
        evalCheckForUnaryOperator,
        evalOperatorIsNegate,
        evalOperatorIsMultiply,
        evalOperatorIsDivide,
        evalOperatorIsAdd,
        evalOperatorIsSubtract,
        evalSingleOperandRemains,
    };

// Supporting Functions
private:
    void stopEngine();
    [[nodiscard]] OperatorToken makeUnaryOperator() const;
    [[nodiscard]] OperatorToken makeBinaryOperator() const;
    [[nodiscard]] constexpr size_t toSize_t(const Time value) const;
    void resetState();

// COSA Logic Table Setup
private:
    struct MatrixRow
    {
        function state;
        procedure trueProcedure;
        size_t trueNext;
        procedure falseProcedure;
        size_t falseNext;
        unsigned int trace;
    };
  
    void insertRule(
            Time index,
            function state,
            procedure trueProcedure,
            Time trueNext,
            procedure falseProcedure,
            Time falseNext,
            unsigned int trace);
    void populateLogicTable();
    void populateTraceTable();

    static constexpr unsigned int m_matrixRows { 8 };
    std::array<MatrixRow, m_matrixRows> m_logic;
    std::array<std::string_view, m_matrixRows> m_trace;
};

#endif
