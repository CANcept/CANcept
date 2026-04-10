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

#include "token_registry.hpp"

#include <map>

namespace Math::TokenRegistry {

auto tokens() -> const std::list<TokenEntry>&
{
    static const std::list<TokenEntry> list = {
        // operators allows + as well as \+ etc.
        {"+", "+", '+', [] { return std::make_unique<OperatorToken>(Operation::Add); }},
        {"-", "\u2212", '-', [] { return std::make_unique<OperatorToken>(Operation::Sub); }},
        {"\u22C5", "\u22C5", '*', [] { return std::make_unique<OperatorToken>(Operation::Mul); }},
        {"\u00BD", "", '/', [] { return std::make_unique<OperatorToken>(Operation::Div); }},

        // functions only with \
        {"sin", "sin", 's', [] { return std::make_unique<FunctionToken>(Function::Sin); }},
        {"cos", "cos", 'c', [] { return std::make_unique<FunctionToken>(Function::Cos); }},
        {"\u007C\u2B1A\u007C", "", 'a',
         [] { return std::make_unique<FunctionToken>(Function::Abs); }},
        {"log", "log", 'l', [] { return std::make_unique<FunctionToken>(Function::Log); }},
        {"\u221A\u2B1A", "", 'q', [] { return std::make_unique<FunctionToken>(Function::Sqrt); }},
    };
    return list;
}

auto buildOperatorMap() -> std::map<Operation, QString>
{
    std::map<Operation, QString> map;
    for (const auto& entry : tokens())
    {
        auto token = entry.factory();
        if (const auto* op = dynamic_cast<OperatorToken*>(token.get()))
            map[op->operation()] = entry.renderSymbol;
    }
    return map;
}

auto buildFunctionMap() -> std::map<Function, QString>
{
    std::map<Function, QString> map;
    for (const auto& entry : tokens())
    {
        auto token = entry.factory();
        if (const auto* fn = dynamic_cast<FunctionToken*>(token.get()))
            map[fn->function()] = entry.renderSymbol;
    }
    return map;
}

auto operatorSymbol(const Operation op) -> QString
{
    static const auto map = buildOperatorMap();
    const auto it = map.find(op);
    return it != map.end() ? it->second : "";
}

auto functionLabel(const Function function) -> QString
{
    static const auto map = buildFunctionMap();
    const auto it = map.find(function);
    return it != map.end() ? it->second : "";
}

auto findByShortcut(const QChar key) -> const TokenEntry*
{
    for (const auto& entry : tokens())
    {
        if (entry.shortcutKey && *entry.shortcutKey == key) return &entry;
    }
    return nullptr;
}

}  // namespace Math::TokenRegistry
