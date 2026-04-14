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

#include <cmath>
#include <memory>

#include "math/service/value_function.hpp"
#include "math/types/tokens/internal/function_token.hpp"
#include "math/types/tokens/internal/operator_token.hpp"
#include "math/types/tokens/leaf/value_token.hpp"
#include "math/types/tokens/leaf/variable_token.hpp"
#include "tests/helpers/token_builders.hpp"

using namespace Math;

// Parse & Evaluate — constants only
TEST(ValueFunctionTest, ParseSimpleValue)
{
    const ValueToken root(42.0);
    ValueFunction valueFunction(&root);
    const auto [success, error] = valueFunction.parse();
    EXPECT_TRUE(success);
    EXPECT_TRUE(valueFunction.isParsed());
    EXPECT_DOUBLE_EQ(valueFunction.evaluate(), 42.0);
}

TEST(ValueFunctionTest, ParseAddition)
{
    const auto root =
        TestHelpers::makeAdd(std::make_unique<ValueToken>(3.0), std::make_unique<ValueToken>(7.0));
    ValueFunction valueFunction(root.get());
    auto [success, error] = valueFunction.parse();
    EXPECT_TRUE(success);
    EXPECT_DOUBLE_EQ(valueFunction.evaluate(), 10.0);
}

TEST(ValueFunctionTest, ParseMultiplication)
{
    const auto root =
        TestHelpers::makeMul(std::make_unique<ValueToken>(4.0), std::make_unique<ValueToken>(5.0));
    ValueFunction vf(root.get());
    EXPECT_TRUE(vf.parse().success);
    EXPECT_DOUBLE_EQ(vf.evaluate(), 20.0);
}

TEST(ValueFunctionTest, ParseNestedExpression)
{
    auto sum =
        TestHelpers::makeAdd(std::make_unique<ValueToken>(2.0), std::make_unique<ValueToken>(3.0));
    const auto root = TestHelpers::makeMul(std::move(sum), std::make_unique<ValueToken>(4.0));
    ValueFunction valueFunction(root.get());
    EXPECT_TRUE(valueFunction.parse().success);
    EXPECT_DOUBLE_EQ(valueFunction.evaluate(), 20.0);
}

TEST(ValueFunctionTest, ParseSinFunction)
{
    const auto root = TestHelpers::makeFunc(Function::Sin, std::make_unique<ValueToken>(0.0));
    ValueFunction valueFunction(root.get());
    EXPECT_TRUE(valueFunction.parse().success);
    EXPECT_NEAR(valueFunction.evaluate(), 0.0, 1e-10);
}

TEST(ValueFunctionTest, ParseSqrtFunction)
{
    const auto root = TestHelpers::makeFunc(Function::Sqrt, std::make_unique<ValueToken>(9.0));
    ValueFunction valueFunction(root.get());
    EXPECT_TRUE(valueFunction.parse().success);
    EXPECT_DOUBLE_EQ(valueFunction.evaluate(), 3.0);
}

TEST(ValueFunctionTest, ParseAbsFunction)
{
    const auto root = TestHelpers::makeFunc(Function::Abs, std::make_unique<ValueToken>(-5.0));
    ValueFunction valueFunction(root.get());
    EXPECT_TRUE(valueFunction.parse().success);
    EXPECT_DOUBLE_EQ(valueFunction.evaluate(), 5.0);
}

// Parse & Evaluate — with variables
TEST(ValueFunctionTest, ParseWithVariable)
{
    const auto val = std::make_shared<double>(7.0);
    const VariableToken root("x", val);
    ValueFunction valueFunction(&root);
    EXPECT_TRUE(valueFunction.parse().success);
    EXPECT_DOUBLE_EQ(valueFunction.evaluate(), 7.0);
}

TEST(ValueFunctionTest, VariableUpdateReflectedInEvaluation)
{
    auto val = std::make_shared<double>(1.0);
    const auto root = TestHelpers::makeAdd(std::make_unique<VariableToken>("x", val),
                                           std::make_unique<ValueToken>(10.0));
    ValueFunction valueFunction(root.get());
    EXPECT_TRUE(valueFunction.parse().success);
    EXPECT_DOUBLE_EQ(valueFunction.evaluate(), 11.0);
    *val = 5.0;
    EXPECT_DOUBLE_EQ(valueFunction.evaluate(), 15.0);
}

TEST(ValueFunctionTest, MultipleVariables)
{
    auto x = std::make_shared<double>(3.0);
    auto y = std::make_shared<double>(4.0);
    const auto root = TestHelpers::makeAdd(std::make_unique<VariableToken>("x", x),
                                           std::make_unique<VariableToken>("y", y));
    ValueFunction vf(root.get());
    EXPECT_TRUE(vf.parse().success);
    EXPECT_DOUBLE_EQ(vf.evaluate(), 7.0);
}

// Expression string
TEST(ValueFunctionTest, ExpressionStringNotEmptyAfterParse)
{
    const auto root =
        TestHelpers::makeAdd(std::make_unique<ValueToken>(1.0), std::make_unique<ValueToken>(2.0));
    ValueFunction vf(root.get());
    vf.parse();
    EXPECT_FALSE(vf.expression().empty());
}

// Parse state
TEST(ValueFunctionTest, IsParsedFalseBeforeParse)
{
    const ValueToken root(1.0);
    const ValueFunction valueFunction(&root);
    EXPECT_FALSE(valueFunction.isParsed());
}

TEST(ValueFunctionTest, LastResultReflectsLatestParse)
{
    const ValueToken root(1.0);
    ValueFunction valueFunction(&root);
    valueFunction.parse();
    EXPECT_TRUE(valueFunction.lastResult().success);
    EXPECT_TRUE(valueFunction.lastResult().error.empty());
}
