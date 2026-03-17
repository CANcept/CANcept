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

#include <QAbstractButton>
#include <QApplication>
#include <QDateTime>
#include <QDialog>
#include <QDir>
#include <QString>
#include <QTimer>

namespace TestHelpers {

// Schedules acceptance of the logging dialog in RAW mode (default switch state)
inline void acceptDialogAsRaw()
{
    QTimer::singleShot(300, []() {
        if (auto* d = qobject_cast<QDialog*>(QApplication::activeModalWidget())) d->accept();
    });
}

// Schedules dialog interaction: switch to DBC mode, check first message card, accept
inline void acceptDialogAsDbc()
{
    QTimer::singleShot(300, []() {
        auto* d = qobject_cast<QDialog*>(QApplication::activeModalWidget());
        if (!d) return;
        if (auto* sw = d->findChild<QAbstractButton*>("logTypeSwitch")) sw->setChecked(true);
        if (auto* cb = d->findChild<QAbstractButton*>("messageHeaderCheckbox")) cb->click();
        d->accept();
    });
}

// Returns the path of the newest CanLogging log file created at or after timestampMs
inline QString findLogFileCreatedAfter(const qint64 timestampMs)
{
    for (const QDir logsDir("logs"); const auto& fi : logsDir.entryInfoList(
                                         {"session_*_CanLogging.log"}, QDir::Files, QDir::Time))
    {
        // filename: session_{sessionId}_CanLogging.log, sessionId = ms timestamp
        if (const QStringList parts = fi.completeBaseName().split('_'); parts.size() >= 2)
        {
            bool ok;
            if (const qint64 ts = parts[1].toLongLong(&ok); ok && ts >= timestampMs)
                return fi.absoluteFilePath();
        }
    }
    return {};
}

}  // namespace TestHelpers
