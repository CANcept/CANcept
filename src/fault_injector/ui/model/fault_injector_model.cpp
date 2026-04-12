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

void FaultInjectorModel::removeFault(const int row)
{
    if (row < 0 || row >= static_cast<int>(m_faults.size()))
    {
        return;
    }
    beginRemoveRows(QModelIndex(), row, row);
    m_faults.erase(m_faults.begin() + row);
    endRemoveRows();
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
    return parent.isValid() ? 0 : static_cast<int>(FaultListColumn::Mutation) + 1;
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
        case FaultListColumn::Mutation:
            return "Mutation";
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
                                      case FaultListColumn::Mutation:
                                          return QVariant::fromValue(f.mutation);
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
                                      case FaultListColumn::Mutation:
                                          return QVariant::fromValue(f.mutation);
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
                        case FaultListColumn::Mutation:
                            return QStringLiteral("Mutation");
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
                        case FaultListColumn::Mutation:
                            return QStringLiteral("Mutation");
                    }
                    return {};
                },
            },
            fault);
    }

    return {};
}

}  // namespace FaultInjector