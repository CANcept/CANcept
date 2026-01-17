//
// Created by Adrian Rupp on 30.12.25.
//
//
// Created by Adrian Rupp on 30.12.25.
//
#pragma once

#include <Qt>  // Required for Qt::UserRole

namespace DbcFile {

/**
 * @brief Custom model roles for accessing specific DBC data attributes.
 *
 * These roles extend the standard Qt roles (like DisplayRole) to provide semantic access
 * to the underlying DBC data structure. They allow Delegates and Proxies to retrieve specific
 * attributes without relying on column indices, which is essential for Views that do not
 * have columns (e.g., QListView, QTreeView with custom painting).
 */
enum DbcRoles {
    // ==============================================================================
    // 1. Structure & Logic Roles
    // Used primarily by Proxy Models (TreeFilterProxy, FlatListProxy) for filtering.
    // ==============================================================================

    /**
     * @brief The semantic type of the node.
     * @return Core::DbcItemType (as int) - e.g., Root, Ecu, Message, or Signal.
     */
    Role_ItemType = Qt::UserRole + 1,

    // ==============================================================================
    // 2. Formatting Roles
    // Used by DbcDelegate in standard QTableViews to format cell text.
    // ==============================================================================

    /**
     * @brief Indicates if the value should be displayed in Hexadecimal format.
     * @return bool - True for Hex (e.g. 0x123), False for Decimal.
     */
    Role_IsHex = Qt::UserRole + 2,

    /**
     * @brief The unit suffix to append to the value.
     * @return QString - e.g., "km/h", "rpm", "%".
     */
    Role_Unit = Qt::UserRole + 3,

    // ==============================================================================
    // 3. Semantic Data Roles
    // Used by EcuTreeDelegate and SignalCardDelegate to paint complex items
    // independently of the column structure.
    // ==============================================================================

    // --- Message Specific Attributes ---

    /** @brief The CAN ID of the message. @return int */
    Role_Id = Qt::UserRole + 10,
    /** @brief The Data Length Code. @return int */
    Role_Dlc = Qt::UserRole + 11,
    /** @brief The name of the sending Node/ECU. @return QString */
    Role_Sender = Qt::UserRole + 12,

    //Applies to both messages and signals

    /** @brief The total number of signals in this message or messages of this ECU.*/
    Role_ChildCount = Qt::UserRole + 13,

    // --- Signal Specific Attributes ---

    /** @brief The start bit of the signal in the payload. @return int */
    Role_StartBit = Qt::UserRole + 20,
    /** @brief The length of the signal in bits. @return int */
    Role_BitLength = Qt::UserRole + 21,
    /** @brief The multiplication factor for physical value calculation. @return double */
    Role_Factor = Qt::UserRole + 22,
    /** @brief The offset for physical value calculation. @return double */
    Role_Offset = Qt::UserRole + 23,
    /** @brief The minimum valid physical value. @return double */
    Role_Min = Qt::UserRole + 24,
    /** @brief The maximum valid physical value. @return double */
    Role_Max = Qt::UserRole + 25,
    /** @brief The byte order (Endianness). @return QString ("Little Endian"/"Big Endian") */
    Role_ByteOrder = Qt::UserRole + 26,
    /** @brief The value type definition. @return QString ("Signed"/"Unsigned") */
    Role_ValueType = Qt::UserRole + 27,
    /** @brief The name of the receiving Node/ECU. @return QString */
    Role_Receivers = Qt::UserRole + 28
};

}  // namespace DbcFile