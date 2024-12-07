#pragma once

#include <string>
#include <vector>
#include <memory>
#include <vector>
#include <variant>
#include "lexer.hpp"

namespace AST
{
    enum class ExprType
    {
        Literal,
        Identifier,
        Binary,
        Enclosed,
        FunctionCall
    };
    using Literal = std::variant<int, float, std::string>;

    class Expr
    {
    public:
        ExprType type;
        virtual ~Expr() = default; // Virtual destructor for polymorphism
    };

    // Binary Expression
    class BinaryExpr : public Expr
    {
    public:
        BinaryExpr(std::shared_ptr<Expr> l, token_t operation, std::shared_ptr<Expr> r)
            : left(std::move(l)), op(operation), right(std::move(r))
        {
            type = ExprType::Binary;
        }

        std::shared_ptr<Expr> left;
        token_t op;
        std::shared_ptr<Expr> right;
    };

    // Literal Expression
    class LiteralExpr : public Expr
    {
    public:
        explicit LiteralExpr(Literal value) : literal(std::move(value))
        {
            type = ExprType::Literal;
        }

        Literal literal;
    };

    // Enclosed Expression
    class EnclosedExpr : public Expr
    {
    public:
        explicit EnclosedExpr(std::shared_ptr<Expr> inner)
            : inner_expr(std::move(inner))
        {
            type = ExprType::Enclosed;
        }

        std::shared_ptr<Expr> inner_expr;
    };

    // Identifier Expression
    class IdentifierExpr : public Expr
    {
    public:
        explicit IdentifierExpr(const std::string &name) : name(name)
        {
            type = ExprType::Identifier;
        }

        std::string name;
    };

    // Call Expression
    class CallExpr : public Expr
    {
    public:
        CallExpr(std::shared_ptr<Expr> callee, std::vector<std::shared_ptr<Expr>> arguments)
            : callee(std::move(callee)), arguments(std::move(arguments))
        {
            type = ExprType::FunctionCall;
        }

        std::shared_ptr<Expr> callee;                 // The function or identifier being called
        std::vector<std::shared_ptr<Expr>> arguments; // List of arguments
    };

    enum class StmtType
    {
        Variable,
        Print,
        Block,
        ExprStmt,
        ProgramStmt
    };

    class Stmt
    {
    public:
        StmtType type;
        virtual ~Stmt() = default; // Virtual destructor for polymorphism
    };

    class VariableStmt : public Stmt
    {
    public:
        VariableStmt(std::string name, std::shared_ptr<Expr> initializer)
            : name(std::move(name)), initializer(std::move(initializer)), isConst(false)
        {
            type = StmtType::Variable;
        }

        std::string name;
        std::shared_ptr<Expr> initializer; // Expression initializing the variable
        bool isConst;
    };

    class BlockStmt : public Stmt
    {
    public:
        explicit BlockStmt(std::vector<std::shared_ptr<Stmt>> stmts)
            : statements(std::move(stmts))
        {
            type = StmtType::Block;
        }

        std::vector<std::shared_ptr<Stmt>> statements;
    };

    class ExprStmt : public Stmt
    {
    public:
        explicit ExprStmt(Expr expr)
            : expr(std::make_shared<Expr>(std::move(expr)))
        {
            type = StmtType::ExprStmt;
        }

        std::shared_ptr<Expr> expr;
    };

    class ProgramStmt : public Stmt
    {
    public:
        ProgramStmt() : body(std::make_shared<std::vector<std::shared_ptr<Stmt>>>())
        {
            type = StmtType::ProgramStmt;
        }

        void addStmt(const Stmt &stmt)
        {
            auto st = std::make_shared<Stmt>(std::move(stmt));
            body->emplace_back(st); // Add statement to the program body
        }

        const std::vector<std::shared_ptr<Stmt>> &getBody() const
        {
            return *body;
        }

        std::shared_ptr<std::vector<std::shared_ptr<Stmt>>> body;
    };
}
