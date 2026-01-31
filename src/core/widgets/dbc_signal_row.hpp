#pragma once

#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QWidget>

namespace Core {
/**
 * @class DbcSignalRowWidget
 * @brief Represents a single signal line item within a message card.
 * @details Matches the mock: "Engine RPM | [Input] rpm | [Toggle] Use function".
 */
class DbcSignalRowWidget final : public QWidget
{
    Q_OBJECT
   public:
    /**
     * @brief Constructs the row UI.
     * @param name The signal name (e.g., "Engine RPM").
     * @param unit The physical unit (e.g., "rpm").
     * @param min The minimum physical value.
     * @param max The maximum physical value.
     */
    explicit DbcSignalRowWidget(const QString& name, const QString& unit, double min, double max,
                                QWidget* parent = nullptr);

    // Accessors for data binding
    [[nodiscard]] QLineEdit* valueEditor() const
    {
        return m_valueEditor;
    }
    [[nodiscard]] QCheckBox* functionToggle() const
    {
        return m_funcToggle;
    }

   private:
    void setupUi(const QString& name, const QString& unit, double min, double max);

    QLabel* m_nameLabel;       ///< e.g., "Engine RPM"
    QLabel* m_rangeLabel;      ///< e.g., "0-8000 rpm"
    QLineEdit* m_valueEditor;  ///< The main input field
    QLabel* m_unitLabel;       ///< e.g., "rpm"
    QCheckBox* m_funcToggle;   ///< The "Use value function" switch
};

}  // namespace Core