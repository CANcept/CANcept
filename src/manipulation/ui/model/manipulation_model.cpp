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

#include "manipulation_model.hpp"

#include <ranges>

#include "entt/core/utility.hpp"
#include "manipulation/service/manipulation_handler.hpp"
#include "manipulation/ui/delegate/manipulation_display.hpp"
namespace Manipulation {

ManipulationModel::ManipulationModel(QObject* parent) : QAbstractTableModel(parent) {}

void ManipulationModel::setMode(const Mode mode)
{
    if (m_mode == mode)
    {
        return;
    }
    m_mode = mode;

    if (mode == Mode::Raw)
    {
        beginResetModel();
        std::erase_if(m_manipulations, [](const ManipulationEntry& f) {
            return std::holds_alternative<DbcManipulation>(f);
        });
        endResetModel();
    }
}

void ManipulationModel::addManipulation(const ManipulationEntry& manipulation)
{
    const int row = static_cast<int>(m_manipulations.size());
    beginInsertRows(QModelIndex(), row, row);
    m_manipulations.push_back(manipulation);
    endInsertRows();
}

void ManipulationModel::removeManipulation(const int row)
{
    if (row < 0 || row >= static_cast<int>(m_manipulations.size()))
    {
        return;
    }
    beginRemoveRows(QModelIndex(), row, row);
    m_manipulations.erase(m_manipulations.begin() + row);
    endRemoveRows();
}

void ManipulationModel::setManipulations(std::vector<ManipulationEntry> manipulations)
{
    beginResetModel();
    m_manipulations = std::move(manipulations);
    endResetModel();
}

ManipulationHandler ManipulationModel::get()
{
    std::vector<RawManipulation> rawManipulations;
    std::vector<DbcManipulation> dbcManipulations;

    std::ranges::for_each(m_manipulations, [&](const ManipulationEntry& manipulation) {
        std::visit(entt::overloaded{
                       [&](const RawManipulation& f) { rawManipulations.push_back(f); },
                       [&](const DbcManipulation& f) { dbcManipulations.push_back(f); },
                   },
                   manipulation);
    });

    return ManipulationHandler(std::move(rawManipulations), std::move(dbcManipulations));
}

int ManipulationModel::rowCount(const QModelIndex& parent = QModelIndex()) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_manipulations.size());
}

int ManipulationModel::columnCount(const QModelIndex& parent = QModelIndex()) const
{
    return parent.isValid() ? 0 : static_cast<int>(ManipulationListColumn::Mutation) + 1;
}

QVariant ManipulationModel::headerData(int section, const Qt::Orientation orientation,
                                       const int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) return {};
    switch (static_cast<ManipulationListColumn>(section))
    {
        case ManipulationListColumn::Type:
            return "Type";
        case ManipulationListColumn::Triggers:
            return "Triggers";
        case ManipulationListColumn::Effects:
            return "Effects";
        case ManipulationListColumn::Strategy:
            return "Strategy";
        case ManipulationListColumn::Mutation:
            return "Mutation";
    }
    return {};
}

QVariant ManipulationModel::data(const QModelIndex& index, const int role) const
{
    if (!index.isValid() || index.row() >= static_cast<int>(m_manipulations.size()))
    {
        return {};
    }

    const auto& manipulation = m_manipulations[index.row()];

    if (role == Qt::UserRole)
    {
        return std::visit(entt::overloaded{
                              [&](const RawManipulation& f) -> QVariant {
                                  switch (static_cast<ManipulationListColumn>(index.column()))
                                  {
                                      case ManipulationListColumn::Type:
                                          return QStringLiteral("RAW");
                                      case ManipulationListColumn::Triggers:
                                          return QVariant::fromValue(f.trigger);
                                      case ManipulationListColumn::Effects:
                                          return QVariant::fromValue(strategyEffects(f.strategy));
                                      case ManipulationListColumn::Strategy:
                                          return QVariant::fromValue(f.strategy);
                                      case ManipulationListColumn::Mutation:
                                          return QVariant::fromValue(f.mutation);
                                  }
                                  return {};
                              },
                              [&](const DbcManipulation& f) -> QVariant {
                                  switch (static_cast<ManipulationListColumn>(index.column()))
                                  {
                                      case ManipulationListColumn::Type:
                                          return QStringLiteral("DBC");
                                      case ManipulationListColumn::Triggers:
                                          return QVariant::fromValue(f.trigger);
                                      case ManipulationListColumn::Effects:
                                          return QVariant::fromValue(strategyEffects(f.strategy));
                                      case ManipulationListColumn::Strategy:
                                          return QVariant::fromValue(f.strategy);
                                      case ManipulationListColumn::Mutation:
                                          return QVariant::fromValue(f.mutation);
                                  }
                                  return {};
                              },
                          },
                          manipulation);
    }

    if (role == Qt::UserRole + 1)
    {
        return std::holds_alternative<RawManipulation>(manipulation) ? 0 : 1;
    }

    if (role == Qt::DisplayRole)
    {
        return std::visit(
            entt::overloaded{
                [&](const RawManipulation& f) -> QVariant {
                    switch (static_cast<ManipulationListColumn>(index.column()))
                    {
                        case ManipulationListColumn::Type:
                            return "RAW";
                        case ManipulationListColumn::Triggers:
                            return QStringLiteral("%1 triggers").arg(f.trigger.size());
                        case ManipulationListColumn::Effects:
                            return QStringLiteral("%1 effects")
                                .arg(strategyEffects(f.strategy).size());
                        case ManipulationListColumn::Strategy:
                            return QStringLiteral("Strategy");
                        case ManipulationListColumn::Mutation:
                            return QStringLiteral("Mutation");
                    }
                    return {};
                },
                [&](const DbcManipulation& f) -> QVariant {
                    switch (static_cast<ManipulationListColumn>(index.column()))
                    {
                        case ManipulationListColumn::Type:
                            return "DBC";
                        case ManipulationListColumn::Triggers:
                            return QStringLiteral("%1 triggers").arg(f.trigger.size());
                        case ManipulationListColumn::Effects:
                            return QStringLiteral("%1 effects")
                                .arg(strategyEffects(f.strategy).size());
                        case ManipulationListColumn::Strategy:
                            return QStringLiteral("Strategy");
                        case ManipulationListColumn::Mutation:
                            return QStringLiteral("Mutation");
                    }
                    return {};
                },
            },
            manipulation);
    }

    return {};
}

}  // namespace Manipulation