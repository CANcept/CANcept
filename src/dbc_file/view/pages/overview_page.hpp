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
 * @brief Overview page for a loaded DBC file.
 *
 * OverviewPage displays general information about the currently
 * loaded DBC file, including:
 * - File name and version
 * - Statistics (ECU, message, signal, and orphan counts)
 * - Overview lists for ECUs and messages
 *
 * The page is scrollable and uses card-based widgets to visually
 * group related information.
 */
class OverviewPage : public QWidget
{
    Q_OBJECT
   public:
    /**
     * @brief Constructs a new OverviewPage.
     *
     * Initializes the complete user interface and all UI sections.
     *
     * @param parent Optional parent widget.
     */
    explicit OverviewPage(QWidget* parent = nullptr);
    ~OverviewPage() override = default;

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

    /**
     * @brief Updates all static labels on the page using data from the model.
     *
     * @caller DbcView::setSourceModel() and connected lambda on modelReset.
     *
     * @details
     * Queries the provided model for the "Overview Item" (Row 0) and extracts
     * metadata (Filename, Version) and statistics (Counts) using the defined
     * column constants. It populates the File Info card and the KPI cards directly.
     *
     * @param model The source data model containing the parsed DBC structure.
     */
    void updateLabels(const QAbstractItemModel* model) const;

   private:
    /**
     * @brief Creates the file information section.
     *
     * Displays file name and version inside a card widget.
     *
     * @param parentLayout Parent layout to which the section is added.
     */
    void setupFileInfoSection(QVBoxLayout* parentLayout);

    /**
     * @brief Creates a statistic card widget.
     *
     * The card contains a title, an icon, and a value label.
     *
     * @param title Title of the statistic.
     * @param valueLabelPtr Reference to a pointer that receives the value QLabel.
     * @param iconPath Path to the icon displayed in the card.
     *
     * @return Pointer to the created statistic card widget.
     */
    static auto createStatCard(const QString& title, QLabel*& valueLabelPtr,
                               const QString& iconPath) -> QWidget*;

    /**
     * @brief Creates the statistics section.
     *
     * Displays statistic cards for ECUs, messages, signals,
     * and orphan messages.
     *
     * @param parentLayout Parent layout to which the section is added.
     */
    void setupStatsSection(QVBoxLayout* parentLayout);

    /**
     * @brief Creates an overview list card with a QListView.
     *
     * The function creates a card widget containing a configured
     * QListView and adds it to the given parent layout.
     *
     * @param parentLayout Layout to which the card is added.
     * @param title Title of the list/card.
     * @param listViewMember Reference to the member pointer that receives the created QListView.
     * @param badgeIconPath Icon used for the item badge.
     */
    static void createOverviewList(QHBoxLayout* parentLayout, const QString& title,
                                   QListView*& listViewMember, const QString& badgeIconPath);

    /**
     * @brief Creates the list overview section.
     *
     * Adds card-based overview lists for ECUs and messages.
     *
     * @param parentLayout Parent layout to which the section is added.
     */
    void setupListsSection(QVBoxLayout* parentLayout);

    /**
     * @brief Sets up the complete user interface layout.
     *
     * Creates the scroll area and initializes all UI sections
     * (file info, statistics, and overview lists).
     */
    void setupUi();

    // --- 1. Labels for File Info Card ---
    /** @brief Label displaying the DBC file name. */
    QLabel* m_lblFileName;
    /** @brief Label displaying the DBC version. */
    QLabel* m_lblVersion;

    // --- 2. Labels for Stat Cards
    /** @brief Label displaying the ECU count. */
    QLabel* m_lblEcuCount;
    /** @brief Label displaying the message count. */
    QLabel* m_lblMessageCount;
    /** @brief Label displaying the signal count. */
    QLabel* m_lblSignalCount;
    /** @brief Label displaying the orphan messages count. */
    QLabel* m_lblOrphanCount;

    // List Views
    /** @brief List view displaying an overview of ECUs. */
    QListView* m_ecuList;
    /** @brief List view displaying an overview of messages. */
    QListView* m_messageList;
};
}  // namespace DbcFile