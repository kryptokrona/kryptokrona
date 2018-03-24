/*
Copyright (C) 2018, The TurtleCoin developers

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <SimpleWallet/Transfer.h>

bool parseAmount(std::string strAmount, uint64_t &amount)
{
    boost::algorithm::trim(strAmount);

    size_t pointIndex = strAmount.find_first_of('.');
    size_t fractionSize;
    size_t numDecimalPlaces = 2;

    if (std::string::npos != pointIndex)
    {
        fractionSize = strAmount.size() - pointIndex - 1;

        while (numDecimalPlaces < fractionSize && '0' == strAmount.back())
        {
            strAmount.erase(strAmount.size() - 1, 1);
            fractionSize--;
        }

        if (numDecimalPlaces < fractionSize)
        {
            return false;
        }

        strAmount.erase(pointIndex, 1);
    }
    else
    {
        fractionSize = 0;
    }

    if (strAmount.empty())
    {
        return false;
    }

    if (!std::all_of(strAmount.begin(), strAmount.end(), ::isdigit))
    {
        return false;
    }

    if (fractionSize < numDecimalPlaces)
    {
        strAmount.append(numDecimalPlaces - fractionSize, '0');
    }

    return Common::fromString(strAmount, amount);
}

bool confirmTransaction(CryptoNote::TransactionParameters t,
                        std::shared_ptr<WalletInfo> walletInfo)
{
    std::cout << std::endl 
              << InformationMsg("Confirm Transaction?") << std::endl;

    std::cout << "You are sending " 
              << SuccessMsg(formatAmount(t.destinations[0].amount))
              << ", with a fee of " << SuccessMsg(formatAmount(t.fee))
              << std::endl
              << "FROM: " << InformationMsg(walletInfo->walletFileName) 
              << std::endl
              << "TO: " << std::endl
              << InformationMsg(t.destinations[0].address)
              << std::endl << std::endl;

    if (confirm("Is this correct?"))
    {
        confirmPassword(walletInfo->walletPass);
        return true;
    }
    else
    {
        return false;
    }
}

void sendMultipleTransactions(CryptoNote::WalletGreen &wallet,
                              std::vector<CryptoNote::TransactionParameters>
                              transfers)
{
    size_t numTxs = transfers.size();
    size_t currentTx = 1;

    std::cout << "Your transaction has been split up into " << numTxs
              << " separate transactions of " 
              << formatAmount(transfers[0].destinations[0].amount)
              << ". It may take some time to send all the transactions, "
              << "please be patient." << std::endl << std::endl;

    for (auto tx : transfers)
    {
        while (true)
        {
            std::cout << "Attempting to send transaction "
                      << InformationMsg(std::to_string(currentTx))
                      << " of " << InformationMsg(std::to_string(numTxs))
                      << std::endl;

            wallet.updateInternalCache();

            uint64_t neededBalance = tx.destinations[0].amount + tx.fee;

            if (neededBalance < wallet.getActualBalance())
            {
                size_t id = wallet.transfer(tx);

                CryptoNote::WalletTransaction sentTx 
                    = wallet.getTransaction(id);

                std::cout << SuccessMsg("Transaction has been sent! ID:\n" + 
                                        Common::podToHex(sentTx.hash))
                          << std::endl << std::endl;

                break;
            }

            std::cout << "Not enough balance available to send transaction, "
                      << "this is because some of your balance is used when "
                      << "sending another transaction to help hide the size "
                      << "of your transaction, and is locked for a short "
                      << "time. It will return shortly." << std::endl
                      << "Needed balance: " << formatAmount(neededBalance) 
                      << std::endl << "Available balance: " 
                      << formatAmount(wallet.getActualBalance())
                      << std::endl << "Locked balance: "
                      << formatAmount(wallet.getPendingBalance())
                      << std::endl << "Will try again in 5 seconds..."
                      << std::endl << std::endl;

            std::this_thread::sleep_for(std::chrono::seconds(5));
        }

        currentTx++;
    }

    std::cout << SuccessMsg("All transactions sent!") << std::endl;
}

void splitTx(CryptoNote::WalletGreen &wallet, 
             CryptoNote::TransactionParameters p)
{
    std::cout << "Wallet optimization failed, transactions are still too large "
              << "to send in one chunk, splitting into multiple chunks." 
              << std::endl << "This may take a long time as portions of your "
              << "balance get locked whilst sending a transaction."
              << std::endl << "It may also slightly raise the fee you have "
              << "to pay, and hence reduce the total amount you can send if "
              << "your balance cannot cover it." << std::endl;

    if (!confirm("Is this OK?"))
    {
        std::cout << WarningMsg("Cancelling transaction.") << std::endl;
        return;
    }

    CryptoNote::TransactionParameters restoreInitialTx = p;

    uint64_t maxSize = CryptoNote::parameters::CRYPTONOTE_MAX_SAFE_TX_SIZE;
    size_t txSize = wallet.getTxSize(p);
    uint64_t minFee = CryptoNote::parameters::MINIMUM_FEE;

    for (int numTxMultiplier = 2; ; numTxMultiplier++)
    {
        /* We modify p a bit in this function, so restore back to initial
           state each time */
        p = restoreInitialTx;

        /* We can't just evenly divide a transaction up to be < 115k bytes by
           decreasing the amount we're sending, because depending upon the
           inputs we might need to split into more transactions, so a good
           start is attempting to split them into chunks of 55k bytes or so.
           We then check at the end that each transaction is small enough, and
           if not, we up the numTxMultiplier and try again with more
           transactions. */
        int numTransactions 
            = int(numTxMultiplier * 
                 (std::ceil(double(txSize) / double(maxSize))));

        /* Split the requested fee over each transaction, i.e. if a fee of 200
           TRTL was requested and we split it into 4 transactions each one will
           have a fee of 5 TRTL. If the fee per transaction is less than the
           min fee, use the min fee. */
        uint64_t feePerTx = std::max (p.fee / numTransactions, minFee);

        uint64_t totalFee = feePerTx * numTransactions;

        uint64_t totalCost = p.destinations[0].amount + totalFee;
        
        /* If we have to use the minimum fee instead of splitting the total fee,
           then it is possible the user no longer has the balance to cover this
           transaction. So, we slightly lower the amount they are sending. */
        if (totalCost > wallet.getActualBalance())
        {
            p.destinations[0].amount = wallet.getActualBalance() - totalFee;
        }

        uint64_t amountPerTx = p.destinations[0].amount / numTransactions;
        /* Left over amount from integral division */
        uint64_t change = p.destinations[0].amount % numTransactions;

        std::vector<CryptoNote::TransactionParameters> transfers;

        for (int i = 0; i < numTransactions; i++)
        {
            CryptoNote::TransactionParameters tmp = p;
            tmp.destinations[0].amount = amountPerTx;
            tmp.fee = feePerTx;
            transfers.push_back(tmp);
        }

        /* Add the extra change to the first transaction */
        transfers[0].destinations[0].amount += change;

        for (auto tx : transfers)
        {
            /* One of the transfers is too large. Retry, cutting the
               transactions into smaller pieces */
            if (wallet.txIsTooLarge(tx))
            {
                std::cout << "Split up transactions are still too large! "
                          << "Splitting up into smaller chunks." << std::endl;
                continue;
            }
        }

        sendMultipleTransactions(wallet, transfers);
        return;
    }
}

size_t makeFusionTransaction(CryptoNote::WalletGreen &wallet, 
                             uint64_t threshold)
{
    uint64_t bestThreshold = threshold;
    size_t optimizable = 0;

    /* Find the best threshold by starting at threshold and decreasing by
       half till we get to the minimum amount, storing the threshold that
       gave us the most amount of optimizable amounts */
    while (threshold > CryptoNote::parameters::MINIMUM_FEE)
    {
        CryptoNote::IFusionManager::EstimateResult r 
            = wallet.estimate(threshold);

        if (r.fusionReadyCount > optimizable)
        {
            optimizable = r.fusionReadyCount;
            bestThreshold = threshold;
        }

        threshold /= 2;
    }

    /* Can throw if it can't create - lol what are error codes - just catch
       it and assume we can't fusion anymore */
    try
    {
        return wallet.createFusionTransaction(bestThreshold, 
                                              CryptoNote::parameters
                                                        ::DEFAULT_MIXIN);
    }
    catch (const std::runtime_error)
    {
        return CryptoNote::WALLET_INVALID_TRANSACTION_ID;
    }
}

void quickOptimize(CryptoNote::WalletGreen &wallet)
{
    std::cout << "Attempting to optimize your wallet to allow you to send "
              << "large amounts at once. " << std::endl 
              << "You can run this command as many times as you like."
              << std::endl << "You will be informed when your wallet is fully "
              << "optimized." << std::endl
              << WarningMsg("This may take a long time!") << std::endl;

    if (!confirm("Do you want to proceed?"))
    {
        std::cout << WarningMsg("Cancelling optimization.") << std::endl;
        return;
    }

    if (!optimize(wallet, wallet.getActualBalance()))
    {
        std::cout << SuccessMsg("Wallet fully optimized!") << std::endl;
    }
    else
    {
        std::cout << SuccessMsg("Optimization completed!") << std::endl
                  << "Your wallet can still be optimized more if you run "
                  << "this command again." << std::endl << "Consider using "
                  << "the " << SuggestionMsg("full_optimize") << " command to "
                  << "automate the process." << std::endl;
    }
}

void fullOptimize(CryptoNote::WalletGreen &wallet)
{
    std::cout << "Attempting to fully optimize your wallet to allow you to "
              << "send large amounts at once. " << std::endl
              << WarningMsg("This may take a very long time!") << std::endl;

    if (!confirm("Do you want to proceed?"))
    {
        std::cout << WarningMsg("Cancelling optimization.") << std::endl;
        return;
    }

    for (int i = 1;;i++)
    {
        std::cout << InformationMsg("Running optimization round "
                                  + std::to_string(i) + "...")
                  << std::endl;

        /* Optimize as many times as possible until optimization is no longer
           possible. */
        if (!optimize(wallet, wallet.getActualBalance()))
        {
            break;
        }
    }

    std::cout << SuccessMsg("Full optimization completed!") << std::endl;
}

bool optimize(CryptoNote::WalletGreen &wallet, uint64_t threshold)
{
    std::vector<Crypto::Hash> fusionTransactionHashes;

    while (true)
    {
        /* Create as many fusion transactions until we can't send anymore,
           either because balance is locked too much or we can no longer
           optimize anymore transactions */
        size_t tmpFusionTxID = makeFusionTransaction(wallet, threshold);

        if (tmpFusionTxID == CryptoNote::WALLET_INVALID_TRANSACTION_ID)
        {
            break;
        }
        else
        {
            CryptoNote::WalletTransaction w
                = wallet.getTransaction(tmpFusionTxID);
            fusionTransactionHashes.push_back(w.hash);
        }
    }

    if (fusionTransactionHashes.empty())
    {
        return false;
    }

    /* Hurr durr grammar */
    if (fusionTransactionHashes.size() == 1)
    {
        std::cout << SuccessMsg("1 fusion transaction has been sent, waiting "
                                "for balance to return and unlock...")
                  << std::endl << std::endl;
    }
    else
    {
        std::cout << SuccessMsg(std::to_string(fusionTransactionHashes.size()) +
                                " fusion transactions have been sent, waiting "
                                "for balance to return and unlock...")
              << std::endl << std::endl;
    }

    wallet.updateInternalCache();

    /* Short sleep to ensure it's in the transaction pool when we poll it */
    std::this_thread::sleep_for(std::chrono::seconds(1));

    while (true)
    {
        std::vector<CryptoNote::WalletTransactionWithTransfers> 
            unconfirmedTransactions = wallet.getUnconfirmedTransactions();

        std::vector<Crypto::Hash> unconfirmedTxHashes;

        for (auto t : unconfirmedTransactions)
        {
            unconfirmedTxHashes.push_back(t.transaction.hash);
        }

        bool fusionCompleted = true;

        /* Is our fusion transaction still unconfirmed? We can't gain the
           benefits of fusioning if the balance hasn't unlocked, so we can
           send this new optimized balance */
        for (auto tx : fusionTransactionHashes)
        {
            /* If the fusion transaction hash is present in the unconfirmed
               transactions pool, we need to wait for it to complete. */
            if (std::find(unconfirmedTxHashes.begin(),
                          unconfirmedTxHashes.end(), tx) 
                       != unconfirmedTxHashes.end())
            {
                fusionCompleted = false; 
            }
            else
            {
                /* We can't find this transaction in the unconfirmed
                   transaction pool anymore, so it has been confirmed. Remove
                   it so we both have to check less transactions each time,
                   and we can easily update the transactions left to confirm
                   output message */
                fusionTransactionHashes.erase(std::remove
                    (fusionTransactionHashes.begin(),
                     fusionTransactionHashes.end(), tx), 
                     fusionTransactionHashes.end());
            }
        }

        if (!fusionCompleted)
        {
            std::cout << WarningMsg("Balance is still locked, "
                  + std::to_string(fusionTransactionHashes.size()));

            /* More grammar... */
            if (fusionTransactionHashes.size() == 1)
            {
                std::cout << WarningMsg(" fusion transaction still to be "
                                        "confirmed.");
            }
            else
            {
                std::cout << WarningMsg(" fusion transactions still to be "
                                        "confirmed.");
            }
            
            std::cout << std::endl
                      << SuccessMsg("Will try again in 5 seconds...")
                      << std::endl;

            std::this_thread::sleep_for(std::chrono::seconds(5));

            wallet.updateInternalCache();
        }
        else
        {
            std::cout << SuccessMsg("All fusion transactions confirmed!")
                      << std::endl;
            break;
        }
    }

    return true;
}

void fusionTX(CryptoNote::WalletGreen &wallet, 
              CryptoNote::TransactionParameters p)
{
    std::cout << WarningMsg("Your transaction is too large to be accepted by "
                            "the network!")
              << std::endl << "We're attempting to optimize your "
              << "wallet, which hopefully will make the transaction small "
              << "enough to fit in a block." << std::endl 
              << "Please wait, this will take some time..." << std::endl 
              << std::endl;

    /* We could check if optimization succeeded, but it's not really needed
       because we then check if the transaction is too large... it could have
       potentially become valid because another payment came in. */
    optimize(wallet, p.destinations[0].amount + p.fee);

    auto startTime = std::chrono::system_clock::now();

    while (wallet.getActualBalance() < p.destinations[0].amount + p.fee)
    {
        /* Break after a minute just in case something has gone wrong */
        if ((std::chrono::system_clock::now() - startTime) > 
             std::chrono::minutes(1))
        {
            std::cout << WarningMsg("Fusion transactions have "
                                    "completed, however available "
                                    "balance is less than transfer "
                                    "amount specified.") << std::endl
                      << WarningMsg("Transfer aborted, please review "
                                    "and start a new transfer.")
                      << std::endl;

            return;
        }

        std::cout << WarningMsg("Optimization completed, but balance "
                                "is not fully unlocked yet!")
                  << std::endl
                  << SuccessMsg("Will try again in 5 seconds...")
                  << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    try
    {
        if (wallet.txIsTooLarge(p))
        {
            splitTx(wallet, p);
        }
        else
        {
            
            size_t id = wallet.transfer(p);
            CryptoNote::WalletTransaction tx = wallet.getTransaction(id);

            std::cout << SuccessMsg("Transaction has been sent! ID:\n" 
                                + Common::podToHex(tx.hash))
                      << std::endl;
        }
    }
    catch (const std::system_error &e)
    {
        std::string errMsg = e.what();

        /* The internal node error I believe is caused by the same issue as
           the mixin error. Rocksteady explained this as not enough traffic
           having occured on the network to allow your to mixin with.
           Hopefully, this will only occur on the testnet and not the main
           network. It seems sending multiple smaller transactions will
           provide the network with more change to allow tx's to go through.
           However, in some wallets that have only received one big single
           transaction, they may be unable to send at all without lowering
           their mixin count to 0 */
        if (errMsg == "MixIn count is too big"
         || errMsg == "Internal node error")
        {
            std::cout << WarningMsg("Failed to send transaction!") << std::endl
                      << "Unable to find enough outputs to mix with."
                      << std::endl << "Try lowering the amount you are "
                      << "sending in one transaction." << std::endl
                      << "Alternatively, you can try lowering the mixin count "
                      << "to 0, but this will compromise privacy." << std::endl;
        }
        else
        {
            std::cout << WarningMsg("Failed to send transaction!")
                      << std::endl << "Error message: " << errMsg
                      << std::endl;
        }
    }
}

void transfer(std::shared_ptr<WalletInfo> walletInfo,
              std::vector<std::string> args)
{
    uint16_t mixin;
    std::string address;
    uint64_t amount;
    uint64_t fee = CryptoNote::parameters::MINIMUM_FEE;
    std::string extra;

    /* Check we have enough args for the default required parameters */
    if (args.size() >= 3)
    {
        if (parseMixin(args[0]) && parseAddress(args[1])
         && parseAmount(args[2]))
        {
            mixin = std::stoi(args[0]);
            address = args[1];
            parseAmount(args[2], amount);
        }
        else
        {
            return;
        }
    }
    else
    {
        std::cout << WarningMsg("Not enough arguments given!") << std::endl
                  << "Try running just " << SuggestionMsg("transfer")
                  << " for a walk through guide to transferring." << std::endl;
        return;
    }

    for (size_t i = 0; i < args.size(); i++)
    {
        if (args[i] == "-f")
        {
            if (i+1 < args.size())
            {
                if (parseFee(args[i+1]))
                {
                    parseAmount(args[i+1], fee);
                }
                else
                {
                    return;
                }
            }
            else
            {
                std::cout << WarningMsg("Fee flag given but no fee follows!")
                          << std::endl;
                return;
            }
        }
        else if (args[i] == "-p")
        {
            if (i+1 < args.size())
            {
                std::vector<uint8_t> extraVec;
                std::string extraString;

                /* Convert the payment ID into an "extra" */
                if (!CryptoNote::createTxExtraWithPaymentId(args[i+1],
                                                            extraVec))
                {
                    std::cout << WarningMsg("Failed to parse payment ID! "
                                            "Payment ID's are 64 character "
                                            "hexadecimal strings.")
                              << std::endl;
                    return;
                }
                else
                {
                    /* Then convert the "extra" back into a string so we
                       can pass the argument that walletgreen expects. 
                       Note this string is not the same as the original
                       paymentID string! */
                    for (auto i : extraVec)
                    {
                        extraString += static_cast<char>(i);
                    }
                }

                extra = extraString;
            }
            else
            {
                std::cout << WarningMsg("Payment ID flag given but no payment "
                                        "ID follows!") << std::endl;
                return;
            }
        }
    }

    doTransfer(mixin, address, amount, fee, extra, walletInfo);
}

void transfer(std::shared_ptr<WalletInfo> walletInfo)
{
    uint64_t balance = walletInfo->wallet.getActualBalance();

    std::string address = getDestinationAddress();
    uint64_t amount = getTransferAmount();

    if (balance < amount)
    {
        std::cout << WarningMsg("You don't have enough funds to cover this "
                                "transaction!") << std::endl
                  << InformationMsg("Funds needed: " + formatAmount(amount))
                  << std::endl
                  << SuccessMsg("Funds available: " + formatAmount(balance))
                  << std::endl;
        return;
    }

    uint64_t fee = getFee();

    if (balance < amount + fee)
    {
        std::cout << WarningMsg("You don't have enough funds to cover this "
                            "transaction!") << std::endl
                  << InformationMsg("Funds needed: " 
                                  + formatAmount(amount + fee))
                  << std::endl
                  << SuccessMsg("Funds available: " + formatAmount(balance))
                  << std::endl;
        return;
    }

    uint16_t mixin = getMixin();

    std::string extra = getPaymentID();

    doTransfer(mixin, address, amount, fee, extra, walletInfo);
}

void doTransfer(uint16_t mixin, std::string address, uint64_t amount,
                uint64_t fee, std::string extra,
                std::shared_ptr<WalletInfo> walletInfo)
{
    uint64_t balance = walletInfo->wallet.getActualBalance();

    if (balance < amount + fee)
    {
        std::cout << WarningMsg("You don't have enough funds to cover this "
                            "transaction!") << std::endl
                  << InformationMsg("Funds needed: " 
                                  + formatAmount(amount + fee))
                  << std::endl
                  << SuccessMsg("Funds available: " + formatAmount(balance))
                  << std::endl;
        return;
    }

    std::vector<CryptoNote::WalletOrder> transfers;

    CryptoNote::DonationSettings d;
    d.address = "";

    CryptoNote::WalletOrder w;
    w.address = address;
    w.amount = amount;
    transfers.push_back(w);

    CryptoNote::TransactionParameters p;
    p.destinations = transfers;
    p.fee = fee;
    p.mixIn = mixin;
    p.extra = extra;
    p.changeDestination = walletInfo->walletAddress;

    if (!confirmTransaction(p, walletInfo))
    {
        std::cout << WarningMsg("Cancelling transaction.") << std::endl;
        return;
    }

    bool txIsTooLarge;

    try
    {
        txIsTooLarge = walletInfo->wallet.txIsTooLarge(p);

        if (txIsTooLarge)
        {
            fusionTX(walletInfo->wallet, p);
        }
        else
        {
            size_t id = walletInfo->wallet.transfer(p);
            
            CryptoNote::WalletTransaction tx 
                = walletInfo->wallet.getTransaction(id);

            std::cout << SuccessMsg("Transaction has been sent! ID:\n" 
                                  + Common::podToHex(tx.hash)) << std::endl;
        }
    }
    catch (const std::system_error &e)
    {
        std::string errMsg = e.what();
        /* For some reason we are unable to send our full balance when
           I have tested. It looks possible it is due to dust amounts,
           possibly these can't be sent? The relevant code can be found
           in src/Wallet/WalletGreen.cpp in the function
           selectTransfers() */
        if (errMsg == "Not enough money: Wrong amount")
        {
            std::cout << WarningMsg("Failed to send transaction - not enough "
                                "funds!") << std::endl
                      << "You sometimes need to send a small amount less "
                      << "than your full balance to get the transfer to "
                      << "succeed." << std::endl << "This is possibly due to "
                      << "dust in your wallet that is unable to be sent."
                      << std::endl;
        }
        /* The internal node error I believe is caused by the same issue as
           the mixin error. Rocksteady explained this as not enough traffic
           having occured on the network to allow your to mixin with.
           Hopefully, this will only occur on the testnet and not the main
           network. It seems sending multiple smaller transactiosn will
           provide the network with more change to allow tx's to go through.
           However, in some wallets that have only recieved one big single
           transaction, they may be unable to send at all without lowering
           their mixin count to 0 */
        else if (errMsg == "MixIn count is too big"
              || errMsg == "Internal node error")
        {
            std::cout << WarningMsg("Failed to send transaction!") << std::endl
                      << "Unable to find enough outputs to mix with."
                      << std::endl << "Try lowering the amount you are "
                      << "sending in one transaction." << std::endl
                      << "Alternatively, you can try lowering the mixin count "
                      << "to 0, but this will compromise privacy." << std::endl;
        }
        else
        {
            std::cout << WarningMsg("Failed to send transaction!")
                      << std::endl << "Error message: " << errMsg
                      << std::endl;
        }
    }
}

std::string getPaymentID()
{
    while (true)
    {
        std::string paymentID;

        std::cout << std::endl
                  << InformationMsg("What payment ID do you want to use?")
                  << std::endl 
                  << "These are usually used for sending to exchanges."
                  << std::endl
                  << WarningMsg("Warning: if you were given a payment ID, you "
                                "MUST use it, or your funds may be lost!")
                  << std::endl
                  << "Hit enter for the default of no payment ID: ";

        std::getline(std::cin, paymentID);

        if (paymentID == "")
        {
            return paymentID;
        }

        std::vector<uint8_t> extra;

        /* Convert the payment ID into an "extra" */
        if (!CryptoNote::createTxExtraWithPaymentId(paymentID, extra))
        {
            std::cout << WarningMsg("Failed to parse! Payment ID's are 64 "
                                    "character hexadecimal strings.")
                      << std::endl;
        }
        else
        {
            /* Then convert the "extra" back into a string so we can pass
               the argument that walletgreen expects. Note this string is not
               the same as the original paymentID string! */
            std::string extraString;

            for (auto i : extra)
            {
                extraString += static_cast<char>(i);
            }

            return extraString;
        }
    }
}

uint64_t getFee()
{
    while (true)
    {
        std::string stringAmount;
        std::cout << std::endl 
                  << InformationMsg("What fee do you want to use?")
                  << std::endl
                  << "Hit enter for the default fee of 0.1 TRTL: ";

        std::getline(std::cin, stringAmount);

        if (stringAmount == "")
        {
            return CryptoNote::parameters::MINIMUM_FEE;
        }

        uint64_t amount;

        if (parseFee(stringAmount))
        {
            parseAmount(stringAmount, amount);
            return amount;
        }
    }
}

uint16_t getMixin()
{
    while (true)
    {
        std::string stringMixin;
        std::cout << std::endl
                  << InformationMsg("What mixin do you want to use?")
                  << std::endl
                  << "Mixin is how many times your transaction is mixed "
                  << "with others for privacy." << std::endl
                  << "Hit enter for the default mixin of 5: ";

        std::getline(std::cin, stringMixin);

        if (stringMixin == "")
        {
            return CryptoNote::parameters::DEFAULT_MIXIN;
        }
        else if (parseMixin(stringMixin))
        {
            return std::stoi(stringMixin);
        }
    }
}

uint64_t getTransferAmount()
{
    while (true)
    {
        std::string stringAmount;

        std::cout << std::endl
                  << InformationMsg("How much TRTL do you want to send?: ");

        std::getline(std::cin, stringAmount);

        uint64_t amount;

        if (parseAmount(stringAmount))
        {
            parseAmount(stringAmount, amount);
            return amount;
        }
    }
}

std::string getDestinationAddress()
{
    while (true)
    {
        std::string transferAddr;

        std::cout << InformationMsg("What address do you want to "
                                    "transfer to?: ");

        std::getline(std::cin, transferAddr);
        boost::algorithm::trim(transferAddr);

        if (parseAddress(transferAddr))
        {
            return transferAddr;
        }
    }
}

bool parseFee(std::string feeString)
{
    uint64_t fee;

    if (!parseAmount(feeString, fee))
    {
        std::cout << WarningMsg("Failed to parse fee! Ensure you entered the "
                                "value correctly.") << std::endl;
        return false;
    }
    else if (fee < CryptoNote::parameters::MINIMUM_FEE)
    {
        std::cout << WarningMsg("Fee must be at least 0.1 TRTL!") << std::endl;
        return false;
    }

    return true;
}


bool parseAddress(std::string address)
{
    if (address.length() != 99)
    {
        std::cout << WarningMsg("Address is wrong length!") << std::endl
                  << "It should be 99 characters long, but it is "
                  << address.length() << " characters long!" << std::endl;

        return false;
    }
    else if (address.substr(0, 4) != "TRTL")
    {
        std::cout << WarningMsg("Invalid address! It should start with "
                                "\"TRTL\"!") << std::endl;

        return false;
    }

    return true;
}

bool parseMixin(std::string mixinString)
{

    try
    {
        /* We shouldn't need to check this is >0 because it should fail
           to parse as it's a uint16_t? */
        std::stoi(mixinString);
        return true;
    }
    catch (const std::invalid_argument)
    {
        std::cout << WarningMsg("Failed to parse mixin! Ensure you entered the "
                                "value correctly.") << std::endl;
        return false;
    }
}

bool parseAmount(std::string amountString)
{
    uint64_t amount;

    if (!parseAmount(amountString, amount))
    {
        std::cout << WarningMsg("Failed to parse amount! Ensure you entered the "
                                "value correctly.") << std::endl
                  << "Please note, the minimum you can send is 0.01 TRTL, "
                  << "and you can only use 2 decimal places."
                  << std::endl;
        return false;
    }

    return true;
}
