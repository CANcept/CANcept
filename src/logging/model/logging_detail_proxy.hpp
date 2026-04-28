/** Copyright 2026 Lino Wertz, Florian Fehrle, Junes Sheikhi, Adrian Rupp and Nele Spatzier
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <QAbstractProxyModel>
#include <QStringList>
#include <memory>
#include <variant>
#include <vector>

#include "can_stream/reader/mdf4_reader.hpp"
#include "core/dto/can_dto.hpp"
#include "core/interface/i_can_reader.hpp"

namespace Logging {

/**
 * @brief Proxy model that reads paged MDF4 records for a single log session.
 *
 * Use loadPage() to navigate between pages; use hasPrevPage() / hasNextPage() to drive buttons.
 */
class LoggingDetailProxy final : public QAbstractProxyModel
{
    Q_OBJECT

   public:
    explicit LoggingDetailProxy(QObject* parent = nullptr);

    /** @brief Opens the .mf4 file for the given session row and resets to page 0. */
    void setSessionIndex(const QModelIndex& sessionIndex);

    [[nodiscard]] auto hasNextPage() const -> bool;
    [[nodiscard]] auto hasPrevPage() const -> bool;
    /** @brief 1-based current page number. */
    [[nodiscard]] auto currentPage() const -> int;
    /** @brief Total number of pages. */
    [[nodiscard]] auto pageCount() const -> int;
    /** @brief Total record count across all pages. */
    [[nodiscard]] auto totalRecordCount() const -> uint64_t;

    // QAbstractProxyModel overrides
    [[nodiscard]] auto mapToSource(const QModelIndex& proxyIndex) const -> QModelIndex override;
    [[nodiscard]] auto mapFromSource(const QModelIndex& sourceIndex) const -> QModelIndex override;
    [[nodiscard]] auto index(int row, int column, const QModelIndex& parent) const
        -> QModelIndex override;
    [[nodiscard]] auto parent(const QModelIndex& child) const -> QModelIndex override;
    [[nodiscard]] auto rowCount(const QModelIndex& parent) const -> int override;
    [[nodiscard]] auto columnCount(const QModelIndex& parent) const -> int override;
    [[nodiscard]] auto data(const QModelIndex& index, int role = Qt::DisplayRole) const
        -> QVariant override;
    [[nodiscard]] auto headerData(int section, Qt::Orientation orientation,
                                  int role = Qt::DisplayRole) const -> QVariant override;

   public slots:
    /** @brief Loads records starting at offset and triggers a model reset. */
    void loadPage(uint64_t offset);

   private:
    void buildHeaders();

    using PageBuffer =
        std::variant<std::vector<Core::RawCanMessage>, std::vector<Core::DbcCanMessage>>;

    std::unique_ptr<CanStream::Mdf4Reader> m_reader;
    QModelIndex m_sessionIndex;
    QStringList m_headers;
    Core::CanFileType m_fileType{Core::CanFileType::Raw};
    uint64_t m_pageOffset{0};
    PageBuffer m_buffer;
};

}  // namespace Logging