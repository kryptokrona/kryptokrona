// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <atomic>
#include <functional>
#include <map>
#include <string>
#include <thread>
#include <vector>

#include "blocking_queue.h"
#include "console_tools.h"

#ifndef _WIN32
#include <sys/select.h>
#endif

namespace common
{

    class AsyncConsoleReader
    {

    public:
        AsyncConsoleReader();
        ~AsyncConsoleReader();

        void start();
        bool getline(std::string &line);
        void stop();
        bool stopped() const;
        void pause();
        void unpause();

    private:
        void consoleThread();
        bool waitInput();

        std::atomic<bool> m_stop;
        std::thread m_thread;
        BlockingQueue<std::string> m_queue;
    };

    class ConsoleHandler
    {
    public:
        ~ConsoleHandler();

        typedef std::function<bool(const std::vector<std::string> &)> ConsoleCommandHandler;

        std::string getUsage() const;
        void setHandler(const std::string &command, const ConsoleCommandHandler &handler, const std::string &usage = "");
        void requestStop();
        bool runCommand(const std::vector<std::string> &cmdAndArgs);

        void start(bool startThread = true, const std::string &prompt = "", console::Color promptColor = console::Color::Default);
        void stop();
        void wait();
        void pause();
        void unpause();

    private:
        typedef std::map<std::string, std::pair<ConsoleCommandHandler, std::string>> CommandHandlersMap;

        virtual void handleCommand(const std::string &cmd);

        void handlerThread();

        std::thread m_thread;
        std::string m_prompt;
        console::Color m_promptColor = console::Color::Default;
        CommandHandlersMap m_handlers;
        AsyncConsoleReader m_consoleReader;
    };
}
