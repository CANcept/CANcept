#pragma once
#include <QStringList>
#include <QStyledItemDelegate>
#include <functional>

namespace FaultInjector {

/**
 * @class FaultInjectorDynamicDelegate
 * @brief This class handles the rendering of the column effect and triggers inside a fault row
 */
class FaultInjectorDynamicDelegate final : public QStyledItemDelegate
{
    Q_OBJECT
   public:
    explicit FaultInjectorDynamicDelegate(std::function<QStringList(const QVariant&)> extractor,
                                          QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

   private:
    std::function<QStringList(const QVariant&)> m_extractor;

    void paintChip(QPainter* painter, const QRect& rect, const QString& label) const;
    void paintOverflow(QPainter* painter, const QRect& rect, int count) const;
};

}  // namespace FaultInjector