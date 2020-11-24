#include "UDPRelay.h"

#include <iostream>

#include <fcntl.h>
#include <memory.h>
#include <unistd.h> 
#include <arpa/inet.h>

UDPRelay::UDPRelay(const std::string& destination_ip_address, uint16_t destination_udp_port, uint16_t local_udp_port)
    : destination_ip_address_(destination_ip_address)
    , destination_udp_port_(destination_udp_port)
    , local_udp_port_(local_udp_port)
{
}

bool UDPRelay::start()
{
    if (!should_exit_)
    {
        stop();
    }
    should_exit_ = false;

    int fd = create_udp_socket(local_udp_port_);
    if (fd < 0) {
        return false;
    }
    run_udp_proxy(fd, destination_ip_address_, destination_udp_port_);

    return true;
}

void UDPRelay::stop()
{
    should_exit_ = true;
    if (client_thread_.joinable())
    {
        client_thread_.join();
    }
}

int UDPRelay::create_udp_socket(uint16_t& port)
{
    int fd;
    struct sockaddr_in servaddr;
      
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        std::cout << "socket creation failed" << std::endl;
        return -1;
    } 
      
    memset(&servaddr, 0, sizeof(servaddr));
      
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
    {
        std::cout << "Error setting socket fd as non-blocking" << std::endl;
        close(fd);
        return -1;
    }

    struct timeval read_timeout;
    read_timeout.tv_sec = 0;
    read_timeout.tv_usec = 10000;
    int recv_buf_size = 1048576;
    int send_buf_size = 1048576;
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout) ||
        setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &send_buf_size, sizeof recv_buf_size) ||
        setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &send_buf_size, sizeof send_buf_size))
    {
        std::cout << "setsockopt failed" << std::endl; 
        close(fd);
        return -1;
    }

    // Bind the socket with the server address 
    if (bind(fd, (const struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
    {
        std::cout << "bind failed" << std::endl;
        close(fd);
        return -1;
    }

    if (port == 0)
    {
        socklen_t len = sizeof(servaddr);
        if (getsockname(fd, (struct sockaddr *) &servaddr, &len) != 0)
        {
            std::cout << "getsockname failed" << std::endl;
            close(fd);
            return -1;
        }
        port = ntohs(servaddr.sin_port);
    }

    return fd;
}

void UDPRelay::run_udp_proxy(int fd, const std::string& destination_ip_address, uint16_t destination_udp_port)
{
    if (fd < 0)
    {
        return;
    }

    client_thread_ = std::thread([&, fd, destination_ip_address, destination_udp_port]()
    {
        char buffer[10240];
        struct sockaddr_in srcaddr;
        socklen_t srclen = sizeof(srcaddr);
        memset(&srcaddr, 0, sizeof(srcaddr));

        struct sockaddr_in destaddr;
        socklen_t destlen = sizeof(destaddr);
        memset(&destaddr, 0, sizeof(destaddr));
        destaddr.sin_family = AF_INET;
        destaddr.sin_port = htons(destination_udp_port);
        inet_pton(AF_INET, destination_ip_address.c_str(), &destaddr.sin_addr);

        while (!should_exit_)
        {
            int n = recvfrom(fd, (char *) buffer, sizeof(buffer), 0, (struct sockaddr *) &srcaddr, &srclen);
            if (n > 0)
            {
                if (sendto(fd, (const char *)buffer, n, 0, (const struct sockaddr *) &destaddr, destlen) < n)
                {
                    std::cout << "Error writing to socket" << std::endl;
                    should_exit_ = true;
                }
            }
        }

        close(fd);
    });
}
