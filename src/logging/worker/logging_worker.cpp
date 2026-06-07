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

#include "logging_worker.hpp"

#include <thread>

namespace Logging {

LoggingWorker::LoggingWorker(std::unique_ptr<Core::ICanWriter> writer, QObject* parent)
    : QThread(parent), m_writer(std::move(writer))
{
}

LoggingWorker::~LoggingWorker()
{
    if (isRunning()) stop();
}

void LoggingWorker::post(Core::RawCanMessage msg)
{
    m_ring.push(std::move(msg));
}

void LoggingWorker::post(Core::DbcCanMessage msg)
{
    m_ring.push(std::move(msg));
}

void LoggingWorker::stop()
{
    m_stop.store(true, std::memory_order_release);
    wait();
}

void LoggingWorker::run()
{
    using clock = std::chrono::steady_clock;
    auto nextFlush = clock::now() + Constants::WORKER_FLUSH_INTERVAL;

    while (!m_stop.load(std::memory_order_acquire))
    {
        Message msg;
        while (m_ring.pop(msg)) std::visit([this](auto& m) { m_writer->write(m); }, msg);

        if (clock::now() >= nextFlush)
        {
            m_writer->flush();
            nextFlush += Constants::WORKER_FLUSH_INTERVAL;
        }

        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

    // Final drain before closing
    Message msg;
    while (m_ring.pop(msg))
    {
        std::visit([this](auto& m) { m_writer->write(m); }, msg);
    }

    m_writer->close();
}

}  // namespace Logging