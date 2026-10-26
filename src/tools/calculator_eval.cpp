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
    /*new*/
    if (values.size() < 2)
    {
        *error = "Not enough operands";
        return false;
    }

    CalcToken::Kind op = ops.top().kind;
    ops.pop();
    double rVal = values.top();
    values.pop();
    double lVal = values.top();
    values.pop();

    if ((rVal == 0) && (op == CalcToken::Kind::Divide))
    {
        *error = "division by zero";
        return false;
    }

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
        if (tKind == CalcToken::Kind::Number) //if it's a number, just push
        {
            values.push(tVal);
            continue;
        }

        if (tKind == CalcToken::Kind::LParen) //if it's a (, just push
        {
            ops.push(token);
            continue;
        }

        if (tKind == CalcToken::Kind::RParen) //if it's a ), then...
        {
            if (ops.empty()) 
            {
                *error = "Parentheses are mismatched";
                return false;
            }
            while (!ops.empty() && ops.top().kind != CalcToken::Kind::LParen)
            {
                if (!operate(values, ops, error)) 
                {
                    return false;
                }
            }
            
            ops.pop(); //pop the (
            continue;
        }
        
          
        if((tKind == CalcToken::Kind::Plus)
        || (tKind == CalcToken::Kind::Minus)
        || (tKind == CalcToken::Kind::Times)
        || (tKind == CalcToken::Kind::Divide))
        {
            if (ops.empty())
            {
                ops.push(token);
                continue;
            }

            int prec = ((tKind == CalcToken::Kind::Plus) 
            || (tKind == CalcToken::Kind::Minus)) ? 1 : 2;

            int topPrec = ((ops.top().kind == CalcToken::Kind::Plus) 
            || (ops.top().kind == CalcToken::Kind::Minus)) ? 1 : 2;

            while (!ops.empty() //while there is an operator on the stack AND the top operator is higher priority than the incoming one AND the top parameter is not opening paretheses
            && (topPrec >= prec) 
            && (ops.top().kind != CalcToken::Kind::LParen))
            {
                topPrec = ((ops.top().kind == CalcToken::Kind::Plus) 
                    || (ops.top().kind == CalcToken::Kind::Minus)) ? 1 : 2;
                bool success = operate(values, ops, error);
                if (!success)
                {
                    return false;
                }
                //if (!ops.empty())
                //{
                 //   topPrec = ((ops.top().kind == CalcToken::Kind::Plus) 
                   // || (ops.top().kind == CalcToken::Kind::Minus)) ? 1 : 2;
                //}
            }
            ops.push(token);
            continue;
        }
    }

    //do remaining operations now that there should be no parentheses
    while (!ops.empty()) 
    {
        //if there is still an open parentheses, error
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
    // *error = "calc_evaluate not implemented yet";   // TODO
    return true;
}
