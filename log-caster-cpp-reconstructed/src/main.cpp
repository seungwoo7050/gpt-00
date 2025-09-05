// [SEQUENCE: CPP-MVP1-14]
#include "LogServer.h"
#include "Persistence.h"
#include "IRCServer.h"
#include <iostream>
#include <getopt.h>
#include <thread>
#include <csignal>

// [SEQUENCE: CPP-MVP6-11]
static std::unique_ptr<LogServer> g_logServer;
static std::unique_ptr<IRCServer> g_ircServer;

void signal_handler(int signum) {
    std::cout << "\nInterrupt signal (" << signum << ") received. Shutting down...\n";
    if (g_ircServer) g_ircServer->stop();
    if (g_logServer) g_logServer->stop();
}

int main(int argc, char* argv[]) {
    int port = 9999;
    PersistenceConfig persist_config;
    // [SEQUENCE: CPP-MVP6-12]
    bool irc_enabled = false;
    int irc_port = 6667;

    // [SEQUENCE: CPP-MVP4-21]
    // 커맨드 라인 인자 파싱
    int opt;
    while ((opt = getopt(argc, argv, "p:d:s:iI:Ph")) != -1) {
        switch (opt) {
            case 'p': port = std::stoi(optarg); break;
            case 'P': persist_config.enabled = true; break;
            case 'd': persist_config.log_directory = optarg; break;
            case 's': persist_config.max_file_size = std::stoul(optarg) * 1024 * 1024; break;
            // [SEQUENCE: CPP-MVP6-13]
            case 'i': irc_enabled = true; break;
            case 'I': 
                irc_enabled = true;
                irc_port = std::stoi(optarg);
                break;
            case 'h':
                std::cout << "Usage: " << argv[0] << " [-p port] [-P] [-d dir] [-s size_mb] [-i] [-I irc_port] [-h]" << std::endl;
                return 0;
        }
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    try {
        g_logServer = std::make_unique<LogServer>(port);

        // [SEQUENCE: CPP-MVP4-22]
        // 영속성 관리자 생성 및 주입
        if (persist_config.enabled) {
            auto persistence = std::make_unique<PersistenceManager>(persist_config);
            g_logServer->setPersistenceManager(std::move(persistence));
            std::cout << "Persistence enabled. Dir: " << persist_config.log_directory 
                      << ", Max Size: " << persist_config.max_file_size / (1024*1024) << " MB" << std::endl;
        }

        // [SEQUENCE: CPP-MVP6-14]
        // IRC 서버 생성 및 별도 스레드에서 실행
        if (irc_enabled) {
            g_ircServer = std::make_unique<IRCServer>(irc_port, g_logServer->getLogBuffer());
            std::thread irc_thread([] { g_ircServer->start(); });
            irc_thread.detach();
            std::cout << "IRC Server enabled on port " << irc_port << std::endl;
        }

        g_logServer->start(); // This will block until the server is stopped

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
