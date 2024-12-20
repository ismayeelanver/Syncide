#include "include/parser/parser.hpp"

bool Parser::notEof()
{
    return pos < tokens->size() && at().Kind != token_t::Tkn_Eof;
}

void Parser::consume(token_t Kind)
{
    auto current = at();
    advance();
    if (notEof() && current.Kind != Kind)
    {
        ExpectedFound(filename,
                      current.Position.line,
                      current.Position.col,
                      TokenToString(Kind),
                      TokenToString(current.Kind));
    }
}

void Parser::advance()
{
    pos++;
}

token_visual_t Parser::at()
{
    return tokens->at(pos);
}

token_visual_t Parser::eat()
{
    auto currentToken = at();
    advance();
    return currentToken;
}

std::shared_ptr<Stmt> Parser::variableOrfunction()
{
    consume(token_t::Tkn_Let); // Consume 'let'
    auto name = eat();         // Get function or variable name

    if (name.Kind != token_t::Tkn_Identifier)
    {
        ExpectedFound(filename, name.Position.line, name.Position.col,
                      "Identifier", TokenToString(name.Kind));
        return nullptr;
    }

    // Check if it's a function (has parameters)
    bool isFunction = at().Kind == token_t::Tkn_Lparen;

    if (isFunction)
    {
        std::vector<std::pair<std::string, std::shared_ptr<AST::Type>>> parameters;
        consume(token_t::Tkn_Lparen); // Consume '('

        // Parse parameters
        while (notEof() && at().Kind != token_t::Tkn_Rparen)
        {
            // Get parameter name
            auto paramName = eat();
            if (paramName.Kind != token_t::Tkn_Identifier)
            {
                ExpectedFound(filename, paramName.Position.line, paramName.Position.col,
                              "Identifier", TokenToString(paramName.Kind));
                return nullptr;
            }

            // Expect ':'
            consume(token_t::Tkn_Colon);

            // Get parameter type
            auto paramType = parseType();
            if (!paramType)
            {
                ExpectedFound(filename, at().Position.line, at().Position.col,
                              "Valid type", TokenToString(at().Kind));
                return nullptr;
            }

            parameters.push_back({paramName.Value, paramType});

            // Handle comma for multiple parameters
            if (at().Kind == token_t::Tkn_Comma)
            {
                advance(); // Consume comma
            }
        }

        consume(token_t::Tkn_Rparen); // Consume ')'

        // Parse return type if present
        std::shared_ptr<AST::Type> returnType = nullptr; // Default to nullptr if no return type
        if (at().Kind == token_t::Tkn_Tilde)
        {
            advance(); // Consume '~'
            returnType = parseType();
            if (!returnType)
            {
                ExpectedFound(filename, at().Position.line, at().Position.col,
                              "Valid return type", TokenToString(at().Kind));
                return nullptr;
            }
        }

        // Expect '::' (constant assignment)
        consume(token_t::Tkn_Const_Assignment);

        // Parse function body
        consume(token_t::Tkn_Begin); // Consume 'begin'

        std::vector<std::shared_ptr<Stmt>> body;
        while (notEof() && at().Kind != token_t::Tkn_End)
        {
            auto statement = stmt();
            if (statement)
            {
                body.push_back(statement);
            }
        }

        consume(token_t::Tkn_End); // Consume 'end'

        return std::make_shared<FunctionStmt>(name.Value, parameters, returnType, body);
    }
    else
    {
        // Handle variable declaration (existing code)
        std::shared_ptr<AST::Type> variableType = std::make_shared<AST::IdentifierType>("unknown");
        bool constant = false;

        // Check if variable has a type
        if (at().Kind == token_t::Tkn_Tilde)
        {
            advance(); // Consume '~'
            if (at().Kind != token_t::Tkn_Identifier)
            {
                ExpectedFound(filename, at().Position.line, at().Position.col,
                              "Identifier", TokenToString(at().Kind));
                return nullptr;
            }
            variableType = parseType();
        }

        // Check for constant or mutable assignment
        if (at().Kind == token_t::Tkn_Const_Assignment || at().Kind == token_t::Tkn_Mut_Assignment)
        {
            constant = (at().Kind == token_t::Tkn_Const_Assignment);
            advance(); // Consume the assignment symbol
        }
        else
        {
            ExpectedFound(filename, at().Position.line, at().Position.col,
                          ":: or :=", TokenToString(at().Kind));
            return nullptr;
        }

        // Parse the expression for the variable's value
        auto valueExpr = expr();
        consume(token_t::Tkn_Semi); // Consume ';' after the expression

        return std::make_shared<VariableStmt>(name.Value, valueExpr, constant, variableType);
    }
}

std::shared_ptr<Stmt> Parser::returnStmt()
{
    consume(token_t::Tkn_Ret);

    auto exprofthisshit = expr();
    consume(token_t::Tkn_Semi);

    return std::make_shared<ReturnStmt>(exprofthisshit);
}

std::shared_ptr<Stmt> Parser::stmtsOutside()
{
    if (at().Kind == token_t::Tkn_Let)
    {
        auto stmt = variableOrfunction();
        return stmt;
    }
    else
    {
        std::cerr << "Parse Error!\n";
        exit(EXIT_FAILURE);
    }
}

std::shared_ptr<Stmt> Parser::stmt()
{
    if (at().Kind == token_t::Tkn_Let)
    {
        auto stmt = variableOrfunction();
        return stmt;
    }
    if (at().Kind == token_t::Tkn_If)
    {
        consume(token_t::Tkn_If);
        auto stmt = ifStmt();
        return stmt;
    }
    if (at().Kind == token_t::Tkn_Ret)
    {
        auto stmt = returnStmt();
        return stmt;
    }
    
    auto expression = expr(); // Get the expression
    if (expression == nullptr)
    {
        return nullptr;  // If expression parsing failed, return nullptr
    }

    // If we got a valid expression, consume semicolon if present
    if (at().Kind == token_t::Tkn_Semi)
    {
        consume(token_t::Tkn_Semi);
    }
    
    // Return the expression statement
    return std::make_shared<ExprStmt>(std::move(expression));
}


std::shared_ptr<Stmt> Parser::ifStmt()
{
    consume(token_t::Tkn_Lparen);
    auto condition = expr();
    if (!condition)
    {
        ExpectedFound(filename, at().Position.line, at().Position.col,
                      "condition expression", TokenToString(at().Kind));
        return nullptr;
    }
    consume(token_t::Tkn_Rparen);

    consume(token_t::Tkn_Then);

    // Parse 'then' block
    std::vector<std::shared_ptr<Stmt>> thenBlock;
    while (notEof() &&
           at().Kind != token_t::Tkn_End &&
           at().Kind != token_t::Tkn_Elif &&
           at().Kind != token_t::Tkn_Else)
    {
        auto statement = stmt();
        if (statement)
        {
            thenBlock.push_back(statement);
        }
    }

    // Handle elif and else blocks
    std::vector<std::shared_ptr<Stmt>> elseBlock;

    if (at().Kind == token_t::Tkn_Elif)
    {
        consume(token_t::Tkn_Elif);
        // Create a nested if statement
        auto nestedIf = ifStmt();
        if (nestedIf)
        {
            elseBlock.push_back(nestedIf);
        }
    }
    else if (at().Kind == token_t::Tkn_Else)
    {
        consume(token_t::Tkn_Else);
        while (notEof() && at().Kind != token_t::Tkn_End)
        {
            auto statement = stmt();
            if (statement)
            {
                elseBlock.push_back(statement);
            }
        }
    }

    // All branches must end with 'end'
    consume(token_t::Tkn_End);

    return std::make_shared<IfStmt>(condition, thenBlock, elseBlock);
}

std::shared_ptr<Expr> Parser::expr()
{
    return binary(0);
}

std::shared_ptr<Expr> Parser::makeAfterIdentifier()
{
    auto identifier = eat();
    if (at().Kind == token_t::Tkn_Lparen)
    {
        // Parse function call
        consume(token_t::Tkn_Lparen); // Consume '('
        std::vector<std::shared_ptr<Expr>> arguments;

        // Parse arguments
        while (notEof() && at().Kind != token_t::Tkn_Rparen)
        {
            arguments.push_back(expr()); // Parse the argument
            if (at().Kind == token_t::Tkn_Comma)
            {
                consume(token_t::Tkn_Comma); // Consume ','
            }
        }

        consume(token_t::Tkn_Rparen); // Consume ')'

        return std::make_shared<CallExpr>(identifier.Value, arguments);
    }
    return std::make_shared<IdentifierExpr>(identifier.Value);
}

// statements inside the globalScope
// are funcitons, lets, structs and that's all for a minimal programming language

std::shared_ptr<Expr> Parser::primary()
{
    auto tk = at().Kind;

    switch (tk)
    {
    case token_t::Tkn_Question:
    case token_t::Tkn_Minus:
    {
        auto op = eat();
        auto operand = primary();
        return std::make_shared<UnaryExpr>(op, operand);
    }
    case token_t::Tkn_Identifier:
    {
        auto expression = makeAfterIdentifier();
        return expression;
    }
    case token_t::Tkn_Number:
    {
        return std::make_shared<LiteralExpr>(std::stoi(eat().Value));
    }
    case token_t::Tkn_Float:
    {
        return std::make_shared<LiteralExpr>(std::stold(eat().Value));
    }
    case token_t::Tkn_String:
    {
        return std::make_shared<LiteralExpr>(eat().Value);
    }
    case token_t::Tkn_Nil:
    {
        advance();
        return std::make_shared<NilExpr>();
    }
    case token_t::Tkn_True:
    case token_t::Tkn_False:
    {
        return std::make_shared<BooleanExpr>(eat().Value);
    }
    case token_t::Tkn_LCurly:
    {
        consume(token_t::Tkn_LCurly);
        std::vector<std::shared_ptr<Expr>> elements;

        while (notEof() && at().Kind != token_t::Tkn_RCurly)
        {
            elements.push_back(expr()); // Parse each element
            if (at().Kind == token_t::Tkn_Comma)
            {
                consume(token_t::Tkn_Comma); // Consume ',' between elements
            }
        }
        consume(token_t::Tkn_RCurly);
        return std::make_shared<ArrayExpr>(elements);
    }
    case token_t::Tkn_Lparen:
    {
        consume(token_t::Tkn_Lparen);
        auto inner = expr();
        consume(token_t::Tkn_Rparen);
        return std::make_shared<ParenthesizedExpr>(inner);
    }
    default:
    {
        ExpectedFound(filename, at().Position.line, at().Position.col, "an Expression", TokenToString(at().Kind));
        return nullptr;
    }
    }
}

std::shared_ptr<Expr> Parser::binary(int p = 0)
{
    auto left = primary();
    if (!left)
        return nullptr;

    while (notEof())
    {
        auto op = at();
        int opPrecedence = getPrecedence(op.Kind);
        if (opPrecedence < p)
            break;

        consume(op.Kind);
        auto right = binary(opPrecedence + 1);

        left = std::make_shared<BinaryExpr>(left, op, right);
    }
    return left;
}

Program Parser::produceAST(std::vector<token_visual_t> tks)
{
    Program p;
    pos = 0;
    tokens = std::make_shared<std::vector<token_visual_t>>(std::move(tks));

    while (notEof())
    {
        auto statement = stmtsOutside();
        p.addStmt(std::move(statement));
    }

    return p;
}

std::shared_ptr<AST::Type> Parser::parseType()
{
    token_visual_t idName = at();

    std::shared_ptr<AST::Type> typeToBeSent;

    if (idName.Kind != token_t::Tkn_Identifier)
    {
        fmt::print("{}\n", idName.Value);
        ExpectedFound(filename, idName.Position.line, idName.Position.col, "a valid type", TokenToString(idName.Kind));
    }
    else
    {
        advance();

        if (at().Kind == token_t::Tkn_Lparen)
        {
            advance();
            std::vector<std::shared_ptr<AST::Type>> paramTypes;
            while (notEof() && at().Kind != token_t::Tkn_Rparen)
            {
                paramTypes.push_back(parseType());
                if (at().Kind == token_t::Tkn_Comma)
                {
                    advance();
                }
                else
                {
                    break;
                }
            }

            consume(token_t::Tkn_Rparen);
            auto returnType = std::make_shared<AST::IdentifierType>(idName.Value);
            typeToBeSent = std::make_shared<AST::FunctionPointerType>(returnType, paramTypes);
        }
        else if (at().Kind == token_t::Tkn_Langle)
        {
            advance();
            std::vector<std::shared_ptr<AST::Type>> templateArgs;
            while (notEof() && at().Kind != token_t::Tkn_Rangle)
            {
                templateArgs.push_back(parseType());
                if (at().Kind == token_t::Tkn_Comma)
                {
                    advance();
                }
                else
                {
                    break;
                }
            }

            consume(token_t::Tkn_Rangle);
            typeToBeSent = std::make_shared<AST::TemplateType>(idName.Value, templateArgs);
        }
        else
        {
            typeToBeSent = std::make_shared<AST::IdentifierType>(idName.Value);
        }

        return typeToBeSent;
    }
}
