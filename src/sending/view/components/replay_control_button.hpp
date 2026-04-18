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

#include <QPushButton>

namespace Sending {

/**
 * @class ReplayControlButton
 * @brief Themed button for replay actions (load/start/pause/resume/stop).
 */
class ReplayControlButton final : public QPushButton
{
    Q_OBJECT

   public:
    enum class Variant {
        Primary,
        Secondary,
        Danger
    };

    explicit ReplayControlButton(const QString& text, Variant variant,
                                 QWidget* parent = nullptr);
    ~ReplayControlButton() override = default;

    void setVariant(Variant variant);
    [[nodiscard]] auto variant() const -> Variant
    {
        return m_variant;
    }

   protected:
    auto event(QEvent* event) -> bool override;

   private:
    void applyStyle();

    Variant m_variant;
};

}  // namespace Sending

