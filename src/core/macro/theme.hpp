#pragma once

#include <QApplication>
#include <QColor>
#include <QIcon>
#include <QMetaType>
#include <QVariant>

#include "console_logging.hpp"

namespace Core::Theme {

/**
 * @brief Internal Raw Accessor of Properties of the qApp. When the property isn't valid an
 * corresponding error is logged.
 */
inline QVariant getRawProperty(const char* propertyName)
{
    if (!qApp)
    {
        LOG_ERR("ThemeEngine", "Access attempt before QApplication initialized: {}", propertyName);
        return QVariant();
    }

    QVariant prop = qApp->property(propertyName);
    if (!prop.isValid())
    {
        LOG_ERR("ThemeEngine", "Property '{}' not found. Check theme.qss!", propertyName);
    }
    return prop;
}

/**
 * @brief Explicit type-safe getters.
 * These are preferred over raw templates to avoid C++20 constexpr evaluation issues.
 */

inline QColor getColor(const char* name)
{
    const QVariant variant = getRawProperty(name);

    if (variant.isValid() && variant.userType() == QMetaType::QColor)
    {
        return *static_cast<const QColor*>(variant.constData());
    }

    if (!variant.isValid())
    {
        LOG_ERR("ThemeEngine", "Color property '{}' missing. Using Magenta.", name);
    }

    return Qt::magenta;
}

inline int getInt(const char* name)
{
    return getRawProperty(name).toInt();
}

inline double getNum(const char* name)
{
    return getRawProperty(name).toDouble();
}

/**
 * @brief Safely extracts an icon from a variant.
 * Uses the non-templated userType check to satisfy strict linters.
 */
inline QIcon getIcon(const QVariant& data)
{
    if (data.isValid() && data.userType() == QMetaType::QIcon)
    {
        return *static_cast<const QIcon*>(data.constData());
    }
    return QIcon();
}

}  // namespace Core::Theme

/** @brief Gets a QColor. Missing properties return Magenta for visual debugging. */
#define THEME_COLOR(name) Core::Theme::getColor(name)

/** @brief Gets an integer (spacing, radius, font-weight). */
#define THEME_INT(name) Core::Theme::getInt(name)

/** @brief Gets a double (opacity, scales). */
#define THEME_NUM(name) Core::Theme::getNum(name)