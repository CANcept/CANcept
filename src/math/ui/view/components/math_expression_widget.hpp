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

#include <QList>
#include <QPoint>
#include <QRect>
#include <QTimer>
#include <QWidget>

#include "math/types/tokens/token.hpp"
#include "math/ui/model/math_input_model.hpp"

namespace Math {

struct HitRegion {
    QRect rect;
    const TokenBase* token;
    const Token<TokenKind::Internal>* slotParent;
    int slotChildIndex = -1;
};

class MathExpressionWidget final : public QWidget
{
    Q_OBJECT

   public:
    /**
     * @brief Constructs the widget bound to the given model for rendering and interaction.
     */
    explicit MathExpressionWidget(MathInputModel* model, QWidget* parent = nullptr);

   protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    [[nodiscard]] auto sizeHint() const -> QSize override;

   private:
    void resetCursorBlink();

    [[nodiscard]] static auto isWhitelisted(QChar symbol) -> bool;

    MathInputModel* m_model;
    QList<HitRegion> m_hitRegions;
    QPoint m_mousePos = {-1, -1};
    QTimer m_cursorTimer;
    bool m_cursorVisible = true;
};

}  // namespace Math
