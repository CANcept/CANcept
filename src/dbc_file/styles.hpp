#pragma once

#include <QString>

class QWidget;

namespace DbcFile::Style
{
namespace LoadPage{
QString uploadZone();
QString uploadInstruction();
QString statusLabel(bool isError);
}
namespace OverviewPage
{
QString scrollArea();
QString secondaryLabel();
QString statTitle();
QString statValue();
}
namespace EcusPage
{
QString treeStyle();
QString emptyLabel();
}
namespace MessagesPage
{
QString detailList();
QString detailLabel();
}
}