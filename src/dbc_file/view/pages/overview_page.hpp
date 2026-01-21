//
// Created by Adrian Rupp on 22.01.26.
//
#pragma once
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
 * **VISUALS:**
 * - Section 1: File Metadata (Version, Date).
 * - Section 2: KPI Cards (Counts for ECUs, Messages, Signals).
 * - Section 3: Horizontal flow lists for ECUs and Messages.
 *
 * **LOGIC:**
 * Uses QDataWidgetMapper for Sections 1 & 2, and QListViews for Section 3.
 */
class OverviewPage : public QWidget
{
    Q_OBJECT

   public:
    explicit OverviewPage(QWidget* parent = nullptr);
    ~OverviewPage() override = default;

    /**
     * @brief Returns the mapper used to populate the static labels (Top/Middle sections).
     * @caller DbcView::setSourceModel() to inject data.
     * @return Pointer to the internal mapper.
     */
    [[nodiscard]] auto getMapper() const -> QDataWidgetMapper*
    {
        return m_mapper;
    }

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

    QDataWidgetMapper* m_mapper;

    // Targets for m_mapper
    QLabel* m_lblVersion;
    QLabel* m_lblLoadedDate;
    QLabel* m_lblEcuCount;
    QLabel* m_lblMessageCount;
    QLabel* m_lblSignalCount;
    QLabel* m_lblLargestMessageName;

    // List Views
    QListView* m_ecuList;
    QListView* m_messageList;
};
}  // namespace DbcFile