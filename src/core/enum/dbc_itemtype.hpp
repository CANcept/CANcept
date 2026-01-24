//
// Created by Adrian Rupp on 29.12.25.
//

#ifndef CANBUSMANAGER_DBCITEMTYPE_HPP
#define CANBUSMANAGER_DBCITEMTYPE_HPP

/**
 * @brief Holds the types a Dbc Item can have.
 */
namespace Core {
enum class DbcItemType { Root, Overview, OrphanHolder, Ecu, Message, Signal };
}
#endif  // CANBUSMANAGER_DBCITEMTYPE_HPP
