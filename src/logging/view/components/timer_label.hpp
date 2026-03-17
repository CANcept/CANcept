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

#include <QLabel>
#include <cstdint>

namespace Logging {

/**
 * @class TimerLabel
 * @brief Styled label displaying elapsed time in HH:MM:SS:CS format.
 *
 * This component provides a consistent styled label for displaying
 * the elapsed time during recording sessions.
 */
class TimerLabel final : public QLabel
{
    Q_OBJECT

   public:
    explicit TimerLabel(QWidget* parent = nullptr);
    ~TimerLabel() override = default;

    /**
     * @brief Updates the timer display with elapsed time.
     * @param elapsedMs The number of milliseconds elapsed since logging started
     */
    void updateTimer(qint64 elapsedMs);

   private:
    void applyStyle();
};

}  // namespace Logging
