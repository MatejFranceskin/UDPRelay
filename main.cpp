#include "UDPRelay.h"

#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <sys/signalfd.h>

static void wait_for_termination_signal()
{
    sigset_t sigs;
    sigemptyset(&sigs);
    sigaddset(&sigs, SIGINT);
    sigaddset(&sigs, SIGTERM);
    int sigfd = ::signalfd(-1, &sigs, SFD_CLOEXEC);
    struct signalfd_siginfo si;
    while (::read(sigfd, &si, sizeof(si)) == 0) {}
}

int main(int argc, char *argv[])
{
    if (argc == 4) {

    } else {
        std::cout << "Usage: UDPRelay \"source port\" \"destination ip\" \"destination port\"" << std::endl;
        return -1;
    }

    uint16_t source_port = std::stoi(argv[1]);
    std::string dest_ip = argv[2];
    uint16_t dest_port = std::stoi(argv[3]);

    UDPRelay relay(dest_ip, dest_port, source_port);

    if (relay.start()) {
        wait_for_termination_signal();
        relay.stop();
    } else {
        std::cout << "Failed to start UDP relay" << std::endl;
        return -1;
    }

    return 0;
}