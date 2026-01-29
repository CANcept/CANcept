#pragma once

#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <vector>

namespace Core {

class DbcSignalRowWidget;  // Forward declaration

/**
 * @class DbcMessageCard
 * @brief Reusable UI component for displaying a DBC message and its signals.
 * @note Used by both Sending (composition) and Logging (filtering/logging).
 */
class DbcMessageCard : public QWidget
{
    Q_OBJECT
   public:
    explicit DbcMessageCard(const QString& name, uint32_t id, int signalCount,
                            QWidget* parent = nullptr);
    ~DbcMessageCard() override = default;

    /** @brief Adds a specific signal row (slider for Sending, or graph-toggle for Logging). */
    void addSignalRow(QWidget* rowWidget);

    /** @brief Clears all signal rows. */
    void clearSignalRows();

    // --- Shared Accessors ---
    QCheckBox* headerCheckbox() const
    {
        return m_headerCheckbox;
    }
    void setHeaderChecked(bool checked);

    /** @brief Expands or collapses the signal list */
    void setExpanded(bool expanded);

   private:
    void setupUi(const QString& name, uint32_t id, int signalCount);

    QLabel* m_nameLabel;
    QLabel* m_idLabel;
    QCheckBox* m_headerCheckbox;  // Used as "Select to Send" or "Select to Log"
    QPushButton* m_expandBtn;     // Arrow icon

    QWidget* m_bodyContainer;
    QVBoxLayout* m_signalsLayout;
};

}  // namespace Core