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

#include <QThread>
#include <atomic>
#include <memory>
#include <variant>

#include "core/dto/can_dto.hpp"
#include "core/interface/i_can_writer.hpp"
#include "core/util/spsc_ring.hpp"
#include "logging/constants.hpp"

namespace Logging {

/**
 * @brief Owns an ICanWriter and services it from a dedicated thread.
 *
 * Both raw and DBC messages are pushed through a single lock-free SPSC ring.
 */
class LoggingWorker final : public QThread
{
   public:
    explicit LoggingWorker(std::unique_ptr<Core::ICanWriter> writer, QObject* parent = nullptr);
    ~LoggingWorker() override;

    void post(Core::RawCanMessage msg);
    void post(Core::DbcCanMessage msg);

    /** @brief Drains remaining messages, closes the file, and joins. */
    void stop();

   protected:
    void run() override;

   private:
    using Message = std::variant<Core::RawCanMessage, Core::DbcCanMessage>;

    std::unique_ptr<Core::ICanWriter> m_writer;
    Core::SpscRing<Message, Constants::WORKER_RING_CAPACITY> m_ring;
    std::atomic<bool> m_stop{false};
};

}  // namespace Logging