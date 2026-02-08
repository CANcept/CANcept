#pragma once

#include <QCheckBox>
#include <QLabel>
#include <QWidget>

#include "card_widget.hpp"

namespace Core {

class StyledLineEdit;
class StyledCheckBox;  // Forward declaration

/**
 * @class DbcSignalRowWidget
 * @brief Generic signal row widget for DBC signals.
 *
 * Supports multiple display modes:
 * - Full mode (Sending): Signal name, value editor, unit, range, optional function toggle
 * - Selection mode (Logging/Monitoring): Signal name and optional selection checkbox
 *
 * The widget automatically adapts its layout based on configuration.
 */
class DbcSignalRowWidget final : public QWidget
{
    Q_OBJECT
   public:
    /**
     * @brief Display mode for the signal row.
     */
    enum class Mode { Full, Selection };

    /**
     * @brief Configuration for signal row appearance and behavior.
     */
    struct Config {
        Mode mode = Mode::Full;
        bool showRange = true;
        bool showSelectionCheckbox = false;
    };

    /**
     * @brief Constructs a signal row widget with custom configuration.
     * @param name Signal name (e.g., "EngineRPM")
     * @param unit Physical unit (e.g., "rpm")
     * @param min Minimum value
     * @param max Maximum value
     * @param config Configuration for signal row behavior
     * @param parent Parent widget
     */
    explicit DbcSignalRowWidget(const QString& name, const QString& unit, double min, double max,
                                const Config& config, QWidget* parent = nullptr);

    /**
     * @brief Constructs a signal row widget with default configuration.
     * @param name Signal name (e.g., "EngineRPM")
     * @param unit Physical unit (e.g., "rpm")
     * @param min Minimum value
     * @param max Maximum value
     * @param parent Parent widget
     */
    explicit DbcSignalRowWidget(const QString& name, const QString& unit, double min, double max,
                                QWidget* parent = nullptr);

    [[nodiscard]] auto valueEditor() const -> StyledLineEdit*
    {
        return m_valueEditor;
    }

    [[nodiscard]] auto functionToggle() const -> QCheckBox*
    {
        return m_funcToggle;
    }

    [[nodiscard]] auto selectionCheckbox() const -> StyledCheckBox*
    {
        return m_selectionCheckbox;
    }

   protected:
    bool event(QEvent* event) override;

   private:
    void setupUi(const QString& name, const QString& unit, double min, double max,
                 const Config& config);
    void setupFullMode(const QString& name, const QString& unit, double min, double max,
                       const Config& config);
    void setupSelectionMode(const QString& name, const QString& unit, const Config& config);
    void applyStyle() const;
    void clampInput() const;

    CardWidget* m_cardContainer;
    StyledCheckBox* m_selectionCheckbox;
    QLabel* m_nameLabel;
    QLabel* m_rangeLabel;
    StyledLineEdit* m_valueEditor;
    QLabel* m_unitLabel;
    QCheckBox* m_funcToggle;

    double m_minValue;
    double m_maxValue;
};

}  // namespace Core