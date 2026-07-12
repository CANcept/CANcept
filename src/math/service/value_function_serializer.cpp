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

#include "value_function_serializer.hpp"

#include <stdexcept>
#include <utility>
#include <vector>

#include "core/dto/dbc_dto.hpp"
#include "core/service/serializer.hpp"
#include "math/service/variable_registry.hpp"
#include "math/service/variables/can_signal_variable.hpp"
#include "math/service/variables/time_variable.hpp"
#include "math/types/tokens/expression_visitor.hpp"
#include "math/types/tokens/internal/function_token.hpp"
#include "math/types/tokens/internal/operator_token.hpp"
#include "math/types/tokens/leaf/value_token.hpp"
#include "math/types/tokens/leaf/variable_token.hpp"

namespace Math {

namespace {

/**
 * @brief Encodes a TokenBase tree into JSON. VariableToken only knows its exprtk symbol (e.g.
 * "a"), not the underlying variable's identity
 */
class JsonEncodeVisitor final : public IExpressionVisitor
{
   public:
    nlohmann::json result;

    void visit(const ValueToken& token) override
    {
        result = nlohmann::json{{"kind", "value"}, {"value", token.value()}};
    }

    void visit(const VariableToken& token) override
    {
        result = nlohmann::json{{"kind", "variable"}, {"symbol", token.toExpression()}};
    }

    void visit(const OperatorToken& token) override
    {
        result = nlohmann::json{{"kind", "operator"},
                                {"operation", static_cast<int>(token.operation())},
                                {"children", encodeChildren(token)}};
    }

    void visit(const FunctionToken& token) override
    {
        result = nlohmann::json{{"kind", "function"},
                                {"function", static_cast<int>(token.function())},
                                {"children", encodeChildren(token)}};
    }

   private:
    template <typename InternalToken>
    static auto encodeChildren(const InternalToken& token) -> nlohmann::json
    {
        auto children = nlohmann::json::array();
        for (const auto& child : token.children())
        {
            JsonEncodeVisitor visitor;
            child->accept(visitor);
            children.push_back(std::move(visitor.result));
        }
        return children;
    }
};

auto decodeToken(const nlohmann::json& json, MathInputModel& model) -> std::unique_ptr<TokenBase>
{
    const auto kind = json.at("kind").get<std::string>();

    if (kind == "value")
    {
        return std::make_unique<ValueToken>(json.at("value").get<double>());
    }
    if (kind == "variable")
    {
        return model.makeVariableToken(json.at("symbol").get<std::string>());
    }
    if (kind == "operator")
    {
        auto token = std::make_unique<OperatorToken>(
            static_cast<Operation>(json.at("operation").get<int>()));
        for (const auto& child : json.at("children"))
        {
            token->addChild(decodeToken(child, model));
        }
        return token;
    }
    if (kind == "function")
    {
        auto token =
            std::make_unique<FunctionToken>(static_cast<Function>(json.at("function").get<int>()));
        for (const auto& child : json.at("children"))
        {
            token->addChild(decodeToken(child, model));
        }
        return token;
    }

    throw std::runtime_error("Unknown expression token kind: " + kind);
}

}  // namespace

auto makeVariableFromConfigKey(const std::string& configKey,
                               const Core::DbcConfig* dbcConfig) -> std::unique_ptr<IVariable>
{
    constexpr std::string_view timePrefix = "time:";
    constexpr std::string_view signalPrefix = "signal:";

    if (configKey.starts_with(timePrefix))
    {
        return std::make_unique<TimeVariable>(
            TimeVariable::unitFromSuffix(configKey.substr(timePrefix.size())));
    }

    if (configKey.starts_with(signalPrefix))
    {
        const auto rest = configKey.substr(signalPrefix.size());
        const auto separator = rest.find(':');
        if (separator == std::string::npos)
        {
            return nullptr;
        }

        const auto messageId = static_cast<uint32_t>(std::stoul(rest.substr(0, separator)));
        auto signalName = rest.substr(separator + 1);

        std::string messageName;
        if (dbcConfig)
        {
            for (const auto& message : dbcConfig->messageDefinitions)
            {
                if (message.messageId == messageId)
                {
                    messageName = message.messageName;
                    break;
                }
            }
        }

        return std::make_unique<CanSignalVariable>(messageId, std::move(signalName),
                                                   std::move(messageName));
    }

    return nullptr;
}

auto ValueFunctionSerializer::encode(const MathInputModel& model) -> nlohmann::json
{
    nlohmann::json tree;
    if (const auto* root = model.root())
    {
        JsonEncodeVisitor visitor;
        root->accept(visitor);
        tree = std::move(visitor.result);
    }

    auto bindings = nlohmann::json::array();
    for (const auto& binding : model.variableBindings())
    {
        bindings.push_back(nlohmann::json{
            {"symbol", std::string(1, binding.symbol)},
            {"typeIndex", binding.typeIndex},
            {"configKey", binding.variable ? binding.variable->configKey() : std::string()},
        });
    }

    return nlohmann::json{{"tree", tree}, {"bindings", bindings}};
}

auto ValueFunctionSerializer::decode(const nlohmann::json& json, MathInputModel& model,
                                     VariableRegistry& registry) -> std::unique_ptr<ValueFunction>
{
    std::vector<VariableBinding> bindings;
    for (const auto& entry : json.at("bindings"))
    {
        const auto configKey = entry.at("configKey").get<std::string>();
        const auto symbolStr = entry.at("symbol").get<std::string>();
        if (configKey.empty() || symbolStr.size() != 1)
        {
            continue;
        }

        const auto* dbcConfig = registry.dbcConfig();
        auto* variable = registry.acquire(configKey, [&configKey, dbcConfig] {
            return makeVariableFromConfigKey(configKey, dbcConfig);
        });

        bindings.push_back(VariableBinding{
            .symbol = symbolStr[0],
            .typeIndex = entry.at("typeIndex").get<int>(),
            .variable = variable,
        });
    }
    model.setVariableBindings(std::move(bindings));

    if (const auto& tree = json.at("tree"); !tree.is_null())
    {
        model.addNode(decodeToken(tree, model), nullptr);
    }

    if (!model.root() || !model.isComplete())
    {
        return nullptr;
    }

    auto valueFunction = std::make_unique<ValueFunction>(model.root());
    valueFunction->parse();
    return valueFunction;
}

auto ValueFunctionSerializer::save(const std::filesystem::path& path,
                                   const MathInputModel& model) -> bool
{
    Core::Serializer serializer;
    serializer.addItem(
        "valueFunction", [&model] { return encode(model); }, [](const nlohmann::json&) {});
    return serializer.saveToFile(path);
}

auto ValueFunctionSerializer::load(const std::filesystem::path& path, MathInputModel& model,
                                   VariableRegistry& registry) -> std::unique_ptr<ValueFunction>
{
    std::unique_ptr<ValueFunction> result;
    Core::Serializer serializer;
    serializer.addItem(
        "valueFunction", [&model] { return encode(model); },
        [&model, &registry, &result](const nlohmann::json& json) {
            result = decode(json, model, registry);
        });
    serializer.loadFromFile(path);
    return result;
}

}  // namespace Math
