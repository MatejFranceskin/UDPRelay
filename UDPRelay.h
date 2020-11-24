#pragma once

#include <atomic>
#include <string>
#include <thread>

class UDPRelay
{
public:
    UDPRelay(const std::string& destination_ip_address, uint16_t destination_udp_port, uint16_t local_udp_port = 0);

    bool start();

    void stop();

private:
    std::atomic<bool> should_exit_{false};
    std::thread client_thread_;
    std::string destination_ip_address_;
    uint16_t destination_udp_port_;
    uint16_t local_udp_port_;

    int create_udp_socket(uint16_t& port);

    void run_udp_proxy(int read_fd, const std::string& destination_ip_address, uint16_t destination_udp_port);
};

