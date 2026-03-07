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