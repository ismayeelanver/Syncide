#pragma once

#include <string>
#include <vector>
#include <memory>
#include <vector>
#include <cstdint>
#include <variant>
#include "../lexer.hpp"

namespace AST
{

    class Type
    {
    public:
        virtual ~Type() = default;
        virtual std::string toString() const = 0;
    };

    class IdentifierType : public Type
    {
    public:
        std::string name;

        explicit IdentifierType(const std::string &name) : name(name) {}

        std::string toString() const override
        {
            return "IdentifierOrType(" + name + ")";
        }
    };
    class TemplateType : public Type
    {
    public:
        std::vector<std::shared_ptr<Type>> tempargs;
        std::string typeName;

        explicit TemplateType(std::string typeName, std::vector<std::shared_ptr<AST::Type>> tempargs) : tempargs(tempargs), typeName(typeName) {}

        std::string toString() const override
        {
            std::ostringstream ss;

            ss << "Template(";
            for (size_t i = 0; i < tempargs.size(); ++i)
            {
                ss << tempargs[i]->toString();
                if (i != tempargs.size() - 1)
                {
                    ss << ", ";
                }
            }
            ss << ")\n";
            return ss.str();
        }
    };

    class FunctionPointerType : public Type
    {
    public:
        std::shared_ptr<Type> returnType;
        std::vector<std::shared_ptr<Type>> paramTypes;

        FunctionPointerType(std::shared_ptr<Type> returnType, std::vector<std::shared_ptr<Type>> paramTypes)
            : returnType(std::move(returnType)), paramTypes(std::move(paramTypes)) {}

        std::string toString() const override
        {
            std::stringstream ss;
            ss << "FunctionType(";

            for (size_t i = 0; i < paramTypes.size(); ++i)
            {
                ss << paramTypes[i]->toString();
                if (i != paramTypes.size() - 1) // If it's not the last element
                {
                    ss << ", ";
                }
            }

            ss << ") -> " << returnType->toString();
            return ss.str();
        }
    };

    enum class ExprType
    {
        Literal,
        Array,
        Boolean,
        Unary,
        Identifier,
        Binary,
        Nil,
        Enclosed,
        FunctionCall
    };
    using Literal = std::variant<int64_t, long double, std::string>;
    inline std::unordered_map<ExprType, std::string> mapOfTypesOfExprs = {
        {ExprType::Literal, "Literal"},
        {ExprType::Identifier, "Identifier"},
        {ExprType::Binary, "Binary"},
        {ExprType::Unary, "Unary"},
        {ExprType::Boolean, "Boolean"},
        {ExprType::Array, "Array"},
        {ExprType::Nil, "Nil"},
        {ExprType::Enclosed, "Enclosed"},
        {ExprType::FunctionCall, "FunctionCall"}};
    inline std::string
    FindExprType(ExprType t)
    {
        auto a = mapOfTypesOfExprs.find(t);
        if (a != mapOfTypesOfExprs.end())
        {
            return a->second;
        }
        else
        {
            return "Expr";
        }
    }

    class Expr
    {
    public:
        ExprType type;
        virtual ~Expr() = default; // Virtual destructor for polymorphism
    };

    class NilExpr : public Expr
    {
    public:
        virtual void nil() = 0;
        NilExpr()
        {
            type = ExprType::Nil;
        }
    };

    class ConcreteNilExpr : public NilExpr
    {
    public:
        void nil() override
        {
            // Provide implementation for the pure virtual function
        }
    };

    // Binary Expression
    class BinaryExpr : public Expr
    {
    public:
        BinaryExpr(std::shared_ptr<Expr> l, token_visual_t operation, std::shared_ptr<Expr> r)
            : left(std::move(l)), op(operation), right(std::move(r))
        {
            type = ExprType::Binary;
        }

        std::shared_ptr<Expr> left;
        token_visual_t op;
        std::shared_ptr<Expr> right;
    };

    class UnaryExpr : public Expr
    {
    public:
        token_visual_t op;
        std::shared_ptr<Expr> operand;
        UnaryExpr(token_visual_t op, std::shared_ptr<Expr> operand)
            : op(op), operand(std::move(operand))
        {
            type = ExprType::Unary;
        }
    };

    class BooleanExpr : public Expr
    {
    public:
        explicit BooleanExpr(std::string boolean) : huh(boolean)
        {
            type = ExprType::Boolean;
        }
        std::string huh;
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
        CallExpr(std::string callee, std::vector<std::shared_ptr<Expr>> arguments)
            : callee(callee), arguments(std::move(arguments))
        {
            type = ExprType::FunctionCall;
        }

        std::string callee;                           // The function or identifier being called
        std::vector<std::shared_ptr<Expr>> arguments; // List of arguments
    };

    class ArrayExpr : public Expr
    {
    public:
        ArrayExpr(std::vector<std::shared_ptr<Expr>> elements) : elements(elements)
        {
            type = ExprType::Array;
        }
        std::vector<std::shared_ptr<Expr>> elements;
    };

    enum class StmtType
    {
        Variable,
        Function,
        Block,
        ExprStmt,
        ProgramStmt,
        If,
        ReturnStmt
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
        std::string name;
        std::shared_ptr<AST::Type> typedname; // Changed to shared_ptr<AST::Type> for consistency
        std::shared_ptr<Expr> initializer;    // Expression initializing the variable
        bool isConst;

        VariableStmt(const std::string &name,
                     std::shared_ptr<Expr> initializer,
                     bool constant,
                     std::shared_ptr<AST::Type> typedname)
            : name(name),
              initializer(std::move(initializer)),
              isConst(constant),
              typedname(std::move(typedname)) // Now using shared_ptr<AST::Type>
        {
            type = StmtType::Variable;
        }
    };

    class BlockStmt : public Stmt
    {
    public:
        explicit BlockStmt()
            : statements(std::make_shared<std::vector<std::shared_ptr<Stmt>>>())
        {
            type = StmtType::Block;
        }

        void addStmt(std::shared_ptr<Stmt> s)
        {
            statements->emplace_back(std::move(s));
        }

        std::shared_ptr<std::vector<std::shared_ptr<Stmt>>> statements;
    };

    class IfStmt : public Stmt
    {
    public:
        std::shared_ptr<Expr> condition;
        std::vector<std::shared_ptr<Stmt>> consequent;
        std::vector<std::shared_ptr<Stmt>> alternate;

        IfStmt(std::shared_ptr<Expr> condition,
               std::vector<std::shared_ptr<Stmt>> thenBlock,
               std::vector<std::shared_ptr<Stmt>> elseBlock)
            : condition(std::move(condition)), consequent(std::move(thenBlock)), alternate(std::move(elseBlock))
        {
            type = StmtType::If;
        }
    };

    class FunctionStmt : public Stmt
    {
    public:
        std::string name;
        std::vector<std::pair<std::string, std::shared_ptr<AST::Type>>> param;
        std::shared_ptr<AST::Type> typedname; // Changed to shared_ptr<AST::Type>
        std::shared_ptr<std::vector<std::shared_ptr<Stmt>>> body;

        FunctionStmt(const std::string &name,
                     const std::vector<std::pair<std::string, std::shared_ptr<AST::Type>>> &parameters,
                     std::shared_ptr<AST::Type> &returnType,
                     const std::vector<std::shared_ptr<Stmt>> &bodyStmts)
            : name(name),
              param(parameters),
              typedname(std::move(returnType)), // Now passing shared_ptr<AST::Type>
              body(std::make_shared<std::vector<std::shared_ptr<Stmt>>>(bodyStmts))
        {
            type = StmtType::Function;
        }

        // Helper method to add statements to the function body
        void addStmt(const std::shared_ptr<Stmt> &stmt)
        {
            body->push_back(stmt);
        }

        // Getter for body statements
        const std::vector<std::shared_ptr<Stmt>> &getBody() const
        {
            return *body;
        }
    };

    class ReturnStmt : public Stmt
    {
    public:
        ReturnStmt(std::shared_ptr<Expr> &expr) : expr(std::move(expr))
        {
            type = StmtType::ReturnStmt;
        }

        std::shared_ptr<Expr> expr;
    };

    class ExprStmt : public Stmt
    {
    public:
        explicit ExprStmt(std::shared_ptr<Expr> expr)
            : expr(std::move(expr))
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

        void addStmt(const std::shared_ptr<Stmt> &stmt)
        {
            auto st = std::move(stmt);
            body->emplace_back(st); // Add statement to the program body
        }

        const std::vector<std::shared_ptr<Stmt>> &getBody() const
        {
            return *body;
        }

        std::shared_ptr<std::vector<std::shared_ptr<Stmt>>> body;
    };

    class DebugVisitor
    {
    private:
        int indent_level = 0;
        std::string indent() const
        {
            return std::string(indent_level * 2, ' ');
        }

        // Helper for literal values
        std::string getLiteralString(const Literal &lit) const
        {
            std::stringstream ss;
            std::visit([&ss](auto &&arg)
                       {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, int64_t>)
                ss << "Integer(" << arg << ")";
            else if constexpr (std::is_same_v<T, long double>)
                ss << "Float(" << arg << ")";
            else if constexpr (std::is_same_v<T, std::string>)
                ss << "String(\"" << arg << "\")"; }, lit);
            return ss.str();
        }

        // Helper for token information
        std::string getTokenInfo(const token_t &token) const
        {
            return TokenToString(token);
        }

    public:
        void visitExpr(std::shared_ptr<Expr> &expr)
        {
            if (!expr.get())
            {
                std::cout << indent() << "null expression\n";
                return;
            }

            switch (expr->type)
            {
            case ExprType::Array:
                visitArrayExpr(std::static_pointer_cast<ArrayExpr>(expr));
                break;
            case ExprType::Boolean:
                visitBooleanExpr(std::static_pointer_cast<BooleanExpr>(expr));
                break;
            case ExprType::Binary:
                visitBinaryExpr(std::static_pointer_cast<BinaryExpr>(expr));
                break;
            case ExprType::Literal:
                visitLiteralExpr(std::static_pointer_cast<LiteralExpr>(expr));
                break;
            case ExprType::Enclosed:
                visitEnclosedExpr(std::static_pointer_cast<EnclosedExpr>(expr));
                break;
            case ExprType::Identifier:
                visitIdentifierExpr(std::static_pointer_cast<IdentifierExpr>(expr));
                break;
            case ExprType::Unary:
                visitUnaryExpr(std::static_pointer_cast<UnaryExpr>(expr));
                break;
            case ExprType::FunctionCall:
                visitCallExpr(std::static_pointer_cast<CallExpr>(expr));
                break;
            case ExprType::Nil:
                visitNilExpr();
                break;
            }
        }

        void visitArrayExpr(const std::shared_ptr<ArrayExpr> &expr)
        {
            std::cout << indent() << "ArrayExpr:\n";
            indent_level++;
            for (auto element : expr->elements)
            {
                visitExpr(element);
            }
            indent_level--;
        }

        void visitBooleanExpr(const std::shared_ptr<BooleanExpr> &expr)
        {
            std::cout << indent() << "BooleanExpr:\n";
            indent_level++;
            std::cout << indent() << expr->huh << "\n";
            indent_level--;
        }

        void visitUnaryExpr(const std::shared_ptr<UnaryExpr> &expr)
        {
            std::cout << indent() << "UnaryExpr:\n";
            indent_level++;
            std::cout << indent() << "Operator: " << expr->op.Value << "\n";
            std::cout << indent() << "Operand:\n";
            indent_level++;
            visitExpr(expr->operand);
            indent_level--;
        }

        void visitNilExpr()
        {
            std::cout << indent() << "NilExpr\n";
        }

        void visitBinaryExpr(const std::shared_ptr<BinaryExpr> &expr)
        {
            std::cout << indent() << "BinaryExpr:\n";
            indent_level++;
            std::cout << indent() << "Operator: " << getTokenInfo(expr->op.Kind)
                      << " (Line: " << expr->op.Position.line
                      << ", Col: " << expr->op.Position.col << ")\n";
            if (!expr->op.Value.empty())
            {
                std::cout << indent() << "Value: '" << expr->op.Value << "'\n";
            }
            std::cout << indent() << "Left:\n";
            indent_level++;
            visitExpr(expr->left);
            indent_level--;
            std::cout << indent() << "Right:\n";
            indent_level++;
            visitExpr(expr->right);
            indent_level--;
            indent_level--;
        }

        void visitLiteralExpr(const std::shared_ptr<LiteralExpr> &expr)
        {
            std::cout << indent() << "LiteralExpr: " << getLiteralString(expr->literal) << "\n";
        }

        void visitEnclosedExpr(const std::shared_ptr<EnclosedExpr> &expr)
        {
            std::cout << indent() << "EnclosedExpr:\n";
            indent_level++;
            visitExpr(expr->inner_expr);
            indent_level--;
        }

        void visitIdentifierExpr(const std::shared_ptr<IdentifierExpr> &expr)
        {
            std::cout << indent() << "IdentifierExpr: " << expr->name << "\n";
        }

        void visitCallExpr(const std::shared_ptr<CallExpr> &expr)
        {
            std::cout << indent() << "CallExpr:\n";
            indent_level++;
            std::cout << indent() << "Callee:\n";
            indent_level++;
            std::cout << indent() << expr->callee << "\n";
            indent_level--;

            if (!expr->arguments.empty())
            {
                std::cout << indent() << "Arguments (" << expr->arguments.size() << "):\n";
                indent_level++;
                for (size_t i = 0; i < expr->arguments.size(); i++)
                {
                    std::cout << indent() << "Arg[" << i << "]:\n";
                    indent_level++;
                    visitExpr(expr->arguments[i]);
                    indent_level--;
                }
                indent_level--;
            }
            else
            {
                std::cout << indent() << "Arguments: none\n";
            }
            indent_level--;
        }

        void visitStmt(const std::shared_ptr<Stmt> &stmt)
        {
            if (!stmt)
            {
                std::cout << indent() << "null statement\n";
                return;
            }

            switch (stmt->type)
            {
            case StmtType::Variable:
                visitVariableStmt(std::static_pointer_cast<VariableStmt>(stmt));
                break;
            case StmtType::Block:
                visitBlockStmt(std::static_pointer_cast<BlockStmt>(stmt));
                break;
            case StmtType::ExprStmt:
                visitExprStmt(std::static_pointer_cast<ExprStmt>(stmt));
                break;
            case StmtType::ProgramStmt:
                visitProgramStmt(std::static_pointer_cast<ProgramStmt>(stmt));
                break;
            case StmtType::Function:
                visitFunctionStmt(std::static_pointer_cast<FunctionStmt>(stmt));
                break;
            case StmtType::ReturnStmt:
                visitReturnStmt(std::static_pointer_cast<ReturnStmt>(stmt));
                break;
            case StmtType::If:
                visitIfStmt(std::static_pointer_cast<IfStmt>(stmt));
                break;
            }
        }

        void visitIfStmt(const std::shared_ptr<IfStmt> &stmt)
        {
            std::cout << indent() << "IfStmt:\n";
            indent_level++;
            std::cout << indent() << "Condition:\n";
            indent_level++;
            visitExpr(stmt->condition);
            indent_level--;
            std::cout << indent() << "Consquent:\n";
            indent_level++;
            for (auto statement : stmt->consequent)
            {
                visitStmt(statement);
            }
            indent_level--;
            std::cout << indent() << "Alternate:\n";
            indent_level++;
            for (auto statement : stmt->alternate)
            {
                visitStmt(statement);
            }
            indent_level--;
            indent_level--;
        }

        void visitReturnStmt(const std::shared_ptr<ReturnStmt> &stmt)
        {
            std::cout << indent() << "ReturnStmt:\n";
            indent_level++;

            std::cout << indent() << "InnerExpr:\n";
            indent_level++;
            visitExpr(stmt->expr);
            indent_level--;
            indent_level--;
        }

        void visitFunctionStmt(const std::shared_ptr<FunctionStmt> &stmt)
        {
            std::cout << indent() << "FunctionStmt:\n";
            indent_level++;

            // Print function name and return type
            std::cout << indent() << "Name: " << stmt->name << "\n";
            std::cout << indent() << "Return Type: \n";
            indent_level++;
            if (stmt->typedname)
            {
                std::cout << indent() << stmt->typedname->toString() << "\n";
            }
            else
            {
                std::cout << indent() << "null\n";
            }
            indent_level--;

            // Print parameters
            std::cout << indent() << "Parameters:\n";
            indent_level++;
            for (const auto &p : stmt->param)
            {
                std::cout << indent() << "Parameter: " << p.first;
                if (p.second)
                {
                    std::cout << " (Type: " << p.second->toString() << ")\n";
                }
                else
                {
                    std::cout << " (Type: null)\n";
                }
            }
            indent_level--;

            // Print function body
            std::cout << indent() << "Body:\n";
            indent_level++;
            if (stmt->body && !stmt->body->empty())
            {
                for (const auto &statement : stmt->getBody())
                {
                    visitStmt(statement);
                }
            }
            else
            {
                std::cout << indent() << "Empty function body\n";
            }
            indent_level--;

            indent_level--;
        }

        void visitVariableStmt(const std::shared_ptr<VariableStmt> &stmt)
        {
            std::cout << indent() << "VariableStmt:\n";
            indent_level++;
            std::cout << indent() << "Name: " << stmt->name << "\n";
            std::cout << indent() << "Type: \n";
            indent_level++;
            std::cout << indent() << stmt->typedname->toString() << "\n";
            indent_level--;
            std::cout << indent() << "IsConst: " << (stmt->isConst ? "true" : "false") << "\n";
            if (stmt->initializer)
            {
                std::cout << indent() << "Initializer:\n";
                indent_level++;
                visitExpr(stmt->initializer);
                indent_level--;
            }
            else
            {
                std::cout << indent() << "Initializer: none\n";
            }
            indent_level--;
        }

        void visitBlockStmt(const std::shared_ptr<BlockStmt> &stmt)
        {
            std::cout << indent() << "BlockStmt:\n";
            indent_level++;
            if (!stmt->statements->empty())
            {
                std::cout << indent() << "Statements (" << stmt->statements->size() << "):\n";
                indent_level++;
                for (size_t i = 0; i < stmt->statements->size(); i++)
                {
                    std::cout << indent() << "Stmt[" << i << "]:\n";
                    indent_level++;
                    visitStmt(stmt->statements->at(i));
                    indent_level--;
                }
                indent_level--;
            }
            else
            {
                std::cout << indent() << "Statements: empty block\n";
            }
            indent_level--;
        }

        void visitExprStmt(const std::shared_ptr<ExprStmt> &stmt)
        {
            std::cout << indent() << "ExprStmt:\n";
            indent_level++;
            visitExpr(stmt->expr);
            indent_level--;
        }

        void visitProgramStmt(const std::shared_ptr<ProgramStmt> &stmt)
        {
            std::cout << indent() << "ProgramStmt:\n";
            indent_level++;
            const auto &body = stmt->getBody();
            if (!body.empty())
            {
                std::cout << indent() << "Body (" << body.size() << " statements):\n";
                indent_level++;
                for (size_t i = 0; i < body.size(); i++)
                {
                    std::cout << indent() << "Stmt[" << i << "]:\n";
                    indent_level++;
                    visitStmt(body[i]);
                    indent_level--;
                }
                indent_level--;
            }
            else
            {
                std::cout << indent() << "Body: empty program\n";
            }
            indent_level--;
        }
    };
}
