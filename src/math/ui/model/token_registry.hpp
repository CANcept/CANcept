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

#include <QString>
#include <functional>
#include <list>
#include <memory>
#include <optional>

#include "math/types/tokens/internal/function_token.hpp"
#include "math/types/tokens/internal/operator_token.hpp"

namespace Math {

/**
 * @brief Single source of truth for token metadata: display, rendering, shortcuts, and factories.
 */
/**
 * @brief Single source of truth for token metadata: display, rendering, shortcuts, and factories.
 */
struct TokenEntry {
    QString label;                     ///< Button-bar display text.
    QString renderSymbol;              ///< Symbol drawn inside the expression tree.
    std::optional<QChar> shortcutKey;  ///< Keyboard shortcut (prefixed with backslash).
    std::function<std::unique_ptr<TokenBase>()> factory;  ///< Creates a fresh token instance.
};

/**
 * @brief Central registry of all insertable tokens.
 *
 * Provides a single list that drives the button bar, expression rendering,
 * and keyboard shortcuts. Lookup helpers avoid redundant switch/map code.
 */
namespace TokenRegistry {

/**
 * @brief Returns the master list of all insertable token entries.
 */
[[nodiscard]] auto tokens() -> const std::list<TokenEntry>&;

/**
 * @brief Returns the render symbol for the given arithmetic operation.
 */
[[nodiscard]] auto operatorSymbol(Operation op) -> QString;

/**
 * @brief Returns the display label for the given math function.
 */
[[nodiscard]] auto functionLabel(Function function) -> QString;

/**
 * @brief Finds the token entry matching the given keyboard shortcut character, or nullptr.
 */
[[nodiscard]] auto findByShortcut(QChar key) -> const TokenEntry*;

}  // namespace TokenRegistry

}  // namespace Math
