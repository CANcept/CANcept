#pragma once

#include <QList>
#include <QPoint>
#include <QRect>
#include <QTimer>
#include <QWidget>

#include "math/types/tokens/token.hpp"

namespace Math {

class MathInputModel;

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
    static constexpr double MARGIN_FACTOR = 1.25;
    void resetCursorBlink();

    MathInputModel* m_model;
    QList<HitRegion> m_hitRegions;
    QPoint m_mousePos = {-1, -1};
    QTimer m_cursorTimer;
    bool m_cursorVisible = true;
};

}  // namespace Math
