//
// Created by Adrian Rupp on 29.12.25.
//
#pragma once

#include <QStyledItemDelegate>

// Include Roles to access semantic data (Hex, Unit)
#include "../model/dbc_roles.hpp"

namespace DbcFile {

/**
 * @class DbcDelegate
 * @brief Standard delegate for formatting cell data in read-only QTableViews.
 *
 * USE CASE:
 * Used in "Messages Page" (Master Table) and "Signals Page" (Table).
 *
 * RESPONSIBILITIES:
 * Intercepts the data before painting to apply text formatting based on Custom Roles.
 */
class DbcDelegate : public QStyledItemDelegate
{
    Q_OBJECT

   public:
    explicit DbcDelegate(QObject* parent = nullptr);
    ~DbcDelegate() override = default;

    /**
     * @brief Initializes the style option structure with data from the model.
     *
     * @caller Qt View (QTableView) immediately before calling paint().
     *
     * @details
     * This method overrides the default text preparation logic:
     * 1. Checks `index.data(DbcRoles::Role_IsHex)`. If true, converts the integer value
     *    to a hex string (e.g., "0x1A").
     * 2. Checks `index.data(DbcRoles::Role_Unit)`. If a string exists, it appends it
     *    to the text (e.g., "50" -> "50 km/h").
     * 3. Writes the final string into `option.text`.
     *
     * @param option The style option to modify (contains text, alignment, font).
     * @param index The model index being processed.
     */
    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override;

    /**
     * @brief Converts raw QVariant data to a string representation.
     *
     * @caller Default implementation of QStyledItemDelegate inside initStyleOption.
     *
     * @details
     * Ensures that floating point numbers and other types are formatted according
     * to the provided locale settings.
     *
     * @param value The raw data from the model (DisplayRole).
     * @param locale The current system locale.
     * @return The string representation of the value.
     */
    [[nodiscard]] auto displayText(const QVariant& value,
                                   const QLocale& locale) const -> QString override;
};

}  // namespace DbcFile