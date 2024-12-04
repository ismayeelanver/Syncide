#include <variant>
#include <memory>
#include <string>
#include <vector>

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
    enum class Op
    {
        Plus,
        Minus,
        Mod,
        Div,
        Mul
    };
    using Literal = std::variant<int, float, std::string>;
    class Expr
    {
    public:
        ExprType type;
        virtual ~Expr() = default;
    };
    // Binary Expression
    class BinaryExpr : public Expr
    {
    public:
        BinaryExpr(std::shared_ptr<Expr> l, Op operation, std::shared_ptr<Expr> r)
            : left(std::move(l)), op(operation), right(std::move(r))
        {
            type = ExprType::Binary;
        }

        std::shared_ptr<Expr> left;
        Op op;
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
        virtual ~Stmt() = default;
    };

    class VariableStmt : public Stmt
    {
    public:
        VariableStmt(std::string name, std::shared_ptr<Expr> initializer)
            : name(std::move(name)), initializer(std::move(initializer))
        {
            type = StmtType::Variable;
        }

        bool isConst;
        std::string name;
        std::shared_ptr<Expr> initializer; // Expression initializing the variable
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
        explicit ProgramStmt() : body(std::make_shared<std::vector<Stmt>>())
        {
            type = StmtType::ProgramStmt;
        }

        void addStmt(const Stmt &stmt)
        {
            std::vector<Stmt> Body;
            if (body)
            {
                body->emplace_back(stmt);
            }
        }

        std::shared_ptr<std::vector<Stmt>> body;
    };
}