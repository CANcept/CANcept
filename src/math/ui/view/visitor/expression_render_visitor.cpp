#include "math/ui/view/visitor/expression_render_visitor.hpp"

#include <QPainterPath>
#include <QPen>

#include "core/macro/theme.hpp"
#include "math/constants.hpp"
#include "math/ui/model/math_input_model.hpp"
#include "math/ui/model/token_registry.hpp"

namespace Math {
using namespace Constants;

ExpressionRenderVisitor::ExpressionRenderVisitor(QPainter& painter, const QFontMetrics& fontMetrics,
                                                 MathInputModel* model,
                                                 QList<HitRegion>& hitRegions,
                                                 const QPoint mousePos, const bool cursorVisible)
    : m_painter(painter),
      m_fm(fontMetrics),
      m_model(model),
      m_hitRegions(hitRegions),
      m_mousePos(mousePos),
      m_cursorVisible(cursorVisible),
      m_measurer(fontMetrics, model)
{
}

void ExpressionRenderVisitor::renderToken(const TokenBase* token, const QPointF origin)
{
    if (!token)
    {
        const QSizeF slotSize = m_measurer.measureSlot(nullptr, 0);
        paintSlot(QRectF(origin, slotSize), nullptr, 0);
        return;
    }
    m_origin = origin;
    token->accept(*this);
}

void ExpressionRenderVisitor::visit(const ValueToken& token)
{
    const QSizeF size = m_measurer.measureToken(&token);
    const QRectF rect(m_origin, size);
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    m_painter.setBrush(colors.surfacePrimary);
    m_painter.setPen(QPen(colors.borderStrong, spacing.borderThin));
    m_painter.drawRoundedRect(rect, spacing.radiusSm, spacing.radiusSm);
    m_painter.setPen(colors.textPrimary);
    m_painter.drawText(rect, Qt::AlignCenter, QString::number(token.value()));
    m_hitRegions.append({rect.toRect(), &token, nullptr, -1});
}

void ExpressionRenderVisitor::visit(const VariableToken& token)
{
    const QSizeF size = m_measurer.measureToken(&token);
    const QRectF rect(m_origin, size);
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();

    m_painter.setBrush(colors.surfaceSecondary);
    m_painter.setPen(QPen(colors.borderSubtle, spacing.borderThin));
    m_painter.drawRoundedRect(rect, spacing.radiusSm, spacing.radiusSm);
    QFont font = m_painter.font();
    font.setItalic(true);
    m_painter.setFont(font);
    m_painter.setPen(colors.textPrimary);
    m_painter.drawText(rect, Qt::AlignCenter, QString::fromStdString(token.toExpression()));
    font.setItalic(false);
    m_painter.setFont(font);
    m_hitRegions.append({rect.toRect(), &token, nullptr, -1});
}

void ExpressionRenderVisitor::visit(const OperatorToken& token)
{
    if (token.operation() == Operation::Div)
        renderFraction(token);
    else
        renderInlineOp(token);
}

void ExpressionRenderVisitor::visit(const FunctionToken& token)
{
    switch (token.function())
    {
        case Function::Sqrt:
            renderRadical(token);
            break;
        case Function::Abs:
            renderAbsBars(token);
            break;
        default:
            renderFunction(token);
            break;
    }
}

void ExpressionRenderVisitor::renderFraction(const OperatorToken& token)
{
    const auto& s = THEME.spacing();
    const int gap = static_cast<int>(s.spacingXs);

    const QPointF origin = m_origin;
    const QSizeF num = childSize(token, 0);
    const QSizeF den = childSize(token, 1);
    const QSizeF total = m_measurer.measureToken(&token);
    const double fracW = total.width();
    const double lineY = origin.y() + num.height() + gap;
    const qreal lineH = 2.0;

    renderChild(&token, 0, {origin.x() + (fracW - num.width()) / 2.0, origin.y()});

    m_painter.setPen(QPen(THEME.colors().textPrimary, lineH));
    m_painter.setBrush(Qt::NoBrush);
    m_painter.drawLine(QPointF(origin.x(), lineY + lineH / 2.0),
                       QPointF(origin.x() + fracW, lineY + lineH / 2.0));
    m_hitRegions.append({QRect(static_cast<int>(origin.x()), static_cast<int>(lineY),
                               static_cast<int>(fracW), static_cast<int>(lineH) + 2),
                         &token, nullptr, -1});

    const double denY = lineY + lineH + gap;
    renderChild(&token, 1, {origin.x() + (fracW - den.width()) / 2.0, denY});
}

void ExpressionRenderVisitor::renderInlineOp(const OperatorToken& token)
{
    const auto& spacing = THEME.spacing();
    const int padV = static_cast<int>(spacing.spacingXs * PAD_VERTICAL_FACTOR);

    const QPointF origin = m_origin;
    const QString sym = TokenRegistry::operatorSymbol(token.operation());
    const QSizeF lhs = childSize(token, 0);
    const QSizeF rhs = childSize(token, 1);
    const QSizeF total = m_measurer.measureToken(&token);
    const int opW = m_fm.horizontalAdvance(sym) + 2 * spacing.radiusXs;
    const int opH = m_fm.height() + 2 * padV;
    const double rowH = total.height();

    auto centerY = [&](double h) { return origin.y() + (rowH - h) / 2.0; };

    renderChild(&token, 0, {origin.x(), centerY(lhs.height())});

    const QRectF opRect(origin.x() + lhs.width() + spacing.spacingXs, centerY(opH), opW, opH);
    m_painter.setPen(THEME.colors().textPrimary);
    m_painter.setBrush(Qt::NoBrush);
    m_painter.drawText(opRect, Qt::AlignCenter, sym);
    m_hitRegions.append({opRect.toRect(), &token, nullptr, -1});

    renderChild(&token, 1,
                {origin.x() + lhs.width() + spacing.spacingXs + opW + spacing.spacingXs,
                 centerY(rhs.height())});
}

void ExpressionRenderVisitor::renderFunction(const FunctionToken& token)
{
    const auto& s = THEME.spacing();
    const int padV = static_cast<int>(s.spacingXs * PAD_VERTICAL_FACTOR);

    const QPointF origin = m_origin;
    const QString name = TokenRegistry::functionLabel(token.function());
    const QSizeF arg = childSize(token, 0);
    const QSizeF total = m_measurer.measureToken(&token);
    const int nameW = m_fm.horizontalAdvance(name);
    const int nameH = m_fm.height() + 2 * padV;
    const int lParenW = m_fm.horizontalAdvance("(");
    const int rParenW = m_fm.horizontalAdvance(")");
    const double rowH = total.height();

    auto centerY = [&](double h) { return origin.y() + (rowH - h) / 2.0; };

    double x = origin.x();

    const QRectF nameRect(x, centerY(nameH), nameW, nameH);
    m_painter.setPen(THEME.colors().textPrimary);
    m_painter.setBrush(Qt::NoBrush);
    m_painter.drawText(nameRect, Qt::AlignVCenter | Qt::AlignLeft, name);
    m_hitRegions.append({nameRect.toRect(), &token, nullptr, -1});
    x += nameW;

    m_painter.drawText(QRectF(x, centerY(nameH), lParenW, nameH), Qt::AlignVCenter | Qt::AlignLeft,
                       "(");
    x += lParenW;

    renderChild(&token, 0, {x, centerY(arg.height())});
    x += arg.width();

    m_painter.setPen(THEME.colors().textPrimary);
    m_painter.drawText(QRectF(x, centerY(nameH), rParenW, nameH), Qt::AlignVCenter | Qt::AlignLeft,
                       ")");
}

void ExpressionRenderVisitor::renderRadical(const FunctionToken& token)
{
    const auto& spacing = THEME.spacing();
    const QPointF origin = m_origin;
    const QSizeF total = m_measurer.measureToken(&token);
    constexpr double overlineGap = 2.0;
    constexpr double overlineH = 2.0;
    const double totalH = total.height();
    const double contentTop = origin.y() + overlineH + overlineGap;

    QPainterPath path;
    path.moveTo(origin.x(), origin.y() + totalH * 0.55);
    path.lineTo(origin.x() + spacing.spacingMd * 0.35, origin.y() + totalH - 1);
    path.lineTo(origin.x() + spacing.spacingMd - 1, origin.y());
    path.lineTo(origin.x() + total.width(), origin.y());

    m_painter.setPen(QPen(THEME.colors().textPrimary, 1.5));
    m_painter.setBrush(Qt::NoBrush);
    m_painter.drawPath(path);

    m_hitRegions.append({QRect(static_cast<int>(origin.x()), static_cast<int>(origin.y()),
                               spacing.spacingMd, static_cast<int>(totalH)),
                         &token, nullptr, -1});

    renderChild(&token, 0, {origin.x() + spacing.spacingMd, contentTop});
}

void ExpressionRenderVisitor::renderAbsBars(const FunctionToken& token)
{
    const auto& spacing = THEME.spacing();
    const int barPad = static_cast<int>(spacing.spacingXs * BAR_PAD_FACTOR);

    const QPointF origin = m_origin;
    const QSizeF arg = childSize(token, 0);
    const QSizeF total = m_measurer.measureToken(&token);
    const int barW = m_fm.horizontalAdvance("|");
    const double rowH = total.height();
    double x = origin.x();

    m_painter.setPen(QPen(THEME.colors().textPrimary, 1.5));
    m_painter.drawLine(QPointF(x + barW / 2.0, origin.y()),
                       QPointF(x + barW / 2.0, origin.y() + rowH));
    m_hitRegions.append({QRect(static_cast<int>(x), static_cast<int>(origin.y()), barW + barPad,
                               static_cast<int>(rowH)),
                         &token, nullptr, -1});
    x += barW + barPad;

    renderChild(&token, 0, {x, origin.y() + (rowH - arg.height()) / 2.0});
    x += arg.width() + barPad;

    m_painter.setPen(QPen(THEME.colors().textPrimary, 1.5));
    m_painter.drawLine(QPointF(x + barW / 2.0, origin.y()),
                       QPointF(x + barW / 2.0, origin.y() + rowH));
}

auto ExpressionRenderVisitor::childSize(const Token<TokenKind::Internal>& parent,
                                        const int index) -> QSizeF
{
    if (const auto& children = parent.children();
        static_cast<int>(children.size()) > index && children[index])
        return m_measurer.measureToken(children[index].get());
    return m_measurer.measureSlot(&parent, index);
}

void ExpressionRenderVisitor::renderChild(const Token<TokenKind::Internal>* parent, const int index,
                                          const QPointF pos)
{
    if (const auto& children = parent->children();
        static_cast<int>(children.size()) > index && children[index])
    {
        renderToken(children[index].get(), pos);
    } else
    {
        const QSizeF slotSize = m_measurer.measureSlot(parent, index);
        paintSlot(QRectF(pos, slotSize), parent, index);
    }
}

void ExpressionRenderVisitor::paintSlot(const QRectF& rect,
                                        const Token<TokenKind::Internal>* parent,
                                        const int childIndex) const
{
    if (const auto& slot = m_model->activeSlot();
        slot && slot->parent == parent && slot->childIndex == childIndex)
    {
        drawActiveSlot(rect);
    } else
    {
        drawEmptySlot(rect);
    }
    m_hitRegions.append({rect.toRect(), nullptr, parent, childIndex});
}

void ExpressionRenderVisitor::drawEmptySlot(const QRectF& rect) const
{
    const bool hovered = rect.contains(QPointF(m_mousePos));
    const auto& colors = THEME.colors();
    const auto& spacing = THEME.spacing();
    const QColor fill = hovered ? colors.surfaceSecondary.lighter(115) : colors.surfaceSecondary;
    m_painter.setBrush(fill);
    m_painter.setPen(Qt::NoPen);
    m_painter.drawRoundedRect(rect, spacing.radiusXs, spacing.radiusXs);
}

void ExpressionRenderVisitor::drawActiveSlot(const QRectF& rect) const
{
    const auto& spacing = THEME.spacing();
    const auto& colors = THEME.colors();
    const auto& buf = m_model->typeBuffer();

    m_painter.setBrush(colors.surfacePrimary);
    m_painter.setPen(Qt::NoPen);
    m_painter.drawRoundedRect(rect, spacing.radiusXs, spacing.radiusXs);

    if (!buf.isEmpty())
    {
        m_painter.setPen(colors.textPrimary);
        m_painter.drawText(rect.adjusted(spacing.radiusXs / 2.0, 0, -4, 0),
                           Qt::AlignVCenter | Qt::AlignLeft, buf);
    }

    if (m_cursorVisible)
    {
        const int textWidth = buf.isEmpty() ? 0 : m_fm.horizontalAdvance(buf);
        const qreal cursorX = rect.left() + spacing.radiusXs / 2.0 + textWidth + 1;
        m_painter.setPen(QPen(colors.colorPrimary, 1.5));
        m_painter.drawLine(QPointF(cursorX, rect.top() + 2), QPointF(cursorX, rect.bottom() - 2));
    }
}

}  // namespace Math
