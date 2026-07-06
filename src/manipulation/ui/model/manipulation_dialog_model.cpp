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

#include "manipulation_dialog_model.hpp"

#include "manipulation_builder.hpp"

namespace Manipulation {

ManipulationDialogModel::ManipulationDialogModel(QObject* parent) : QObject(parent) {}

void ManipulationDialogModel::open(const bool isRaw)
{
    if (m_isRaw != isRaw) clearEntries();
    m_isRaw = isRaw;
    m_strategy = {};
    m_mutation = {};

    if (m_triggers.empty())
    {
        addTrigger();
    }
    if (m_effects.empty())
    {
        addEffect();
    }
}

void ManipulationDialogModel::clearEntries()
{
    for (int i = m_triggers.size() - 1; i >= 0; --i)
    {
        m_triggers.removeAt(i);
        emit triggerRemoved(i);
    }
    for (int i = m_effects.size() - 1; i >= 0; --i)
    {
        m_effects.removeAt(i);
        emit effectRemoved(i);
    }
}

void ManipulationDialogModel::addTrigger(SectionEntry entry)
{
    const int index = m_triggers.size();
    m_triggers.append(std::move(entry));
    emit triggerAdded(index);
}

void ManipulationDialogModel::removeTrigger(const int index)
{
    if (index < 0 || index >= m_triggers.size())
    {
        return;
    }
    m_triggers.removeAt(index);
    emit triggerRemoved(index);
}

void ManipulationDialogModel::setTrigger(const int index, SectionEntry entry)
{
    if (index < 0 || index >= m_triggers.size())
    {
        return;
    }
    m_triggers[index] = std::move(entry);
}

void ManipulationDialogModel::addEffect(SectionEntry entry)
{
    const int index = m_effects.size();
    m_effects.append(std::move(entry));
    emit effectAdded(index);
}

void ManipulationDialogModel::removeEffect(const int index)
{
    if (index < 0 || index >= m_effects.size())
    {
        return;
    }
    m_effects.removeAt(index);
    emit effectRemoved(index);
}

void ManipulationDialogModel::setEffect(const int index, SectionEntry entry)
{
    if (index < 0 || index >= m_effects.size())
    {
        return;
    }
    m_effects[index] = std::move(entry);
}

void ManipulationDialogModel::setStrategy(SectionEntry entry)
{
    m_strategy = std::move(entry);
}

void ManipulationDialogModel::setMutation(SectionEntry entry)
{
    m_mutation = std::move(entry);
}

auto ManipulationDialogModel::acquire() -> std::optional<ManipulationEntry>
{
    const Strategy strategy = buildStrategy(m_strategy);
    const Mutation mutation = buildMutation(m_mutation);

    if (m_isRaw)
    {
        RawManipulation manipulation;
        manipulation.strategy = strategy;
        manipulation.mutation = mutation;
        for (const auto& entry : m_triggers)
        {
            auto trigger = buildRawTrigger(entry);
            if (!trigger)
            {
                return std::nullopt;
            }
            manipulation.trigger.push_back(*trigger);
        }
        for (const auto& entry : m_effects)
        {
            auto effect = buildRawEffect(entry);
            if (!effect)
            {
                return std::nullopt;
            }
            manipulation.effect.push_back(*effect);
        }
        clearEntries();
        return ManipulationEntry(std::move(manipulation));
    }

    DbcManipulation manipulation;
    manipulation.strategy = strategy;
    manipulation.mutation = mutation;
    for (const auto& entry : m_triggers)
    {
        auto trigger = buildDbcTrigger(entry);
        if (!trigger)
        {
            return std::nullopt;
        }
        manipulation.trigger.push_back(*trigger);
    }
    for (const auto& entry : m_effects)
    {
        auto effect = buildDbcEffect(entry);
        if (!effect)
        {
            return std::nullopt;
        }
        manipulation.effect.push_back(*effect);
    }
    clearEntries();
    return ManipulationEntry(std::move(manipulation));
}

}  // namespace Manipulation