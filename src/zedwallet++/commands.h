// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

#pragma once

#include <string>
#include <vector>

class Command
{
    public:
        Command(std::string commandName, std::string description) :
                commandName(commandName), description(description) {}

        const std::string commandName;
        const std::string description;
};

class AdvancedCommand : public Command
{
    public:
        AdvancedCommand(std::string commandName, std::string description,
                        bool viewWalletSupport, bool advanced) :
                        Command(commandName, description),
                        viewWalletSupport(viewWalletSupport),
                        advanced(advanced) {}

        const bool viewWalletSupport;
        const bool advanced;
};

std::vector<Command> startupCommands();

std::vector<Command> nodeDownCommands();

std::vector<AdvancedCommand> allCommands();

std::vector<AdvancedCommand> basicCommands();

std::vector<AdvancedCommand> advancedCommands();

std::vector<AdvancedCommand> basicViewWalletCommands();

std::vector<AdvancedCommand> advancedViewWalletCommands();

std::vector<AdvancedCommand> allViewWalletCommands();
