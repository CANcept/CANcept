//
// Created by Adrian Rupp on 05.02.26.
//
#pragma once

#include <QListView>
#include <QWidget>

class QLineEdit;
class QComboBox;
class QFrame;
class QHBoxLayout;

namespace Core {
class TintedIconLabel;

class StyledFilterBar final : public QWidget
{
    Q_OBJECT

   public:
    explicit StyledFilterBar(QWidget* parent = nullptr);

    [[nodiscard]] auto searchText() const -> QString;
    [[nodiscard]] auto currentFilter() const -> QString;

    void setPlaceholderText(const QString& text) const;
    void setSearchText(const QString& text) const;
    void setFilterOptions(const QStringList& options) const;
    void setCurrentFilter(const QString& text) const;
    void setCurrentFilterIndex(int index) const;

   signals:
    void searchTextChanged(const QString& text);
    void filterIndexChanged(int index);

   private:
    void setupUi();
    void setupStyles() const;

    QLineEdit* m_searchBar = nullptr;
    QComboBox* m_filterBox = nullptr;
};

}  // namespace Core