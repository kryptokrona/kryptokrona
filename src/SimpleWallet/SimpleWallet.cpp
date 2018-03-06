#include "SimpleWallet.h"

int main()
{
    Logging::LoggerManager logManager;
    Logging::LoggerRef logger(logManager, "simplewallet");

    CryptoNote::Currency currency = CryptoNote::CurrencyBuilder(logManager).currency();
    CryptoNote::NodeRpcProxy node("localhost", 11898, logger.getLogger());
    System::Dispatcher dispatcher;

    CryptoNote::WalletGreen wallet(dispatcher, currency, node, logger.getLogger());
    
    bool success = true;

    do
    {
        std::cout << "TurtleCoin v" 
                  << PROJECT_VERSION 
                  << " Simplewallet" 
                  << std::endl;

        Action action = getAction();

        if (action == Generate)
        {
            generateWallet(wallet);
        }
        else if (action == Open)
        {
            success = openWallet(wallet);
        }
        else if (action == Import)
        {
            importWallet(wallet);
        }
        else if (action == SeedImport)
        {
            mnemonicImportWallet(wallet);
        }
    }
    while (!success);

    std::cout << "Bye." << std::endl;
}

void importWallet(CryptoNote::WalletGreen &wallet)
{
    Crypto::SecretKey privateSpendKey = getPrivateKey("Private Spend Key: ");
    Crypto::SecretKey privateViewKey = getPrivateKey("Private View Key: ");
    importFromKeys(wallet, privateSpendKey, privateViewKey);
}

void mnemonicImportWallet(CryptoNote::WalletGreen &wallet)
{
    std::string mnemonicPhrase;

    Crypto::SecretKey privateSpendKey;
    Crypto::SecretKey privateViewKey;

    do
    {
        std::cout << "Mnemonic Phrase (25 words): ";
        std::getline(std::cin, mnemonicPhrase);
    }
    while (!isValidMnemonic(mnemonicPhrase, privateSpendKey));

    CryptoNote::AccountBase::generateViewFromSpend(privateSpendKey, privateViewKey);

    importFromKeys(wallet, privateSpendKey, privateViewKey);
}

void importFromKeys(CryptoNote::WalletGreen &wallet, 
                    Crypto::SecretKey privateSpendKey, 
                    Crypto::SecretKey privateViewKey)
{
    std::string walletFileName = getNewWalletFileName();
    std::string walletPass = getWalletPassword(true);
    wallet.initializeWithViewKey(walletFileName, walletPass, privateViewKey);
    wallet.createAddress(privateSpendKey);
}

void generateWallet(CryptoNote::WalletGreen &wallet)
{
    std::string walletFileName = getNewWalletFileName();
    std::string walletPass = getWalletPassword(true);
    wallet.initialize(walletFileName, walletPass);
}

bool openWallet(CryptoNote::WalletGreen &wallet)
{
    std::string walletFileName = getExistingWalletFileName();

    while (true)
    {
        std::string walletPass = getWalletPassword(false);

        try
        {
            wallet.load(walletFileName, walletPass);
            return true;
        }
        catch (const std::system_error& e)
        {
            /* Blegh, char[] not std::string */
            if (strcmp(e.what(), "Restored view public key doesn't correspond "
                                 "to secret key: The password is wrong") == 0)
            {
                std::cout << "Incorrect password! Try again." << std::endl;
            }
            /* Different message depending upon if we open a walletlegacy or
               a walletgreen wallet */
            else if (strcmp(e.what(), ": The password is wrong") == 0)
            {
                std::cout << "Incorrect password! Try again." << std::endl;
            }
            else
            {
                std::cout << "Failed to load wallet: "  << e.what() << std::endl;
                return false;
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

        if (privateKeyString.length() != privateKeyLen)
        {
            std::cout << "Invalid private key, should be 64 characters! "
                         "Try again." << std::endl;
            continue;
        }
        else if (!Common::fromHex(privateKeyString, &privateKeyHash, sizeof(privateKeyHash), size)
                 || size != sizeof(privateKeyHash))
        {
            std::cout << "Invalid private key, failed to parse! Ensure "
                         "you entered it correctly." << std::endl;
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
            std::cout << "A wallet with that name doesn't exist! Ensure you "
                         "entered your wallet name correctly." << std::endl;
        }
        else if (walletName == "")
        {
            std::cout << "Wallet name can't be empty! Try again." << std::endl;
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
            std::cout << "A file with that name already exists! Try another one." << std::endl;
        }
        else if (walletName == "")
        {
            std::cout << "Wallet name can't be empty! Try again." << std::endl;
        }
        else
        {
            return walletFileName;
        }
    }
}

std::string getWalletPassword(bool verify)
{
    Tools::PasswordContainer pwdContainer;
    pwdContainer.read_password(verify);
    return pwdContainer.password();
}

Action getAction()
{
    while (true)
    {
        std::cout << std::endl << "Welcome, please choose an option below:"
                  << std::endl 
                  << std::endl << "\t[G] - Generate a new wallet address"
                  << std::endl << "\t[O] - Open a wallet already on your system"
                  << std::endl << "\t[S] - Regenerate your wallet using a seed phrase of words"
                  << std::endl << "\t[I] - Import your wallet using a View Key and Spend Key"
                  << std::endl << std::endl << "or, press CTRL_C to exit: "
                  << std::flush;

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
            std::cout << "Unknown command: " << answer << std::endl;
        }
    }
}

bool isValidMnemonic(std::string &mnemonic_phrase, Crypto::SecretKey &private_spend_key)
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
     seed for example to be imported as an english seed */

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
        std::cout << "Invalid mnemonic phrase! Seed phrase is not 25 words! Please try again." << std::endl;
        logIncorrectMnemonicWords(words);
        return false;
    }

    /* Check every language for our phrase so the user doesn't have to specify
    it, this shouldn't be an issue as long as one language doesn't have enough
    of another languages words, might need some testing */
    for (int i = 0; i < num_of_languages; i++)
    {
        if (crypto::ElectrumWords::words_to_bytes(mnemonic_phrase, private_spend_key, languages[i]))
        {
            return true;
        }
    }

    /* The issue with this is if we try and automagically determine what language
    the seed phrase is in, then we can't log words which aren't in the x
    dictionary, we will have to take an argument to know what language they
    are in, but this is less user friendly. */
    std::cout << "Invalid mnemonic phrase!" << std::endl;
    logIncorrectMnemonicWords(words);

    return false;
}

void logIncorrectMnemonicWords(std::vector<std::string> words)
{
    Language::Base *language = Language::Singleton<Language::English>::instance();

    const std::vector<std::string> &dictionary = language->get_word_list();

    for (auto i : words)
    {
        if (std::find(dictionary.begin(), dictionary.end(), i) == dictionary.end())
        {
            std::cout << i << " is not in the english word list!" << std::endl;
        }
    }
}
