//
// Created by Adrian Rupp on 28.01.26.
//
#pragma once
#include <QFrame>
namespace Core {
/**
 * @class StyledFrame
 * @brief A standard container widget with application styling.
 *
 * @details
 * Provides a uniform look for cards and containers:
 */
class StyledFrame : public QFrame {
    Q_OBJECT

public:
    explicit StyledFrame(QWidget* parent = nullptr);
    ~StyledFrame() override = default;

};
}
