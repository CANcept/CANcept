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

#include "replay_log_parser.hpp"

#include <QFile>
#include <QTextStream>
#include <QTime>

#include <limits>

namespace Logging {

auto ReplayLogParser::parseDurationToMs(const QString& duration) -> std::chrono::milliseconds
{
    const QTime parsed = QTime::fromString(duration, "hh:mm:ss");
    if (!parsed.isValid())
    {
        return std::chrono::milliseconds(0);
    }

    return std::chrono::milliseconds(QTime(0, 0).msecsTo(parsed));
}

auto ReplayLogParser::countLogEntries(const QString& logFilePath) -> size_t
{
    QFile file(logFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return 0;
    }

    QTextStream stream(&file);
    bool headerSkipped = false;
    size_t entries = 0;

    while (!stream.atEnd())
    {
        const QString line = stream.readLine().trimmed();
        if (line.isEmpty())
        {
            continue;
        }

        if (!headerSkipped)
        {
            headerSkipped = true;
            continue;
        }

        ++entries;
    }

    return entries;
}

auto ReplayLogParser::parseRawReplayFrames(const QString& logFilePath,
                                           QList<Core::ReplayFrame>& frames,
                                           size_t& skippedFrameCount,
                                           QString& errorMessage) -> bool
{
    QFile file(logFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        errorMessage = "Failed to open log file";
        return false;
    }

    QTextStream stream(&file);
    bool headerSkipped = false;

    while (!stream.atEnd())
    {
        const QString rawLine = stream.readLine().trimmed();
        if (rawLine.isEmpty())
        {
            continue;
        }

        if (!headerSkipped)
        {
            headerSkipped = true;
            continue;
        }

        const QStringList parts = rawLine.split(',', Qt::KeepEmptyParts);
        if (parts.size() < 3)
        {
            ++skippedFrameCount;
            continue;
        }

        bool timestampOk = false;
        const qint64 timestampMs = parts.at(0).trimmed().toLongLong(&timestampOk);
        if (!timestampOk || timestampMs < 0)
        {
            ++skippedFrameCount;
            continue;
        }

        QString messageIdPart = parts.at(1).trimmed();
        if (messageIdPart.startsWith("0x", Qt::CaseInsensitive))
        {
            messageIdPart = messageIdPart.mid(2);
        }

        bool messageIdOk = false;
        const uint messageId = messageIdPart.toUInt(&messageIdOk, 16);
        if (!messageIdOk || messageId > std::numeric_limits<uint16_t>::max())
        {
            ++skippedFrameCount;
            continue;
        }

        const QString payloadText = parts.mid(2).join(",").trimmed();
        const QStringList dataTokens = payloadText.split(' ', Qt::SkipEmptyParts);
        if (dataTokens.isEmpty() || dataTokens.size() > 8)
        {
            ++skippedFrameCount;
            continue;
        }

        Core::ReplayFrame frame;
        frame.receiveTime = std::chrono::milliseconds(timestampMs);
        frame.messageId = static_cast<uint16_t>(messageId);
        frame.dlc = static_cast<uint8_t>(dataTokens.size());

        bool validData = true;
        for (int i = 0; i < dataTokens.size(); ++i)
        {
            bool byteOk = false;
            const uint byte = dataTokens.at(i).toUInt(&byteOk, 16);
            if (!byteOk || byte > std::numeric_limits<uint8_t>::max())
            {
                validData = false;
                break;
            }
            frame.data[static_cast<size_t>(i)] = static_cast<uint8_t>(byte);
        }

        if (!validData)
        {
            ++skippedFrameCount;
            continue;
        }

        frames.append(frame);
    }

    return true;
}

auto ReplayLogParser::parseDbcReplayMessages(
    const QString& logFilePath,
    const std::map<uint32_t, QStringList>& selectedSignalsPerMessage,
    const std::map<uint16_t, std::pair<int, int>>& signalsBeforeAfterMessage,
    QList<Core::DbcCanMessage>& messages,
    size_t& skippedFrameCount,
    QString& errorMessage) -> bool
{
    QFile file(logFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        errorMessage = "Failed to open log file";
        return false;
    }

    QTextStream stream(&file);
    bool headerSkipped = false;

    while (!stream.atEnd())
    {
        const QString rawLine = stream.readLine().trimmed();
        if (rawLine.isEmpty())
        {
            continue;
        }

        if (!headerSkipped)
        {
            headerSkipped = true;
            continue;
        }

        const QStringList parts = rawLine.split(',', Qt::KeepEmptyParts);
        if (parts.size() < 2)
        {
            ++skippedFrameCount;
            continue;
        }

        bool timestampOk = false;
        const qint64 timestampMs = parts.at(0).trimmed().toLongLong(&timestampOk);
        if (!timestampOk || timestampMs < 0)
        {
            ++skippedFrameCount;
            continue;
        }

        bool lineInvalid = false;
        bool foundMessage = false;
        Core::DbcCanMessage parsedMessage;

        for (const auto& [messageId32, signalNames] : selectedSignalsPerMessage)
        {
            if (signalNames.isEmpty())
            {
                continue;
            }

            const auto offsetsIt =
                signalsBeforeAfterMessage.find(static_cast<uint16_t>(messageId32));
            if (offsetsIt == signalsBeforeAfterMessage.end())
            {
                continue;
            }

            const int firstSignalCol = 1 + offsetsIt->second.first;
            const int signalCount = static_cast<int>(signalNames.size());
            if (firstSignalCol < 1 || firstSignalCol + signalCount > parts.size())
            {
                lineInvalid = true;
                break;
            }

            Core::DbcCanMessage candidate;
            candidate.receiveTime = std::chrono::milliseconds(timestampMs);
            candidate.messageId = static_cast<uint16_t>(messageId32);

            bool hasAnySignalValue = false;
            for (int i = 0; i < signalCount; ++i)
            {
                const QString token = parts.at(firstSignalCol + i).trimmed();
                if (token.isEmpty())
                {
                    continue;
                }

                bool valueOk = false;
                const double value = token.toDouble(&valueOk);
                if (!valueOk)
                {
                    lineInvalid = true;
                    break;
                }

                Core::DbcCanSignal signal;
                signal.name = signalNames.at(i).toStdString();
                signal.value = value;
                candidate.signalValues.push_back(std::move(signal));
                hasAnySignalValue = true;
            }

            if (lineInvalid)
            {
                break;
            }

            if (hasAnySignalValue)
            {
                if (foundMessage)
                {
                    lineInvalid = true;
                    break;
                }

                parsedMessage = std::move(candidate);
                foundMessage = true;
            }
        }

        if (lineInvalid || !foundMessage)
        {
            ++skippedFrameCount;
            continue;
        }

        messages.append(std::move(parsedMessage));
    }

    return true;
}

}  // namespace Logging

