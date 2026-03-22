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
    static constexpr int SPIN_INTERVAL_MS = 30;
    static constexpr int SPIN_STEP_DEG = 12;
    static constexpr int SPIN_SPAN_DEG = 270;
    static constexpr qreal STROKE_WIDTH = 2.0;
    static constexpr qreal MARGIN = 2.0;
    static constexpr qreal CHECK_RATIO_START_X = 0.15;
    static constexpr qreal CHECK_RATIO_MID_X = 0.4;
    static constexpr qreal CHECK_RATIO_END_X = 0.85;
    static constexpr qreal CHECK_RATIO_START_Y = 0.5;
    static constexpr qreal CHECK_RATIO_MID_Y = 0.75;
    static constexpr qreal CHECK_RATIO_END_Y = 0.25;

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
