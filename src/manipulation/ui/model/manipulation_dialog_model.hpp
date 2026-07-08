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
#include <QObject>
#include <map>
#include <optional>
#include <string>

#include "core/dto/dbc_dto.hpp"
#include "manipulation/types/manipulation.hpp"
#include "section_entry.hpp"

namespace Manipulation {

/**
 * @class ManipulationDialogModel
 * @brief Single source of truth for the manipulation dialog state.
 */
class ManipulationDialogModel : public QObject
{
    Q_OBJECT

   public:
    explicit ManipulationDialogModel(QObject* parent);
    ~ManipulationDialogModel() override = default;

    /** @brief Resets state and marks the mode (raw or DBC). */
    void open(bool isRaw);

    /** @brief Returns whether the model is currently in raw mode. */
    [[nodiscard]] auto isRaw() const -> bool
    {
        return m_isRaw;
    }

    /**
     * @brief Appends a trigger entry and emits triggerAdded().
     * @param entry Initial entry; defaults to typeIndex 0 with no params.
     */
    void addTrigger(SectionEntry entry = {});

    /**
     * @brief Removes the trigger at the given index and emits triggerRemoved().
     * @param index No-op if out of range.
     */
    void removeTrigger(int index);

    /**
     * @brief Replaces the trigger entry at the given index.
     * @param index No-op if out of range.
     * @param entry the trigger to set
     */
    void setTrigger(int index, SectionEntry entry);

    /** @brief Returns the current list of trigger entries. */
    [[nodiscard]] auto triggerEntries() const -> const QList<SectionEntry>&
    {
        return m_triggers;
    }

    /**
     * @brief Appends an effect entry and emits effectAdded().
     * @param entry Initial entry
     */
    void addEffect(SectionEntry entry = {});

    /**
     * @brief Removes the effect at the given index and emits effectRemoved().
     * @param index the index
     */
    void removeEffect(int index);

    /**
     * @brief Replaces the effect entry at the given index.
     * @param index the index
     * @param entry the effect to set
     */
    void setEffect(int index, SectionEntry entry);

    /** @brief Returns the current list of effect entries. */
    [[nodiscard]] auto effectEntries() const -> const QList<SectionEntry>&
    {
        return m_effects;
    }

    /** @brief Replaces the strategy entry. */
    void setStrategy(SectionEntry entry);

    /** @brief Returns the current strategy entry. */
    [[nodiscard]] auto strategyEntry() const -> const SectionEntry&
    {
        return m_strategy;
    }

    /** @brief Replaces the mutation entry. */
    void setMutation(SectionEntry entry);

    /** @brief Returns the current mutation entry. */
    [[nodiscard]] auto mutationEntry() const -> const SectionEntry&
    {
        return m_mutation;
    }

    /**
     * @brief Sets the DBC message to insert for the insert strategy, resetting every
     * signal's value to its DBC-defined minimum.
     * @param messageId the message id
     * @param messageDef the message's DBC definition, used to seed default signal values
     */
    void setInsertMessage(uint32_t messageId, const Core::DbcMessageDescription& messageDef);

    /** @brief Replaces a single signal's value for the insert strategy's message. */
    void setInsertSignalValue(const std::string& signalName, double value);

    /**
     * @brief Marks the insert strategy to copy the frame that triggered it, instead of a
     * fixed, user-configured message. Clears any previously picked message/signal values.
     */
    void setInsertUseCurrentMessage();

    /**
     * @brief Converts the stored entries to a typed ManipulationEntry and resets the model.
     * @return The built ManipulationEntry, or std::nullopt if any entry is invalid.
     */
    auto acquire() -> std::optional<ManipulationEntry>;

   signals:
    /** @brief Emitted after a trigger entry is appended at the given index. */
    void triggerAdded(int index);

    /** @brief Emitted after a trigger entry is removed from the given index. */
    void triggerRemoved(int index);

    /** @brief Emitted after an effect entry is appended at the given index. */
    void effectAdded(int index);

    /** @brief Emitted after an effect entry is removed from the given index. */
    void effectRemoved(int index);

   private:
    void clearEntries();

    bool m_isRaw = true;
    QList<SectionEntry> m_triggers;
    QList<SectionEntry> m_effects;
    SectionEntry m_strategy;
    SectionEntry m_mutation;

    bool m_insertUseCurrentMessage = false;
    std::optional<uint32_t> m_insertMessageId;
    std::map<std::string, double> m_insertSignalValues;
};

}  // namespace Manipulation