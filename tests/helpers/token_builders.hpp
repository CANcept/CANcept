#pragma once
#include <memory>

#include "math/types/tokens/internal/function_token.hpp"
#include "math/types/tokens/internal/operator_token.hpp"
#include "math/types/tokens/token.hpp"

namespace TestHelpers {
static auto makeAdd(std::unique_ptr<Math::TokenBase> lhs,
                    std::unique_ptr<Math::TokenBase> rhs) -> std::unique_ptr<Math::OperatorToken>
{
    auto op = std::make_unique<Math::OperatorToken>(Math::Operation::Add);
    op->setChild(0, std::move(lhs));
    op->setChild(1, std::move(rhs));
    return op;
}

static auto makeMul(std::unique_ptr<Math::TokenBase> lhs,
                    std::unique_ptr<Math::TokenBase> rhs) -> std::unique_ptr<Math::OperatorToken>
{
    auto op = std::make_unique<Math::OperatorToken>(Math::Operation::Mul);
    op->setChild(0, std::move(lhs));
    op->setChild(1, std::move(rhs));
    return op;
}

static auto makeSub(std::unique_ptr<Math::TokenBase> lhs,
                    std::unique_ptr<Math::TokenBase> rhs) -> std::unique_ptr<Math::OperatorToken>
{
    auto op = std::make_unique<Math::OperatorToken>(Math::Operation::Sub);
    op->setChild(0, std::move(lhs));
    op->setChild(1, std::move(rhs));
    return op;
}

static auto makeDiv(std::unique_ptr<Math::TokenBase> lhs,
                    std::unique_ptr<Math::TokenBase> rhs) -> std::unique_ptr<Math::OperatorToken>
{
    auto op = std::make_unique<Math::OperatorToken>(Math::Operation::Div);
    op->setChild(0, std::move(lhs));
    op->setChild(1, std::move(rhs));
    return op;
}

static auto makeFunc(Math::Function func,
                     std::unique_ptr<Math::TokenBase> arg) -> std::unique_ptr<Math::FunctionToken>
{
    auto token = std::make_unique<Math::FunctionToken>(func);
    token->setChild(0, std::move(arg));
    return token;
}
}  // namespace TestHelpers