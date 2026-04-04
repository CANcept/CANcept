#pragma once

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>

#include "core/dto/dbc_dto.hpp"
#include "math/service/variables/can_signal_variable.hpp"
#include "variable_type_provider.hpp"

namespace Math {

/**
 * @brief Provider for CAN signal variables with message/signal dropdown selection.
 */
class SignalVariableProvider final : public IVariableTypeProvider
{
   public:
    explicit SignalVariableProvider(const Core::DbcConfig* dbcConfig) : m_dbcConfig(dbcConfig) {}

    auto typeName() const -> QString override
    {
        return QStringLiteral("Signal");
    }

    auto createOptionsWidget(QWidget* parent) -> QWidget* override;

    auto configKey(const QWidget* optionsWidget) const -> std::string override;
    void restoreFromVariable(QWidget* optionsWidget, const IVariable* variable) const override;
    auto displayName(const QWidget* optionsWidget) const -> std::string override;
    auto createVariable(const QWidget* optionsWidget) const -> std::unique_ptr<IVariable> override;

   private:
    static auto findCombos(const QWidget* optionsWidget)
        -> std::pair<const QComboBox*, const QComboBox*>;

    const Core::DbcConfig* m_dbcConfig;
};

}  // namespace Math
