/** Copyright 2026 Lino Wertz, Florian Fehrle, Junes Sheikhi, Adrian Rupp and Nele Spatzier
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include <memory>

#include "math/types/tokens/internal/function_token.hpp"
#include "math/types/tokens/internal/operator_token.hpp"
#include "math/types/tokens/leaf/value_token.hpp"
#include "math/types/tokens/leaf/variable_token.hpp"

using namespace Math;

// ValueToken
TEST(ValueTokenTest, ToExpressionReturnsNumericString)
{
    const ValueToken token(42.0);
    const auto expr = token.toExpression();
    EXPECT_DOUBLE_EQ(std::stod(expr), 42.0);
}

TEST(ValueTokenTest, ValueAccessor)
{
    const ValueToken token(-7.5);
    EXPECT_DOUBLE_EQ(token.value(), -7.5);
    EXPECT_DOUBLE_EQ(std::stod(token.toExpression()), -7.5);
}

TEST(ValueTokenTest, ZeroValue)
{
    const ValueToken token(0.0);
    EXPECT_DOUBLE_EQ(token.value(), 0.0);
}

// VariableToken
TEST(VariableTokenTest, ToExpressionReturnsName)
{
    const auto val = std::make_shared<double>(5.0);
    const VariableToken token("x", val.get());
    EXPECT_EQ(token.toExpression(), "x");
}

TEST(VariableTokenTest, GetReturnsPointerToValue)
{
    const auto val = std::make_shared<double>(10.0);
    const VariableToken token("y", val.get());
    EXPECT_EQ(token.get(), val.get());
    EXPECT_DOUBLE_EQ(*token.get(), 10.0);
}

TEST(VariableTokenTest, SharedOwnershipKeepsValueAlive)
{
    auto val = std::make_shared<double>(42.0);
    const VariableToken token("z", val.get());
    const double* raw = token.get();
    val.reset();
    EXPECT_DOUBLE_EQ(*raw, 42.0);
}

TEST(VariableTokenTest, CollectVariablesAddsEntry)
{
    const auto val = std::make_shared<double>(1.0);
    const VariableToken token("a", val.get());

    std::vector<std::pair<std::string, double*>> vars;
    token.collectVariables(vars);
    ASSERT_EQ(vars.size(), 1);
    EXPECT_EQ(vars[0].first, "a");
    EXPECT_EQ(vars[0].second, val.get());
}

// OperatorToken
class OperatorTokenTest : public ::testing::Test
{
   protected:
    static auto makeOp(Operation op, double lhs, double rhs) -> std::unique_ptr<OperatorToken>
    {
        auto token = std::make_unique<OperatorToken>(op);
        token->setChild(0, std::make_unique<ValueToken>(lhs));
        token->setChild(1, std::make_unique<ValueToken>(rhs));
        return token;
    }
};

TEST_F(OperatorTokenTest, ExpectedChildCountIsTwo)
{
    const OperatorToken token(Operation::Add);
    EXPECT_EQ(token.expectedChildCount(), 2);
}

TEST_F(OperatorTokenTest, AddExpression)
{
    const auto token = makeOp(Operation::Add, 1.0, 2.0);
    const auto expr = token->toExpression();
    EXPECT_NE(expr.find('+'), std::string::npos);
}

TEST_F(OperatorTokenTest, SubExpression)
{
    const auto token = makeOp(Operation::Sub, 5.0, 3.0);
    const auto expr = token->toExpression();
    EXPECT_NE(expr.find('-'), std::string::npos);
}

TEST_F(OperatorTokenTest, MulExpression)
{
    const auto token = makeOp(Operation::Mul, 4.0, 2.0);
    const auto expr = token->toExpression();
    EXPECT_NE(expr.find('*'), std::string::npos);
}

TEST_F(OperatorTokenTest, DivExpression)
{
    const auto token = makeOp(Operation::Div, 8.0, 4.0);
    const auto expr = token->toExpression();
    EXPECT_NE(expr.find('/'), std::string::npos);
}

TEST_F(OperatorTokenTest, OperationAccessor)
{
    OperatorToken add(Operation::Add);
    EXPECT_EQ(add.operation(), Operation::Add);

    OperatorToken div(Operation::Div);
    EXPECT_EQ(div.operation(), Operation::Div);
}

TEST_F(OperatorTokenTest, CollectsVariablesFromChildren)
{
    auto token = std::make_unique<OperatorToken>(Operation::Add);
    auto val = std::make_shared<double>(1.0);
    token->setChild(0, std::make_unique<VariableToken>("x", val.get()));
    token->setChild(1, std::make_unique<ValueToken>(2.0));

    std::vector<std::pair<std::string, double*>> vars;
    token->collectVariables(vars);
    ASSERT_EQ(vars.size(), 1);
    EXPECT_EQ(vars[0].first, "x");
}

// FunctionToken
class FunctionTokenTest : public ::testing::Test
{
   protected:
    static auto makeFunc(Function func, double arg) -> std::unique_ptr<FunctionToken>
    {
        auto token = std::make_unique<FunctionToken>(func);
        token->setChild(0, std::make_unique<ValueToken>(arg));
        return token;
    }
};

TEST_F(FunctionTokenTest, ExpectedChildCountIsOne)
{
    const FunctionToken token(Function::Sin);
    EXPECT_EQ(token.expectedChildCount(), 1);
}

TEST_F(FunctionTokenTest, SinExpression)
{
    const auto token = makeFunc(Function::Sin, 1.0);
    EXPECT_TRUE(token->toExpression().starts_with("sin("));
}

TEST_F(FunctionTokenTest, CosExpression)
{
    const auto token = makeFunc(Function::Cos, 0.0);
    EXPECT_TRUE(token->toExpression().starts_with("cos("));
}

TEST_F(FunctionTokenTest, AbsExpression)
{
    const auto token = makeFunc(Function::Abs, -5.0);
    EXPECT_TRUE(token->toExpression().starts_with("abs("));
}

TEST_F(FunctionTokenTest, LogExpression)
{
    const auto token = makeFunc(Function::Log, 10.0);
    EXPECT_TRUE(token->toExpression().starts_with("log("));
}

TEST_F(FunctionTokenTest, SqrtExpression)
{
    const auto token = makeFunc(Function::Sqrt, 4.0);
    EXPECT_TRUE(token->toExpression().starts_with("sqrt("));
}

TEST_F(FunctionTokenTest, FunctionAccessor)
{
    const FunctionToken token(Function::Abs);
    EXPECT_EQ(token.function(), Function::Abs);
}

TEST_F(FunctionTokenTest, CollectsVariablesFromChild)
{
    auto token = std::make_unique<FunctionToken>(Function::Sin);
    auto val = std::make_shared<double>(1.0);
    token->setChild(0, std::make_unique<VariableToken>("t", val.get()));

    std::vector<std::pair<std::string, double*>> vars;
    token->collectVariables(vars);
    ASSERT_EQ(vars.size(), 1);
    EXPECT_EQ(vars[0].first, "t");
}

// Token child management
TEST(InternalTokenTest, AddChildAppendsToEnd)
{
    OperatorToken token(Operation::Add);
    token.addChild(std::make_unique<ValueToken>(1.0));
    token.addChild(std::make_unique<ValueToken>(2.0));
    EXPECT_EQ(token.children().size(), 2);
}

TEST(InternalTokenTest, SetChildResizesIfNeeded)
{
    OperatorToken token(Operation::Add);
    token.setChild(3, std::make_unique<ValueToken>(9.0));
    EXPECT_GE(token.children().size(), 4);
    EXPECT_NE(token.children()[3], nullptr);
}

TEST(InternalTokenTest, RemoveChildNullifiesSlot)
{
    OperatorToken token(Operation::Add);
    token.addChild(std::make_unique<ValueToken>(1.0));
    token.addChild(std::make_unique<ValueToken>(2.0));
    token.removeChild(0);
    EXPECT_EQ(token.children()[0], nullptr);
    EXPECT_NE(token.children()[1], nullptr);
}

TEST(InternalTokenTest, RemoveOutOfRangeDoesNothing)
{
    OperatorToken token(Operation::Add);
    token.addChild(std::make_unique<ValueToken>(1.0));
    token.removeChild(99);
    EXPECT_EQ(token.children().size(), 1);
}
