
#include "fault_injector_model.hpp"

#include <ranges>

#include "entt/core/utility.hpp"
#include "fault_injector/service/fault_handler.hpp"
namespace FaultInjector {

FaultInjectorModel::FaultInjectorModel(QObject* parent) : QAbstractTableModel(parent) {}

void FaultInjectorModel::addFault(const Fault& fault)
{
    const int row = static_cast<int>(m_faults.size());
    beginInsertRows(QModelIndex(), row, row);
    m_faults.push_back(fault);
    endInsertRows();
}

FaultHandler FaultInjectorModel::get()
{
    std::vector<RawFault> rawFaults;
    std::vector<DbcFault> dbcFaults;

    std::ranges::for_each(m_faults, [&](const Fault& fault) {
        std::visit(entt::overloaded{
                       [&](const RawFault& f) { rawFaults.push_back(f); },
                       [&](const DbcFault& f) { dbcFaults.push_back(f); },
                   },
                   fault);
    });

    return FaultHandler(std::move(rawFaults), std::move(dbcFaults));
}

int FaultInjectorModel::rowCount(const QModelIndex& parent = QModelIndex()) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_faults.size());
}

int FaultInjectorModel::columnCount(const QModelIndex& parent = QModelIndex()) const
{
    return parent.isValid() ? 0 : static_cast<int>(FaultListColumn::Strategy) + 1;
}

QVariant FaultInjectorModel::headerData(int section, const Qt::Orientation orientation,
                                        const int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) return {};
    switch (static_cast<FaultListColumn>(section))
    {
        case FaultListColumn::Type:
            return "Type";
        case FaultListColumn::Triggers:
            return "Triggers";
        case FaultListColumn::Effects:
            return "Effects";
        case FaultListColumn::Strategy:
            return "Strategy";
    }
    return {};
}

QVariant FaultInjectorModel::data(const QModelIndex& index, const int role) const
{
    if (!index.isValid() || index.row() >= static_cast<int>(m_faults.size()))
    {
        return {};
    }

    const auto& fault = m_faults[index.row()];

    if (role == Qt::UserRole)
    {
        return std::visit(entt::overloaded{
                              [&](const RawFault& f) -> QVariant {
                                  switch (static_cast<FaultListColumn>(index.column()))
                                  {
                                      case FaultListColumn::Type:
                                          return QStringLiteral("RAW");
                                      case FaultListColumn::Triggers:
                                          return QVariant::fromValue(f.trigger);
                                      case FaultListColumn::Effects:
                                          return QVariant::fromValue(f.effect);
                                      case FaultListColumn::Strategy:
                                          return QVariant::fromValue(f.strategy);
                                  }
                                  return {};
                              },
                              [&](const DbcFault& f) -> QVariant {
                                  switch (static_cast<FaultListColumn>(index.column()))
                                  {
                                      case FaultListColumn::Type:
                                          return QStringLiteral("DBC");
                                      case FaultListColumn::Triggers:
                                          return QVariant::fromValue(f.trigger);
                                      case FaultListColumn::Effects:
                                          return QVariant::fromValue(f.effect);
                                      case FaultListColumn::Strategy:
                                          return QVariant::fromValue(f.strategy);
                                  }
                                  return {};
                              },
                          },
                          fault);
    }

    if (role == Qt::UserRole + 1)
    {
        return std::holds_alternative<RawFault>(fault) ? 0 : 1;
    }

    if (role == Qt::DisplayRole)
    {
        return std::visit(
            entt::overloaded{
                [&](const RawFault& f) -> QVariant {
                    switch (static_cast<FaultListColumn>(index.column()))
                    {
                        case FaultListColumn::Type:
                            return "RAW";
                        case FaultListColumn::Triggers:
                            return QStringLiteral("%1 triggers").arg(f.trigger.size());
                        case FaultListColumn::Effects:
                            return QStringLiteral("%1 effects").arg(f.effect.size());
                        case FaultListColumn::Strategy:
                            return QStringLiteral("Strategy");
                    }
                    return {};
                },
                [&](const DbcFault& f) -> QVariant {
                    switch (static_cast<FaultListColumn>(index.column()))
                    {
                        case FaultListColumn::Type:
                            return "DBC";
                        case FaultListColumn::Triggers:
                            return QStringLiteral("%1 triggers").arg(f.trigger.size());
                        case FaultListColumn::Effects:
                            return QStringLiteral("%1 effects").arg(f.effect.size());
                        case FaultListColumn::Strategy:
                            return QStringLiteral("Strategy");
                    }
                    return {};
                },
            },
            fault);
    }

    return {};
}

}  // namespace FaultInjector