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
#include <QWidget>

namespace Manipulation {

/**
 * @class IManipulationRowTypeProvider
 * @brief Interface for supplying a named type and its configuration widget to a
 * ManipulationRowWidget.
 */
class IManipulationRowTypeProvider
{
   public:
    virtual ~IManipulationRowTypeProvider() = default;

    /** @brief Name shown in the type combo box. */
    [[nodiscard]] virtual auto typeName() const -> QString = 0;

    /**
     * @brief Creates the configuration widget for this type.
     * @param parent Parent widget for the created widget.
     * @return A widget containing the options for this type, or nullptr if none.
     */
    [[nodiscard]] virtual auto createOptionsWidget(QWidget* parent) const -> QWidget* = 0;
};

}  // namespace Manipulation