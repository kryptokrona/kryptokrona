#include <SimpleWallet/SimpleWallet.h>

int main(int argc, char **argv)
{
    Config config = parseArguments(argc, argv);

    /* User requested --help or --version */
    if (config.exit)
    {
        return 0;
    }

    /* Only log to file so we don't have crap filling up the terminal */
    Logging::LoggerManager logManager;
    /* Should we maybe change this to INFO? On the one hand, DEBUGGING really
       spams the file, on the other hand, it exposes a lot of information
       that might help more devs debug a users issues. */
    logManager.setMaxLevel(Logging::DEBUGGING);

    Logging::FileLogger fileLogger;
    fileLogger.init("simplewallet.log");
    logManager.addLogger(fileLogger);

    Logging::LoggerRef logger(logManager, "simplewallet");

    /* Currency contains our coin parameters, such as decimal places, supply */
    CryptoNote::Currency currency 
        = CryptoNote::CurrencyBuilder(logManager).currency();

    System::Dispatcher localDispatcher;
    System::Dispatcher *dispatcher = &localDispatcher;

    /* Our connection to turtlecoind */
    std::unique_ptr<CryptoNote::INode> node(
        new CryptoNote::NodeRpcProxy(config.host, config.port, 
                                     logger.getLogger()));

    std::promise<std::error_code> errorPromise;
    std::future<std::error_code> error = errorPromise.get_future();
    auto callback = [&errorPromise](std::error_code e) 
                    {errorPromise.set_value(e); };

    node->init(callback);

    if (error.get())
    {
        throw("Failed to initialize node!");
    }

    /* Create the wallet instance */
    CryptoNote::WalletGreen wallet(*dispatcher, currency, *node, 
                                   logger.getLogger());

    /* Run the interactive wallet interface */
    run(wallet, *node);
}

void run(CryptoNote::WalletGreen &wallet, CryptoNote::INode &node)
{
    std::cout << PurpleMsg("TurtleCoin v" + std::string(PROJECT_VERSION)
                         + " Simplewallet") << std::endl;

    /* Open/import/generate the wallet */
    Action action = getAction();
    std::shared_ptr<WalletInfo> walletInfo = handleAction(wallet, action);

    ThreadHandler threadHandler;

    /* This will call shutdown when ctrl+c is hit. This is a lambda function.
       We use &variable to capture a variable, allowing us to use it in the
       lambda. */
    Tools::SignalHandler::install([&threadHandler, &walletInfo, &node] {
        shutdown(walletInfo->wallet, node, threadHandler);
        /* You shouldn't really do this. I'm lazy and stupid. */
        exit(0);
    });

    if (node.getLastKnownBlockHeight() == 0)
    {
        std::cout << RedMsg("It looks like TurtleCoind isn't open!")
                  << std::endl << std::endl
                  << RedMsg("Ensure TurtleCoind is open and has finished "
                            "initializing.")
                  << std::endl
                  << RedMsg("If it's still not working, try restarting "
                            "TurtleCoind. The daemon sometimes gets stuck.") 
                  << std::endl
                  << RedMsg("Alternatively, perhaps TurtleCoind can't "
                            "communicate with any peers.")
                  << std::endl << std::endl
                  << RedMsg("The wallet can't function until it can "
                            "communicate with the network.")
                  << std::endl
                  << PurpleMsg("Hit any key to exit: ");

        std::cin.get();

        shutdown(walletInfo->wallet, node, threadHandler);
    }
    else
    {
        /* Scan the chain for new transactions. In the case of an imported 
           wallet, we need to scan the whole chain to find any transactions. 
           If we opened the wallet however, we just need to scan from when we 
           last had it open. If we are generating a wallet, there is no need
           to check for transactions as there is no way the wallet can have
           received any money yet. */
        if (action != Generate)
        {
            findNewTransactions(node, walletInfo->wallet);
        }

        /* Look for transactions in the background */
        /* We need to use std::ref here, because when you pass a variable to
           a boost::thread, or a std::thread as well, they get copied by
           value, and then passed as reference to the function, so whilst it
           appears to have been passed by reference, it hasn't really. Using
           a std::ref wrapper fixes this. */
        boost::thread txWatcher(&transactionWatcher, walletInfo,
                                std::ref(threadHandler));

        welcomeMsg();

        inputLoop(walletInfo, node, threadHandler);

        shutdown(walletInfo->wallet, node, threadHandler);

        /* Wait for the tx watcher to terminate */
        txWatcher.join();
    }
}

std::shared_ptr<WalletInfo> handleAction(CryptoNote::WalletGreen &wallet,
                                         Action action)
{
    if (action == Generate)
    {
        return generateWallet(wallet);
    }
    else if (action == Open)
    {
        return openWallet(wallet);
    }
    else if (action == Import)
    {
        return importWallet(wallet);
    }
    else if (action == SeedImport)
    {
        return mnemonicImportWallet(wallet);
    }
    else
    {
        throw("Unimplemented action!");
    }
}

std::shared_ptr<WalletInfo> importWallet(CryptoNote::WalletGreen &wallet)
{
    Crypto::SecretKey privateSpendKey = getPrivateKey("Private Spend Key: ");
    Crypto::SecretKey privateViewKey = getPrivateKey("Private View Key: ");
    return importFromKeys(wallet, privateSpendKey, privateViewKey);
}

std::shared_ptr<WalletInfo> mnemonicImportWallet(CryptoNote::WalletGreen
                                                 &wallet)
{
    std::string mnemonicPhrase;

    Crypto::SecretKey privateSpendKey;
    Crypto::SecretKey privateViewKey;

    do
    {
        std::cout << "Mnemonic Phrase (25 words): ";
        std::getline(std::cin, mnemonicPhrase);
        boost::algorithm::trim(mnemonicPhrase);
    }
    while (!isValidMnemonic(mnemonicPhrase, privateSpendKey));

    CryptoNote::AccountBase::generateViewFromSpend(privateSpendKey, 
                                                   privateViewKey);

    return importFromKeys(wallet, privateSpendKey, privateViewKey);
}

std::shared_ptr<WalletInfo> importFromKeys(CryptoNote::WalletGreen &wallet,
                                           Crypto::SecretKey privateSpendKey, 
                                           Crypto::SecretKey privateViewKey)
{
    std::string walletFileName = getNewWalletFileName();
    std::string walletPass = getWalletPassword(true);

    connectingMsg();

    wallet.initializeWithViewKey(walletFileName, walletPass, privateViewKey);

    std::string walletAddress = wallet.createAddress(privateSpendKey);

    std::cout << PurpleMsg("\nYour wallet " + walletAddress 
                         + " has been successfully imported!")
              << std::endl << std::endl;

    return std::make_shared<WalletInfo>(walletFileName, walletPass, 
                                        walletAddress, wallet);
}

std::shared_ptr<WalletInfo> generateWallet(CryptoNote::WalletGreen &wallet)
{
    std::string walletFileName = getNewWalletFileName();
    std::string walletPass = getWalletPassword(true);

    
    CryptoNote::KeyPair spendKey;
    Crypto::SecretKey privateViewKey;

    Crypto::generate_keys(spendKey.publicKey, spendKey.secretKey);
    CryptoNote::AccountBase::generateViewFromSpend(spendKey.secretKey,
                                                   privateViewKey);

    wallet.initializeWithViewKey(walletFileName, walletPass, privateViewKey);

    std::string walletAddress = wallet.createAddress(spendKey.secretKey);

    promptSaveKeys(wallet);

    std::cout << RedMsg("If you lose these your wallet cannot be recreated!")
              << std::endl << std::endl;

    return std::make_shared<WalletInfo>(walletFileName, walletPass,
                                        walletAddress, wallet);
}

std::shared_ptr<WalletInfo> openWallet(CryptoNote::WalletGreen &wallet)
{
    std::string walletFileName = getExistingWalletFileName();

    while (true)
    {
        std::string walletPass = getWalletPassword(false);

        connectingMsg();

        try
        {
            wallet.load(walletFileName, walletPass);

            std::string walletAddress = wallet.getAddress(0);

            std::cout << std::endl
                      << PurpleMsg("Your wallet " + walletAddress
                                 + " has been successfully opened!\n\n");

            return std::make_shared<WalletInfo>(walletFileName, walletPass, 
                                                walletAddress, wallet);
        }
        catch (const std::system_error& e)
        {
            std::string walletGreenBadPwdMsg = 
                "Restored view public key doesn't correspond to secret key: "
                "The password is wrong";

            std::string walletGreenBadPwdMsg2 =
                "Restored spend public key doesn't correspond to secret key: "
                "The password is wrong";

            std::string walletLegacyBadPwdMsg =
                ": The password is wrong";

            std::string errorMsg = e.what();
                
            /* There are two different error messages depending upon if we're
               opening a walletgreen or a walletlegacy wallet */
            if (errorMsg == walletGreenBadPwdMsg || 
                errorMsg == walletGreenBadPwdMsg2 ||
                errorMsg == walletLegacyBadPwdMsg)
            {
                std::cout << RedMsg("Incorrect password! Try again.")
                          << std::endl;
            }
            else
            {
                throw(e);
            }
        }
    }
}

Crypto::SecretKey getPrivateKey(std::string msg)
{
    size_t privateKeyLen = 64;
    size_t size;

    std::string privateKeyString;
    Crypto::Hash privateKeyHash;
    Crypto::SecretKey privateKey;
    Crypto::PublicKey publicKey;

    while (true)
    {
        std::cout << msg;

        std::getline(std::cin, privateKeyString);
        boost::algorithm::trim(privateKeyString);

        if (privateKeyString.length() != privateKeyLen)
        {
            std::cout << RedMsg("Invalid private key, should be 64 "
                                "characters! Try again.") << std::endl;
            continue;
        }
        else if (!Common::fromHex(privateKeyString, &privateKeyHash, 
                  sizeof(privateKeyHash), size)
                 || size != sizeof(privateKeyHash))
        {
            std::cout << RedMsg("Invalid private key, failed to parse! "
                                "Ensure you entered it correctly.")
                      << std::endl;
            continue;
        }

        privateKey = *(struct Crypto::SecretKey *) &privateKeyHash;

        /* Just used for verification purposes before we pass it to
           walletgreen */
        if (!Crypto::secret_key_to_public_key(privateKey, publicKey))
        {
            std::cout << "Invalid private key, failed to parse! Ensure "
                         "you entered it correctly." << std::endl;
            continue;
        }

        return privateKey;
    }
}

std::string getExistingWalletFileName()
{
    std::string walletName;

    while (true)
    {
        std::cout << "What is the name of the wallet you want to open?: ";
        std::getline(std::cin, walletName);

        std::string walletFileName = walletName + ".wallet";

        if (!boost::filesystem::exists(walletFileName))
        {
            std::cout << RedMsg("A wallet with the filename " + walletFileName
                              + " doesn't exist!") << std::endl
                      << "Ensure you entered your wallet name correctly."
                      << std::endl;
        }
        else if (walletName == "")
        {
            std::cout << RedMsg("Wallet name can't be blank! Try again.")
                      << std::endl;
        }
        else
        {
            return walletFileName;
        }
    }
}

std::string getNewWalletFileName()
{
    std::string walletName;

    while (true)
    {
        std::cout << "What would you like to call your new wallet?: ";
        std::getline(std::cin, walletName);

        std::string walletFileName = walletName + ".wallet";

        if (boost::filesystem::exists(walletFileName))
        {
            std::cout << RedMsg("A wallet with the filename " + walletFileName
                              + " already exists!") << std::endl
                      << "Try another name." << std::endl;
        }
        else if (walletName == "")
        {
            std::cout << RedMsg("Wallet name can't be blank! Try again.")
                      << std::endl;
        }
        else
        {
            return walletFileName;
        }
    }
}

std::string getWalletPassword(bool verifyPwd)
{
    Tools::PasswordContainer pwdContainer;
    pwdContainer.read_password(verifyPwd);
    return pwdContainer.password();
}

Action getAction()
{
    while (true)
    {
        std::cout << std::endl << "Welcome, please choose an option below:"
                  << std::endl << std::endl << "\t[" << YellowMsg("G")
                  << "] - Generate a new wallet address" << std::endl 
                  << "\t[" << YellowMsg("O") << "] - Open a wallet already "
                  << "on your system" << std::endl << "\t[" << YellowMsg("S")
                  << "] - Regenerate your wallet using a seed phrase of words"
                  << std::endl << "\t[" << YellowMsg("I") << "] - Import "
                  << "your wallet using a View Key and Spend Key"
                  << std::endl << std::endl
                  << "or, press CTRL_C to exit: ";

        std::string answer;
        std::getline(std::cin, answer);

        char c = answer[0];
        c = std::tolower(c);

        if (c == 'o')
        {
            return Open;
        }
        else if (c == 'g')
        {
            return Generate;
        }
        else if (c == 'i')
        {
            return Import;
        }
        else if (c == 's')
        {
            return SeedImport;
        }
        else
        {
            std::cout << "Unknown command: " << RedMsg(answer) << std::endl;
        }
    }
}

bool isValidMnemonic(std::string &mnemonic_phrase, 
                     Crypto::SecretKey &private_spend_key)
{

  /* Uncommenting these will allow importing of different languages, exporting
     in different languages however has not been added, as it will require
     changing the export_keys command to take an argument to specify what
     language the seed should be exported in. For now, multilanguage support
     has been disabled as there are a couple of issues - we can't print out
     what words aren't present in the dictionary if we don't know what
     dictionary they are using, and it's a lot more friendly to work that
     out automatically rather than asking, and secondly, it is possible that
     dictionaries of other words can overlap enough to allow an esperanto
     seed for example to be imported as an English seed */

    /*
    static std::string languages[] = {"English", "Nederlands", "Français", 
                                      "Português", "Italiano", "Deutsch", 
                                      "русский язык", "简体中文 (中国)", 
                                      "Esperanto", "Lojban"};

    static const int num_of_languages = 10;
    */

    static std::string languages[] = {"English"};

    static const int num_of_languages = 1;

    static const int mnemonic_phrase_length = 25;

    std::vector<std::string> words;

    words = boost::split(words, mnemonic_phrase, ::isspace);

    if (words.size() != mnemonic_phrase_length)
    {
        std::cout << RedMsg("Invalid mnemonic phrase! Seed phrase is not 25 "
                            "words! Please try again.") << std::endl;

        logIncorrectMnemonicWords(words);
        return false;
    }

    /* Check every language for our phrase so the user doesn't have to specify
    it, this shouldn't be an issue as long as one language doesn't have enough
    of another languages words, might need some testing */
    for (int i = 0; i < num_of_languages; i++)
    {
        if (crypto::ElectrumWords::words_to_bytes(mnemonic_phrase, 
                                                  private_spend_key,
                                                  languages[i]))
        {
            return true;
        }
    }

    /* The issue with this is if we try and automagically determine what
    language the seed phrase is in, then we can't log words which aren't in the
    dictionary, we will have to take an argument to know what language they
    are in, but this is less user friendly. */
    std::cout << RedMsg("Invalid mnemonic phrase!") << std::endl;
    logIncorrectMnemonicWords(words);

    return false;
}

void logIncorrectMnemonicWords(std::vector<std::string> words)
{
    Language::Base *language 
        = Language::Singleton<Language::English>::instance();

    const std::vector<std::string> &dictionary = language->get_word_list();

    for (auto i : words)
    {
        if (std::find(dictionary.begin(), dictionary.end(), i) 
                   == dictionary.end())
        {
            std::cout << PurpleMsg(i) << RedMsg(" is not in the English word "
                                                "list!") << std::endl;
        }
    }
}

void promptSaveKeys(CryptoNote::WalletGreen &wallet)
{
    std::cout << "Welcome to your new wallet, here is your payment address:"
              << std::endl << PurpleMsg(wallet.getAddress(0))
              << std::endl << std::endl 
              << "Please copy your secret keys and mnemonic seed and store "
              << "them in a secure location: " << std::endl;

    printPrivateKeys(wallet);

    std::cout << std::endl;
}

void exportKeys(std::shared_ptr<WalletInfo> walletInfo)
{
    confirmPassword(walletInfo->walletPass);
    printPrivateKeys(walletInfo->wallet);
}

void printPrivateKeys(CryptoNote::WalletGreen &wallet)
{
    Crypto::SecretKey privateSpendKey = wallet.getAddressSpendKey(0).secretKey;
    Crypto::SecretKey privateViewKey = wallet.getViewKey().secretKey;

    Crypto::SecretKey derivedPrivateViewKey;

    CryptoNote::AccountBase::generateViewFromSpend(privateSpendKey,
                                                   derivedPrivateViewKey);

    bool deterministicPrivateKeys = derivedPrivateViewKey == privateViewKey;

    std::cout << GreenMsg("Private spend key: " 
                        + Common::podToHex(privateSpendKey)) << std::endl
              << GreenMsg("Private view key: " 
                        + Common::podToHex(privateViewKey)) << std::endl;

    if (deterministicPrivateKeys)
    {
        std::string mnemonicSeed;

        crypto::ElectrumWords::bytes_to_words(privateSpendKey, 
                                              mnemonicSeed,
                                              "English");

        std::cout << GreenMsg("Mnemonic seed: " + mnemonicSeed) << std::endl;
    }
}

void welcomeMsg()
{
    std::cout << "Use the " << YellowMsg("help") << " command to see the list "
              << "of available commands." << std::endl << "Use "
              << YellowMsg("exit") << " when closing to ensure your wallet "
              << "file doesn't get corrupted." << std::endl << std::endl;
}

void inputLoop(std::shared_ptr<WalletInfo> walletInfo, CryptoNote::INode &node,
               ThreadHandler &threadHandler)
{ 
    while (true)
    {
        std::cout << getPrompt(walletInfo);

        std::string command;
        std::getline(std::cin, command);

        /* Split into args to support legacy transfer command, for example
           transfer 5 TRTLxyz... 100, sends 100 TRTL to TRTLxyz... with a mixin
           of 5 */
        std::vector<std::string> words;
        words = boost::split(words, command, ::isspace);
        
        if (command == "")
        {
            // no-op
        }
        else if (command == "export_keys")
        {
            exportKeys(walletInfo);
        }
        else if (command == "help")
        {
            help();
        }
        else if (command == "balance")
        {
            balance(node, walletInfo->wallet);
        }
        else if (command == "address")
        {
            std::cout << GreenMsg(walletInfo->walletAddress) << std::endl;
        }
        else if (command == "incoming_transfers")
        {
            listTransfers(true, false, walletInfo->wallet);
        }
        else if (command == "outgoing_transfers")
        {
            listTransfers(false, true, walletInfo->wallet);
        }
        else if (command == "list_transfers")
        {
            listTransfers(true, true, walletInfo->wallet);
        }
        else if (command == "transfer")
        {
            transfer(walletInfo);
        }
        else if (words[0] == "transfer")
        {
            /* remove the first item from words - this is the "transfer"
               command, leaving us just the transfer arguments. */
            words.erase(words.begin());
            transfer(walletInfo, words);
        }
        else if (command == "exit")
        {
            return;
        }
        else if (command == "bc_height")
        {
            blockchainHeight(node, walletInfo->wallet);
        }
        else if (command == "quick_optimize")
        {
            quickOptimize(walletInfo->wallet);
        }
        else if (command == "full_optimize")
        {
            fullOptimize(walletInfo->wallet);
        }
        else if (command == "reset")
        {
            reset(node, walletInfo, threadHandler);
        }
        else
        {
            std::cout << "Unknown command: " << RedMsg(command) 
                      << ", use " << YellowMsg("help") << " command to list "
                      << "all possible commands." << std::endl;
        }
    }
}

void help()
{
    std::cout << "Available commands:" << std::endl
              << GreenMsg("help", 25)
              << "List this help message" << std::endl
              << GreenMsg("quick_optimize", 25)
              << "Quickly optimize your wallet to send large amounts"
              << std::endl
              << GreenMsg("full_optimize", 25)
              << "Fully optimize your wallet to send large amounts"
              << std::endl
              << GreenMsg("reset", 25)
              << "Discard cached data and recheck for transactions" << std::endl
              << GreenMsg("bc_height", 25)
              << "Show the blockchain height" << std::endl
              << GreenMsg("balance", 25)
              << "Display how much TRTL you have" << std::endl
              << GreenMsg("export_keys", 25)
              << "Export your private keys" << std::endl
              << GreenMsg("address", 25)
              << "Displays your payment address" << std::endl
              << GreenMsg("incoming_transfers", 25)
              << "Show incoming transfers" << std::endl
              << GreenMsg("outgoing_transfers", 25)
              << "Show outgoing transfers" << std::endl
              << GreenMsg("list_transfers", 25)
              << "Show all transfers" << std::endl
              << GreenMsg("transfer", 25)
              << "Send TRTL to someone" << std::endl
              << GreenMsg("exit", 25)
              << "Exit and save your wallet" << std::endl;
}

void balance(CryptoNote::INode &node, CryptoNote::WalletGreen &wallet)
{
    uint64_t unconfirmedBalance = wallet.getPendingBalance();
    uint64_t confirmedBalance = wallet.getActualBalance();
    uint64_t totalBalance = unconfirmedBalance + confirmedBalance;

    uint32_t localHeight = node.getLastLocalBlockHeight();
    uint32_t remoteHeight = node.getLastKnownBlockHeight();
    uint32_t walletHeight = wallet.getBlockCount();

    if (localHeight < remoteHeight)
    {
        std::cout << "Your daemon is not fully synced with the network!"
                  << std::endl << "Your balance may be incorrect until you "
                  << "are fully synced!" << std::endl << std::endl;
    }
    /* Small buffer because wallet height doesn't update instantly like node
       height does */
    else if (walletHeight + 1000 < remoteHeight)
    {
        std::cout << "The blockchain is still being scanned for incoming "
                  << "payments, and so your balance may be incorrect until "
                  << "this is complete!" << std::endl;
    }

    std::cout << "Available balance: "
              << GreenMsg(formatAmount(confirmedBalance)) << std::endl
              << "Locked (unconfirmed) balance: "
              << RedMsg(formatAmount(unconfirmedBalance))
              << std::endl << "Total balance: "
              << PurpleMsg(formatAmount(totalBalance)) << std::endl;
}

void blockchainHeight(CryptoNote::INode &node, CryptoNote::WalletGreen &wallet)
{
    uint32_t localHeight = node.getLastLocalBlockHeight();
    uint32_t remoteHeight = node.getLastKnownBlockHeight();
    uint32_t walletHeight = wallet.getBlockCount();

    /* This is the height that the wallet has been scanned to. The blockchain
       can be fully updated, but we have to walk the chain to find our
       transactions, and this number indicates that progress. */
    std::cout << "Wallet blockchain height: ";

    /* Small buffer because wallet height doesn't update instantly like node
       height does */
    if (walletHeight + 1000 > remoteHeight)
    {
        std::cout << GreenMsg(std::to_string(walletHeight));
    }
    else
    {
        std::cout << RedMsg(std::to_string(walletHeight));
    }

    std::cout << std::endl << "Local blockchain height: ";

    if (localHeight == remoteHeight)
    {
        std::cout << GreenMsg(std::to_string(localHeight));
    }
    else
    {
        std::cout << RedMsg(std::to_string(localHeight));
    }

    std::cout << std::endl << "Network blockchain height: "
              << GreenMsg(std::to_string(remoteHeight)) << std::endl;

    if (localHeight == 0 && remoteHeight == 0)
    {
        std::cout << "Uh oh, it looks like you don't have TurtleCoind open!"
                  << std::endl;
    }
    else if (localHeight == remoteHeight)
    {
        std::cout << "Yay! You are synced!" << std::endl;
    }
    else
    {
        std::cout << "Be patient, you are still syncing with the network!"
                  << std::endl;
    }
}

void shutdown(CryptoNote::WalletGreen &wallet, CryptoNote::INode &node,
              ThreadHandler &threadHandler)
{
    if (threadHandler.shouldDie)
    {
        std::cout << "Patience little turtle, we're already shutting down!" 
                  << std::endl;
        return;
    }
    else
    {
        std::cout << PurpleMsg("Saving wallet and shutting down, please "
                               "wait...") << std::endl;
    }

    threadHandler.shouldDie = true;

    /* This is a lambda function. We have to "capture" wallet and node so they
       can be used inside our lambda. */
    boost::thread shutdownThread([&wallet, &node]()
    {
            wallet.save();
            wallet.shutdown();
            node.shutdown();
    });

    bool shutdownSuccess 
       = shutdownThread.try_join_for(boost::chrono::seconds(20));

    /* If we fail to shutdown correctly, then force close. Otherwise when
       we go out of main scope we'll get an ugly error message when terminate
       is called on the node/wallet thread */
    if (!shutdownSuccess)
    {
        std::cout << RedMsg("Wallet took too long to save! Force closing.") 
                  << std::endl;
        std::cout << "Bye." << std::endl;
        exit(0);
    }

    std::cout << "Bye." << std::endl;
}

void printOutgoingTransfer(CryptoNote::WalletTransaction t)
{
    std::cout << RedMsg("Outgoing transfer: " + Common::podToHex(t.hash) +
                        "\nSpent: " + formatAmount(-t.totalAmount - t.fee) + 
                        "\nFee: " + formatAmount(t.fee) +
                        "\nTotal Spent: " + formatAmount(-t.totalAmount)) 
              << std::endl << std::endl;
}

void printIncomingTransfer(CryptoNote::WalletTransaction t)
{
    std::cout << GreenMsg("Incoming transfer: " + Common::podToHex(t.hash) +
                          "\nAmount: " + formatAmount(t.totalAmount))
              << std::endl << std::endl;
}

void listTransfers(bool incoming, bool outgoing, 
                   CryptoNote::WalletGreen &wallet)
{
    size_t numTransactions = wallet.getTransactionCount();
    int64_t totalSpent = 0;
    int64_t totalReceived = 0;

    for (size_t i = 0; i < numTransactions; i++)
    {
        CryptoNote::WalletTransaction t = wallet.getTransaction(i);

        if (t.totalAmount < 0 && outgoing)
        {
            printOutgoingTransfer(t);
            totalSpent += -t.totalAmount;
        }
        else if (t.totalAmount > 0 && incoming)
        {
            printIncomingTransfer(t);
            totalReceived += t.totalAmount;
        }
    }

    if (incoming)
    {
        std::cout << GreenMsg("Total received: " + formatAmount(totalReceived))
                  << std::endl;
    }

    if (outgoing)
    {
        std::cout << RedMsg("Total spent: " + formatAmount(totalSpent))
                  << std::endl;
    }
}

void transactionWatcher(std::shared_ptr<WalletInfo> walletInfo,
                        ThreadHandler &threadHandler)
{
    size_t transactionCount = walletInfo->wallet.getTransactionCount();

    while(true)
    {
        if (threadHandler.shouldDie)
        {
            return;
        }

        /* Pause watching until we get the signal to unpause. Will fuck up
           reset. */
        if (threadHandler.shouldPause)
        {
            threadHandler.havePaused = true;

            while (threadHandler.shouldPause)
            {
                std::this_thread::sleep_for(std::chrono::seconds(5));
            }
        }

        size_t tmpTransactionCount = walletInfo->wallet.getTransactionCount();

        if (tmpTransactionCount != transactionCount)
        {
            for (size_t i = transactionCount; i < tmpTransactionCount; i++)
            {
                CryptoNote::WalletTransaction t 
                    = walletInfo->wallet.getTransaction(i);

                /* Don't print outgoing or fusion transfers */
                if (t.totalAmount > 0)
                {
                    std::cout << std::endl
                              << PurpleMsg("New transaction found!")
                              << std::endl
                              << GreenMsg("Incoming transfer: " 
                                        + Common::podToHex(t.hash) +
                                          "\nAmount: " 
                                        + formatAmount(t.totalAmount))
                              << std::endl
                              << getPrompt(walletInfo)
                              << std::flush;
                }
            }

            transactionCount = tmpTransactionCount;
        }

        walletInfo->wallet.updateInternalCache();

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

void reset(CryptoNote::INode &node, std::shared_ptr<WalletInfo> walletInfo,
           ThreadHandler &threadHandler)
{
    std::cout << PurpleMsg("Resetting wallet...") << std::endl;

    /* Pause the transaction watcher whilst we reset. It could potentially
       fuck up, and if it doesn't, it still will be printing stuff when we
       only want findNewTransactions to be printing */
    threadHandler.shouldPause = true;

    while (!threadHandler.havePaused)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    /* Wallet is now unitialized. You must reinit with load, initWithKeys,
       or whatever. This function wipes the cache, then saves the wallet. */
    walletInfo->wallet.clearCacheAndShutdown();

    /* Now, we reopen the wallet. It now has no cached tx's, and balance */
    walletInfo->wallet.load(walletInfo->walletFileName,
                            walletInfo->walletPass);

    /* Now we rescan the chain to re-discover our balance and transactions */
    findNewTransactions(node, walletInfo->wallet);

    /* Then we save the wallet again, with the hopefully fixed balance. This
       step is not strictly neccessary, but it's nice to do in case the user
       doesn't shut down cleanly. */
    walletInfo->wallet.save();

    threadHandler.shouldPause = false;
    threadHandler.shouldDie = false;
    threadHandler.havePaused = false;
}

void findNewTransactions(CryptoNote::INode &node, 
                         CryptoNote::WalletGreen &wallet)
{
    uint32_t localHeight = node.getLastLocalBlockHeight();
    uint32_t walletHeight = wallet.getBlockCount();
    uint32_t remoteHeight = node.getLastKnownBlockHeight();

    size_t transactionCount = wallet.getTransactionCount();

    int stuckCounter = 0;

    if (localHeight != remoteHeight)
    {
        std::cout << "Your TurtleCoind isn't fully synced yet!" << std::endl
                  << "Until you are fully synced, you won't be able to send "
                  << "transactions, and your balance may be missing or "
                  << "incorrect!" << std::endl << std::endl;
    }

    /* If we open a legacy wallet then it will load the transactions but not
       have the walletHeight == transaction height. Lets just throw away the
       transactions and rescan. */
    if (walletHeight == 1 && transactionCount != 0)
    {
        std::cout << "Upgrading your wallet from an older version of the "
                  << "software..." << std::endl << "Unfortunately, we have "
                  << "to rescan the chain to find your transactions."
                  << std::endl;
        transactionCount = 0;
        wallet.clearCaches(true, false);
    }

    if (walletHeight == 1)
    {
        std::cout << "Scanning through the blockchain to find transactions "
                     "that belong to you." << std::endl
                     << "Please wait, this will take some time."
                     << std::endl << std::endl;
    }
    else
    {
        std::cout << "Scanning through the blockchain to find any new "
                     "transactions you received whilst your wallet wasn't "
                     "open." << std::endl
                     << "Please wait, this may take some time."
                     << std::endl << std::endl;
    }

    while (walletHeight < localHeight)
    {
        wallet.updateInternalCache();

        size_t tmpTransactionCount = wallet.getTransactionCount();

        uint32_t tmpWalletHeight = wallet.getBlockCount();

        if (tmpWalletHeight == walletHeight)
        {
            stuckCounter++;
        }
        else
        {
            stuckCounter = 0;
        }

        /* Should be around a minute */
        if (stuckCounter > 60)
        {
            std::cout << RedMsg("It looks like syncing might have got stuck...")
                      << std::endl
                      << RedMsg("This is probably due to TurtleCoind not "
                                "responding. Try restarting TurtleCoind and "
                                "the wallet.") << std::endl
                      << RedMsg("If this error still continues after "
                                "restarting the software, you may need to "
                                "resync the blockchain.") << std::endl
                      << RedMsg("See https://github.com/turtlecoin/turtlecoin/"
                                "wiki/Bootstrapping-the-Blockchain for a "
                                "quicker sync.") << std::endl;

        }

        walletHeight = tmpWalletHeight;

        localHeight = node.getLastLocalBlockHeight();
        remoteHeight = node.getLastKnownBlockHeight();

        std::cout << GreenMsg(std::to_string(walletHeight))
                  << " of " << PurpleMsg(std::to_string(localHeight))
                  << std::endl;

        if (tmpTransactionCount != transactionCount)
        {
            for (size_t i = transactionCount; i < tmpTransactionCount; i++)
            {
                CryptoNote::WalletTransaction t = wallet.getTransaction(i);

                /* Don't print out fusion transactions */
                if (t.totalAmount != 0)
                {
                    std::cout << std::endl
                              << PurpleMsg("New transaction found!")
                              << std::endl << std::endl;
                }

                if (t.totalAmount < 0)
                {
                    printOutgoingTransfer(t);
                }
                /* Don't display fusion transactions */
                else if (t.totalAmount > 0)
                {
                    printIncomingTransfer(t);
                }
            }

            transactionCount = tmpTransactionCount;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << GreenMsg("Finished scanning blockchain!") << std::endl
              << std::endl;
}

ColouredMsg getPrompt(std::shared_ptr<WalletInfo> walletInfo)
{
    int extPos = walletInfo->walletFileName.find('.');

    std::string walletName = walletInfo->walletFileName.substr(0, extPos);

    return PurpleMsg("[TRTL " + walletName + "]: ");
}

void connectingMsg()
{
    std::cout << std::endl << "Making initial contact with TurtleCoind."
              << std::endl
              << "Please note, this sometimes can take a long time."
              << std::endl << "Please wait..." << std::endl
              << std::endl;
}
