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
    auto name = eat();         // Get function name

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
        std::vector<std::pair<std::string, std::string>> parameters;
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
            auto paramType = eat();
            if (paramType.Kind != token_t::Tkn_Identifier)
            {
                ExpectedFound(filename, paramType.Position.line, paramType.Position.col,
                              "Type", TokenToString(paramType.Kind));
                return nullptr;
            }

            parameters.push_back({paramName.Value, paramType.Value});

            // Handle comma for multiple parameters
            if (at().Kind == token_t::Tkn_Comma)
            {
                advance(); // Consume comma
            }
        }

        consume(token_t::Tkn_Rparen); // Consume ')'

        // Parse return type if present
        std::string returnType = "void"; // Default return type
        if (at().Kind == token_t::Tkn_Tilde)
        {
            advance(); // Consume '~'
            auto type = eat();
            if (type.Kind != token_t::Tkn_Identifier)
            {
                ExpectedFound(filename, type.Position.line, type.Position.col,
                              "Type", TokenToString(type.Kind));
                return nullptr;
            }
            returnType = type.Value;
        }

        // Expect '::'
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
        std::string VariableType = "Unknown";
        bool constant = false;

        if (at().Kind == token_t::Tkn_Tilde)
        {
            advance();
            if (at().Kind != token_t::Tkn_Identifier)
            {
                ExpectedFound(filename, at().Position.line, at().Position.col,
                              "Identifier", TokenToString(at().Kind));
                return nullptr;
            }
            VariableType = eat().Value;
        }

        if (at().Kind == token_t::Tkn_Const_Assignment ||
            at().Kind == token_t::Tkn_Mut_Assignment)
        {
            constant = (at().Kind == token_t::Tkn_Const_Assignment);
            advance();
        }
        else
        {
            ExpectedFound(filename, at().Position.line, at().Position.col,
                          ":: or :=", TokenToString(at().Kind));
            return nullptr;
        }

        auto valueExpr = expr();
        consume(token_t::Tkn_Semi);
        return std::make_shared<VariableStmt>(name.Value, valueExpr, constant, VariableType);
    }
}

std::shared_ptr<Stmt> Parser::stmt()
{
    if (at().Kind == token_t::Tkn_Let)
    {
        auto stmt = variableOrfunction();
        return stmt;
    }
    auto expression = expr(); // Get the expression
    consume(token_t::Tkn_Semi);
    return std::make_shared<ExprStmt>(std::move(expression));
};

std::shared_ptr<Expr> Parser::expr()
{
    return binary(0);
}

std::shared_ptr<Expr> Parser::primary()
{
    auto tk = at().Kind;

    switch (tk)
    {
    case token_t::Tkn_Identifier:
    {
        return std::make_shared<IdentifierExpr>(eat().Value);
    }
    case token_t::Tkn_Number:
    {
        return std::make_shared<LiteralExpr>(std::stoi(eat().Value));
    }
    case token_t::Tkn_Float:
    {
        return std::make_shared<LiteralExpr>(std::stof(eat().Value));
    }
    case token_t::Tkn_String:
    {
        return std::make_shared<LiteralExpr>(eat().Value);
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
        ExpectedFound(filename, at().Position.line, at().Position.col, "a Number, String or an Identifier", TokenToString(at().Kind));
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
        auto statement = stmt();
        p.addStmt(std::move(statement));
    }

    return p;
}