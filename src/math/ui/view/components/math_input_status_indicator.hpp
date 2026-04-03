#pragma once

#include <QTimer>
#include <QWidget>

#include "math/service/value_function.hpp"

namespace Math {

/**
 * @brief Animated status indicator: spins while parsing, shows check on success, X on error.
 */
class MathInputStatusIndicator final : public QWidget
{
    Q_OBJECT

   public:
    enum class State { Idle, Parsing, Success, Error };

    /**
     * @brief Constructs the indicator in idle state with an optional parent.
     */
    explicit MathInputStatusIndicator(QWidget* parent = nullptr);

    /**
     * @brief Returns the preferred square size for the indicator icon.
     */
    [[nodiscard]] auto sizeHint() const -> QSize override;

    /**
     * @brief Transitions to the spinning animation state.
     */
    void startParsing();

    /**
     * @brief Shows success or error state based on the parse result.
     */
    void setParseResult(const ValueFunction::ParseResult& result);

   protected:
    void paintEvent(QPaintEvent* event) override;
    auto event(QEvent* event) -> bool override;

   private:
    void applyStyle();
    void drawSpinner(QPainter& painter) const;
    void drawCheck(QPainter& painter) const;
    void drawCross(QPainter& painter) const;

    State m_state = State::Idle;
    int m_spinAngle = 0;
    QTimer* m_spinTimer;
    QColor m_successColor;
    QColor m_errorColor;
    QColor m_spinColor;
    QString m_errorText;
};

}  // namespace Math
