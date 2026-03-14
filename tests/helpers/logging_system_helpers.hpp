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
