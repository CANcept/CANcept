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
#include <linux/if.h>
#include <linux/if_link.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <stdexcept>
#include <string>
namespace TestHelpers {
class SocketCanDeviceManager
{
   public:
    explicit SocketCanDeviceManager(const std::string& ifname) : ifname_(ifname) {}

    void create()
    {
        system(("sudo ip link add dev " + ifname_ + " type vcan").c_str());
    }

    void up()
    {
        system(("sudo ip link set up " + ifname_).c_str());
    }

    void down()
    {
        system(("sudo ip link set down " + ifname_).c_str());
    }

    void remove()
    {
        system(("sudo ip link delete dev " + ifname_).c_str());
    }

   private:
    std::string ifname_;
};
}  // namespace TestHelpers