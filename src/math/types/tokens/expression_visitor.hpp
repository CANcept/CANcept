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

#pragma once

namespace Math {

class ValueToken;
class VariableToken;
class OperatorToken;
class FunctionToken;

/**
 * @brief Interface for visitor pattern traversal of the expression token tree.
 */
class IExpressionVisitor
{
   public:
    virtual ~IExpressionVisitor() = default;

    /**
     * @brief Processes a numeric literal token.
     */
    virtual void visit(const ValueToken& token) = 0;

    /**
     * @brief Processes a variable reference token.
     */
    virtual void visit(const VariableToken& token) = 0;

    /**
     * @brief Processes a operator token.
     */
    virtual void visit(const OperatorToken& token) = 0;

    /**
     * @brief Processes a function-call token.
     */
    virtual void visit(const FunctionToken& token) = 0;
};

}  // namespace Math
