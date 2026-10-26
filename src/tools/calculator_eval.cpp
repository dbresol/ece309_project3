// src/tools/calculator_eval.cpp — YOU IMPLEMENT (spec §4.6).
//
// calc_evaluate: evaluate a well-formed token list with two stacks
// (one for numbers, one for operators and '('). See spec §4.6 and the
// worked trace in Appendix C. std::stack or std::vector are both fine.

#include "tools/calculator.h"
#include <stdexcept>
#include <cstdint>
#include <cstddef>
#include <utility>
#include <stack>

bool operate(std::stack<double>& values, std::stack<CalcToken>& ops, 
    std::string* error)
{
    //if there are two operands in a row, there are not enough values
    if (values.size() < 2)
    {
        *error = "Not enough values";
        return false;
    }

    //if there is no left parentheses, there are not enough operands
    if (ops.size() < 1)
    {
        *error = "Not enough operands";
        return false;
    }

    //pop operator and the two values needed
    CalcToken::Kind op = ops.top().kind;
    ops.pop();
    double rVal = values.top();
    values.pop();
    double lVal = values.top();
    values.pop();

    //if it's going to divide by zero, return error
    if ((rVal == 0) && (op == CalcToken::Kind::Divide))
    {
        *error = "division by zero";
        return false;
    }

    //divide by zero already considered, so no special behavior there
    auto res = (op == CalcToken::Kind::Plus) ? (lVal + rVal) 
    : (op == CalcToken::Kind::Minus) ? (lVal - rVal) 
    : (op == CalcToken::Kind::Times) ? (lVal * rVal) 
    : (lVal / rVal);

    values.push(res);
    return true;
}

bool calc_evaluate(const std::vector<CalcToken>& tokens, double* out,
                   std::string* error) {

    //if there is already a value in error for whatever reason
    if (!(*error).empty())
    {
        return false;
    }

    std::stack<double> values;
    std::stack<CalcToken> ops;

    //through each token
    for (size_t i = 0; i < tokens.size(); i++)
    {
        CalcToken token = tokens[i];
        CalcToken::Kind tKind = token.kind;
        double tVal = token.value;
        /*
        CalcToken::Kind topKind;
        if(!ops.empty())
        {
            //top = ops.top();
            topKind = ops.top().kind;
        }
        */

        //if it's a number, just push
        if (tKind == CalcToken::Kind::Number) 
        {
            values.push(tVal);
            continue;
        }

        //if it's a (, just push
        if (tKind == CalcToken::Kind::LParen) 
        {
            ops.push(token);
            continue;
        }
        
        //if it's a ), operate until left parentheses is reached
        if (tKind == CalcToken::Kind::RParen)
        {
            //return error if left parentheses don't exist
            if (ops.empty()) 
            {
                *error = "Parentheses are mismatched";
                return false;
            }

            //operate until it finds a left parentheses. if there is none, it will return an error in operate
            while (ops.top().kind != CalcToken::Kind::LParen)
            {
                if (!operate(values, ops, error)) 
                {
                    return false;
                }
            }
            
            //pop the (
            ops.pop(); 
            continue;
        }
        
        //if its a plus or a minus or a times or a divide
        if((tKind == CalcToken::Kind::Plus)
        || (tKind == CalcToken::Kind::Minus)
        || (tKind == CalcToken::Kind::Times)
        || (tKind == CalcToken::Kind::Divide))
        {
            //if it's the only operator, push it
            if (ops.empty())
            {
                ops.push(token);
                continue;
            }

            //get precedence of incoming token
            int prec = ((tKind == CalcToken::Kind::Plus) 
            || (tKind == CalcToken::Kind::Minus)) ? 1 : 2;

            //get precedence of token on top of stack
            int topPrec = ((ops.top().kind == CalcToken::Kind::Plus) 
            || (ops.top().kind == CalcToken::Kind::Minus)) ? 1 : 2;

            //while there is an operator on the stack 
            //AND the top operator is higher priority than the incoming one 
            //AND the top parameter is not a left paretheses
            while (!ops.empty() 
            && (topPrec >= prec) 
            && (ops.top().kind != CalcToken::Kind::LParen))
            {

                //get the precendence of the top operator
                topPrec = ((ops.top().kind == CalcToken::Kind::Plus) 
                    || (ops.top().kind == CalcToken::Kind::Minus)) ? 1 : 2;
                if(!operate(values, ops, error))
                {
                    return false;
                }
            }


            ops.push(token);
            continue;
        }
    }

    //do remaining operations now that there should be no parentheses
    while (!ops.empty()) 
    {
        //if there is still a left parentheses, error
        if (ops.top().kind == CalcToken::Kind::LParen) 
        {
            if (error) *error = "mismatched parentheses";
            return false;
        }
        if (!operate(values, ops, error))
        {
            return false;
        }
    }

    //if there is not exactly one output value for whatever reason
    if (values.size() != 1) 
    {
        *error = "invalid expression";
        return false;
    }
    
    *out = values.top();
    return true;
}
