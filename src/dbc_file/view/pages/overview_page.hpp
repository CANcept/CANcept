//
// Created by Adrian Rupp on 22.01.26.
//
#pragma once
#include <qboxlayout.h>

#include <QDataWidgetMapper>
#include <QLabel>
#include <QListView>
#include <QWidget>
namespace DbcFile {
/**
 * @class OverviewPage
 * @brief The Overview page displaying file statistics and summaries (SRS 6.2).
 *
 * @details
 * VISUALS:
 * - Section 1: File Metadata (Version, Date).
 * - Section 2: KPI Cards (Counts for ECUs, Messages, Signals).
 * - Section 3: Horizontal flow lists for ECUs and Messages.
 *
 * LOGIC:
 * Uses QDataWidgetMapper for Sections 1 & 2, and QListViews for Section 3.
 */
class OverviewPage : public QWidget
{
    Q_OBJECT

   public:
    explicit OverviewPage(QWidget* parent = nullptr);
    ~OverviewPage() override = default;

    // --- File Info ---
    void setFileName(const QString& text) const;
    void setVersion(const QString& text) const;

    // --- Stats ---
    void setEcuCount(const QString& text) const;
    void setMessageCount(const QString& text) const;
    void setSignalCount(const QString& text) const;
    void setOrphanCount(const QString& text) const;
    void setupFileInfoSection(QVBoxLayout* parentLayout);
    void setupStatsSection(QVBoxLayout* parentLayout);
    void setupListsSection(QVBoxLayout* parentLayout);

    /**
     * @brief Returns the list view for the ECUs section.
     * @caller DbcView::setSourceModel() to set the ECU Proxy.
     * @return Pointer to the ECU list view.
     */
    [[nodiscard]] auto getEcuList() const -> QListView*
    {
        return m_ecuList;
    }

    /**
     * @brief Returns the list view for the Messages section.
     * @caller DbcView::setSourceModel() to set the Message Proxy.
     * @return Pointer to the Message list view.
     */
    [[nodiscard]] auto getMessageList() const -> QListView*
    {
        return m_messageList;
    }

   private:
    /**
     * @brief Assembles the dashboard layout.
     * @caller Constructor.
     * @details Creates the ScrollArea, File Info Group, Stat Cards Layout, and the Overview Lists.
     */
    void setupUi();

    /**
     * @brief Helper to create one of the 4 Statistics cards in the middle section.
     * @caller Internal setupUi().
     */
    auto createStatCard(const QString& title, QLabel*& valueLabelPtr,
                        const QString& iconName) -> QWidget*;

    // --- 1. Labels for File Info Card ---
    QLabel* m_lblFileName;
    QLabel* m_lblVersion;

    // --- 2. Labels for KPI Cards
    QLabel* m_lblEcuCount;
    QLabel* m_lblMessageCount;
    QLabel* m_lblSignalCount;
    QLabel* m_lblOrphanCount;

    // List Views
    QListView* m_ecuList;
    QListView* m_messageList;
};
}  // namespace DbcFile