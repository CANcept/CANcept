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

#include <QCheckBox>

namespace Core {

/**
 * @class StyledCheckBox
 * @brief A customized QCheckBox widget with modern styling and custom painting.
 */
class StyledCheckBox final : public QCheckBox
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs a new Styled CheckBox.
     * @param parent The parent widget
     */
    explicit StyledCheckBox(QWidget* parent = nullptr);

    /**
     * @brief Constructs a new Styled CheckBox with text label.
     * @param text The label text to display next to the checkbox.
     * @param parent The parent widget
     */
    explicit StyledCheckBox(const QString& text, QWidget* parent = nullptr);

   protected:
    bool event(QEvent* event) override;

   private:
    /**
     * @brief Applies the application theme via Qt Stylesheets.
     */
    void applyStyle();

    bool m_hovered = false;
};

}  // namespace Core
