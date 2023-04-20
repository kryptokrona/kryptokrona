// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

///////////////////////////////////
#include <wallet_backend/transfer.h>
///////////////////////////////////

#include <config/wallet_config.h>

#include <cryptonote_core/cryptonote_tools.h>
#include <cryptonote_core/currency.h>
#include <cryptonote_core/mixins.h>
#include <cryptonote_core/transaction_extra.h>

#include <errors/validate_parameters.h>

#include <utilities/addresses.h>
#include <utilities/format_tools.h>
#include <utilities/utilities.h>

#include <wallet_backend/wallet_backend.h>

namespace send_transaction
{

    std::tuple<Error, crypto::Hash> sendFusionTransactionBasic(
        const std::shared_ptr<Nigel> daemon,
        const std::shared_ptr<SubWallets> subWallets)
    {
        const auto [minMixin, maxMixin, defaultMixin] = cryptonote::Mixins::getMixinAllowableRange(
            daemon->networkBlockCount());

        /* Assumes the container has at least one subwallet - this is true as long
           as the static constructors were used */
        const std::string defaultAddress = subWallets->getPrimaryAddress();

        return sendFusionTransactionAdvanced(
            defaultMixin, {}, defaultAddress, daemon, subWallets);
    }

    std::tuple<Error, crypto::Hash> sendFusionTransactionAdvanced(
        const uint64_t mixin,
        const std::vector<std::string> addressesToTakeFrom,
        std::string destination,
        const std::shared_ptr<Nigel> daemon,
        const std::shared_ptr<SubWallets> subWallets)
    {
        if (destination == "")
        {
            destination = subWallets->getPrimaryAddress();
        }

        /* Validate the transaction input parameters */
        Error error = validateFusionTransaction(
            mixin, addressesToTakeFrom, destination, subWallets,
            daemon->networkBlockCount());

        if (error)
        {
            return {error, crypto::Hash()};
        }

        /* If no address to take from is given, we will take from all available. */
        const bool takeFromAllSubWallets = addressesToTakeFrom.empty();

        /* Convert the addresses to public spend keys */
        const std::vector<crypto::PublicKey> subWalletsToTakeFrom = utilities::addressesToSpendKeys(addressesToTakeFrom);

        /* Grab inputs for our fusion transaction */
        auto [ourInputs, maxFusionInputs, foundMoney] = subWallets->getFusionTransactionInputs(
            takeFromAllSubWallets, subWalletsToTakeFrom, mixin,
            daemon->networkBlockCount());

        /* Mixin is too large to get enough outputs whilst remaining in the size
           and ratio constraints */
        if (maxFusionInputs < cryptonote::parameters::FUSION_TX_MIN_INPUT_COUNT)
        {
            return {FUSION_MIXIN_TOO_LARGE, crypto::Hash()};
        }

        /* Payment ID's are not needed with fusion transactions */
        const std::string paymentID = "";

        cryptonote::Transaction tx;

        std::vector<wallet_types::KeyOutput> transactionOutputs;

        cryptonote::KeyPair txKeyPair;

        while (true)
        {
            /* Not got enough unspent inputs for a fusion tx - we're fully optimized. */
            if (ourInputs.size() < cryptonote::parameters::FUSION_TX_MIN_INPUT_COUNT)
            {
                return {FULLY_OPTIMIZED, crypto::Hash()};
            }

            /* Grab the public keys from the receiver address */
            const auto [publicSpendKey, publicViewKey] = utilities::addressToKeys(destination);

            std::vector<wallet_types::TransactionDestination> destinations;

            /* Split transfer into denominations and create an output for each */
            for (const auto denomination : splitAmountIntoDenominations(foundMoney))
            {
                wallet_types::TransactionDestination destination;

                destination.amount = denomination;
                destination.receiverPublicSpendKey = publicSpendKey;
                destination.receiverPublicViewKey = publicViewKey;

                destinations.push_back(destination);
            }

            uint64_t requiredInputOutputRatio = cryptonote::parameters::FUSION_TX_MIN_IN_OUT_COUNT_RATIO;

            /* We need to have at least 4x more inputs than outputs */
            if (destinations.size() == 0 || (ourInputs.size() / destinations.size()) < requiredInputOutputRatio)
            {
                /* Reduce the amount we're sending */
                foundMoney -= ourInputs.back().input.amount;

                /* Remove the last input */
                ourInputs.pop_back();

                /* And try again */
                continue;
            }

            const uint64_t unlockTime = 0;

            TransactionResult txResult = makeTransaction(
                mixin, daemon, ourInputs, paymentID, destinations, subWallets,
                unlockTime);

            tx = txResult.transaction;
            transactionOutputs = txResult.outputs;
            txKeyPair = txResult.txKeyPair;

            if (txResult.error)
            {
                return {txResult.error, crypto::Hash()};
            }

            const uint64_t txSize = cryptonote::toBinaryArray(tx).size();

            /* Transaction is too large, remove an input and try again */
            if (txSize > cryptonote::parameters::FUSION_TX_MAX_SIZE)
            {
                /* Reduce the amount we're sending */
                foundMoney -= ourInputs.back().input.amount;

                /* Remove the last input */
                ourInputs.pop_back();

                /* And try again */
                continue;
            }

            break;
        }

        if (!verifyAmounts(tx))
        {
            return {AMOUNTS_NOT_PRETTY, crypto::Hash()};
        }

        if (!verifyTransactionFee(0, tx))
        {
            return {UNEXPECTED_FEE, crypto::Hash()};
        }

        /* Try and send the transaction */
        const auto [sendError, txHash] = relayTransaction(tx, daemon);

        if (sendError)
        {
            return {sendError, crypto::Hash()};
        }

        /* No fee or change with fusion */
        const uint64_t fee(0), changeRequired(0);

        /* Store the unconfirmed transaction, update our balance */
        storeSentTransaction(
            txHash, fee, paymentID, ourInputs, destination, changeRequired,
            subWallets);

        /* Update our locked balance with the incoming funds */
        storeUnconfirmedIncomingInputs(
            subWallets, transactionOutputs, txKeyPair.publicKey, txHash);

        subWallets->storeTxPrivateKey(txKeyPair.secretKey, txHash);

        /* Lock the input for spending till it is confirmed as spent in a block */
        for (const auto input : ourInputs)
        {
            subWallets->markInputAsLocked(
                input.input.keyImage, input.publicSpendKey);
        }

        return {SUCCESS, txHash};
    }

    /* A basic send transaction, the most common transaction, one destination,
       default fee, default mixin, default change address

       WARNING: This is NOT suitable for multi wallet containers, as the change
       address is undefined - it will return to one of the subwallets, but it
       is not defined which, since they are not ordered by creation time or
       anything like that.

       If you want to return change to a specific wallet, use
       sendTransactionAdvanced() */
    std::tuple<Error, crypto::Hash> sendTransactionBasic(
        std::string destination,
        const uint64_t amount,
        std::string paymentID,
        const std::shared_ptr<Nigel> daemon,
        const std::shared_ptr<SubWallets> subWallets)
    {
        std::vector<std::pair<std::string, uint64_t>> destinations = {
            {destination, amount}};

        const auto [minMixin, maxMixin, defaultMixin] = cryptonote::Mixins::getMixinAllowableRange(
            daemon->networkBlockCount());

        const uint64_t fee = wallet_config::defaultFee;

        /* Assumes the container has at least one subwallet - this is true as long
           as the static constructors were used */
        const std::string changeAddress = subWallets->getPrimaryAddress();

        const uint64_t unlockTime = 0;

        return sendTransactionAdvanced(
            destinations, defaultMixin, fee, paymentID, {}, changeAddress, daemon,
            subWallets, unlockTime);
    }

    std::tuple<Error, crypto::Hash> sendTransactionAdvanced(
        std::vector<std::pair<std::string, uint64_t>> addressesAndAmounts,
        const uint64_t mixin,
        const uint64_t fee,
        std::string paymentID,
        const std::vector<std::string> addressesToTakeFrom,
        std::string changeAddress,
        const std::shared_ptr<Nigel> daemon,
        const std::shared_ptr<SubWallets> subWallets,
        const uint64_t unlockTime)
    {
        /* Append the fee transaction, if a fee is being used */
        const auto [feeAmount, feeAddress] = daemon->nodeFee();

        if (feeAmount != 0)
        {
            addressesAndAmounts.push_back({feeAddress, feeAmount});
        }

        if (changeAddress == "")
        {
            changeAddress = subWallets->getPrimaryAddress();
        }

        /* Validate the transaction input parameters */
        Error error = validateTransaction(
            addressesAndAmounts, mixin, fee, paymentID, addressesToTakeFrom,
            changeAddress, subWallets, daemon->networkBlockCount());

        if (error)
        {
            return {error, crypto::Hash()};
        }

        /* Convert integrated addresses to standard address + paymentID, if
           present. We have already validated they are valid integrated addresses
           in validateTransaction(), and the paymentID's do not conflict. */
        for (auto &[address, amount] : addressesAndAmounts)
        {
            if (address.length() != wallet_config::integratedAddressLength)
            {
                continue;
            }

            auto [extractedAddress, extractedPaymentID] = utilities::extractIntegratedAddressData(address);

            address = extractedAddress;
            paymentID = extractedPaymentID;
        }

        /* If no address to take from is given, we will take from all available. */
        const bool takeFromAllSubWallets = addressesToTakeFrom.empty();

        /* The total amount we are sending */
        const uint64_t totalAmount = utilities::getTransactionSum(addressesAndAmounts) + fee;

        /* Convert the addresses to public spend keys */
        const std::vector<crypto::PublicKey> subWalletsToTakeFrom = utilities::addressesToSpendKeys(addressesToTakeFrom);

        /* The transaction 'inputs' - key images we have previously received, plus
           their sum. The sumOfInputs is sometimes (most of the time) greater than
           the amount we want to send, so we need to send some back to ourselves
           as change. */
        auto [ourInputs, sumOfInputs] = subWallets->getTransactionInputsForAmount(
            totalAmount, takeFromAllSubWallets, subWalletsToTakeFrom,
            daemon->networkBlockCount());

        /* If the sum of inputs is > total amount, we need to send some back to
           ourselves. */
        uint64_t changeRequired = sumOfInputs - totalAmount;

        /* Split the transfers up into an amount, a public spend+view key */
        const auto destinations = setupDestinations(
            addressesAndAmounts, changeRequired, changeAddress);

        TransactionResult txResult = makeTransaction(
            mixin, daemon, ourInputs, paymentID, destinations, subWallets,
            unlockTime);

        if (txResult.error)
        {
            return {txResult.error, crypto::Hash()};
        }

        error = isTransactionPayloadTooBig(
            txResult.transaction, daemon->networkBlockCount());

        if (error)
        {
            return {error, crypto::Hash()};
        }

        if (!verifyAmounts(txResult.transaction))
        {
            return {AMOUNTS_NOT_PRETTY, crypto::Hash()};
        }

        if (!verifyTransactionFee(fee, txResult.transaction))
        {
            return {UNEXPECTED_FEE, crypto::Hash()};
        }

        const auto [sendError, txHash] = relayTransaction(
            txResult.transaction, daemon);

        if (sendError)
        {
            return {sendError, crypto::Hash()};
        }

        /* Store the unconfirmed transaction, update our balance */
        storeSentTransaction(
            txHash, fee, paymentID, ourInputs, changeAddress, changeRequired,
            subWallets);

        /* Update our locked balance with the incoming funds */
        storeUnconfirmedIncomingInputs(
            subWallets, txResult.outputs, txResult.txKeyPair.publicKey, txHash);

        subWallets->storeTxPrivateKey(txResult.txKeyPair.secretKey, txHash);

        /* Lock the input for spending till it is confirmed as spent in a block */
        for (const auto input : ourInputs)
        {
            subWallets->markInputAsLocked(
                input.input.keyImage, input.publicSpendKey);
        }

        return {SUCCESS, txHash};
    }

    Error isTransactionPayloadTooBig(
        const cryptonote::Transaction tx,
        const uint64_t currentHeight)
    {
        const uint64_t txSize = cryptonote::toBinaryArray(tx).size();

        const uint64_t maxTxSize = utilities::getMaxTxSize(currentHeight);

        if (txSize > maxTxSize)
        {
            std::stringstream errorMsg;

            errorMsg << "Transaction is too large: ("
                     << utilities::prettyPrintBytes(txSize)
                     << "). Max allowed size is "
                     << utilities::prettyPrintBytes(maxTxSize)
                     << ". Decrease the amount you are sending, or perform some "
                     << "fusion transactions.";

            return Error(
                TOO_MANY_INPUTS_TO_FIT_IN_BLOCK,
                errorMsg.str());
        }

        return SUCCESS;
    }

    /* Possibly we could abstract some of this from processTransactionOutputs...
       but I think it would make the code harder to follow */
    void storeUnconfirmedIncomingInputs(
        const std::shared_ptr<SubWallets> subWallets,
        const std::vector<wallet_types::KeyOutput> keyOutputs,
        const crypto::PublicKey txPublicKey,
        const crypto::Hash txHash)
    {
        crypto::KeyDerivation derivation;

        crypto::generate_key_derivation(
            txPublicKey, subWallets->getPrivateViewKey(), derivation);

        uint64_t outputIndex = 0;

        for (const auto output : keyOutputs)
        {
            crypto::PublicKey spendKey;

            /* Not our output */
            crypto::underive_public_key(derivation, outputIndex, output.key, spendKey);

            const auto spendKeys = subWallets->m_publicSpendKeys;

            /* See if the derived spend key is one of ours */
            const auto it = std::find(spendKeys.begin(), spendKeys.end(), spendKey);

            if (it != spendKeys.end())
            {
                crypto::PublicKey ourSpendKey = *it;

                wallet_types::UnconfirmedInput input;

                input.amount = keyOutputs[outputIndex].amount;
                input.key = keyOutputs[outputIndex].key;
                input.parentTransactionHash = txHash;

                subWallets->storeUnconfirmedIncomingInput(input, ourSpendKey);
            }

            outputIndex++;
        }
    }

    void storeSentTransaction(
        const crypto::Hash hash,
        const uint64_t fee,
        const std::string paymentID,
        const std::vector<wallet_types::TxInputAndOwner> ourInputs,
        const std::string changeAddress,
        const uint64_t changeRequired,
        const std::shared_ptr<SubWallets> subWallets)
    {
        std::unordered_map<crypto::PublicKey, int64_t> transfers;

        /* Loop through each input, and minus that from the transfers array */
        for (const auto input : ourInputs)
        {
            transfers[input.publicSpendKey] -= input.input.amount;
        }

        /* Grab the subwallet the change address points to */
        const auto [spendKey, viewKey] = utilities::addressToKeys(changeAddress);

        /* Increment the change address with the amount we returned to ourselves */
        if (changeRequired != 0)
        {
            transfers[spendKey] += changeRequired;
        }

        /* Not initialized till it's in a block */
        const uint64_t timestamp(0), blockHeight(0), unlockTime(0);

        const bool isCoinbaseTransaction = false;

        /* Create the unconfirmed transaction (Will be overwritten by the
           confirmed transaction later) */
        wallet_types::Transaction tx(
            transfers, hash, fee, timestamp, blockHeight, paymentID,
            unlockTime, isCoinbaseTransaction);

        subWallets->addUnconfirmedTransaction(tx);
    }

    std::tuple<Error, crypto::Hash> relayTransaction(
        const cryptonote::Transaction tx,
        const std::shared_ptr<Nigel> daemon)
    {
        const auto [success, connectionError] = daemon->sendTransaction(tx);

        if (connectionError)
        {
            return {DAEMON_OFFLINE, crypto::Hash()};
        }

        if (!success)
        {
            return {DAEMON_ERROR, crypto::Hash()};
        }

        return {SUCCESS, getTransactionHash(tx)};
    }

    std::vector<wallet_types::TransactionDestination> setupDestinations(
        std::vector<std::pair<std::string, uint64_t>> addressesAndAmounts,
        const uint64_t changeRequired,
        const std::string changeAddress)
    {
        /* Need to send change back to our own address */
        if (changeRequired != 0)
        {
            addressesAndAmounts.push_back({changeAddress, changeRequired});
        }

        std::vector<wallet_types::TransactionDestination> destinations;

        for (const auto [address, amount] : addressesAndAmounts)
        {
            /* Grab the public keys from the receiver address */
            const auto [publicSpendKey, publicViewKey] = utilities::addressToKeys(address);

            /* Split transfer into denominations and create an output for each */
            for (const auto denomination : splitAmountIntoDenominations(amount))
            {
                wallet_types::TransactionDestination destination;

                destination.amount = denomination;
                destination.receiverPublicSpendKey = publicSpendKey;
                destination.receiverPublicViewKey = publicViewKey;

                destinations.push_back(destination);
            }
        }

        return destinations;
    }

    std::tuple<Error, std::vector<cryptonote::RandomOuts>> getRingParticipants(
        const uint64_t mixin,
        const std::shared_ptr<Nigel> daemon,
        const std::vector<wallet_types::TxInputAndOwner> sources)
    {
        /* Request one more than our mixin, then if we get our output as one of
           the mixin outs, we can skip it and still form the transaction */
        uint64_t requestedOuts = mixin + 1;

        if (mixin == 0)
        {
            return {SUCCESS, {}};
        }

        std::vector<uint64_t> amounts;

        /* Add each amount to the request vector */
        std::transform(sources.begin(), sources.end(), std::back_inserter(amounts), [](const auto destination)
                       { return destination.input.amount; });

        const auto [success, fakeOuts] = daemon->getRandomOutsByAmounts(amounts, requestedOuts);

        if (!success)
        {
            return {DAEMON_OFFLINE, fakeOuts};
        }

        /* Verify outputs are sufficient */
        for (const uint64_t amount : amounts)
        {
            const auto it = std::find_if(fakeOuts.begin(), fakeOuts.end(),
                                         [amount](const auto output)
                                         {
                                             return output.amount == amount;
                                         });

            /* Item is not present at all */
            if (it == fakeOuts.end())
            {
                std::stringstream error;

                error << "Failed to get any matching outputs for amount "
                      << amount << " (" << utilities::formatAmount(amount)
                      << "). Further explanation here: "
                      << "https://gist.github.com/zpalmtree/80b3e80463225bcfb8f8432043cb594c";

                return {Error(NOT_ENOUGH_FAKE_OUTPUTS, error.str()), fakeOuts};
            }

            /* Check we have at least mixin outputs for each fake out. We *may* need
               mixin+1 outs for some, in case our real output gets included. This is
               unlikely though, and so we will error out down the line instead of here. */
            if (it->outs.size() < mixin)
            {
                std::stringstream error;

                error << "Failed to get enough matching outputs for amount "
                      << amount << " (" << utilities::formatAmount(amount)
                      << "). Requested outputs: " << requestedOuts
                      << ", found outputs: " << it->outs.size()
                      << ". Further explanation here: "
                      << "https://gist.github.com/zpalmtree/80b3e80463225bcfb8f8432043cb594c";

                return {Error(NOT_ENOUGH_FAKE_OUTPUTS, error.str()), fakeOuts};
            }
        }

        if (fakeOuts.size() != amounts.size())
        {
            return {NOT_ENOUGH_FAKE_OUTPUTS, fakeOuts};
        }

        for (auto fakeOut : fakeOuts)
        {
            /* Do the same check as above here, again. The reason being that
               we just find the first set of outputs matching the amount above,
               and if we requests, say, outputs for the amount 100 twice, the
               first set might be sufficient, but the second are not.

               We could just check here instead of checking above, but then we
               might hit the length message first. Checking this way gives more
               informative errors. */
            if (fakeOut.outs.size() < mixin)
            {
                std::stringstream error;

                error << "Failed to get enough matching outputs for amount "
                      << fakeOut.amount << " (" << utilities::formatAmount(fakeOut.amount)
                      << "). Requested outputs: " << requestedOuts
                      << ", found outputs: " << fakeOut.outs.size()
                      << ". Further explanation here: "
                      << "https://gist.github.com/zpalmtree/80b3e80463225bcfb8f8432043cb594c";

                return {Error(NOT_ENOUGH_FAKE_OUTPUTS, error.str()), fakeOuts};
            }

            /* Sort the fake outputs by their indexes (don't want there to be an
               easy way to determine which output is the real one) */
            std::sort(fakeOut.outs.begin(), fakeOut.outs.end(), [](const auto &lhs, const auto &rhs)
                      { return lhs.global_amount_index < rhs.global_amount_index; });
        }

        return {SUCCESS, fakeOuts};
    }

    /* Take our inputs and pad them with fake inputs, based on our mixin value */
    std::tuple<Error, std::vector<wallet_types::ObscuredInput>> prepareRingParticipants(
        std::vector<wallet_types::TxInputAndOwner> sources,
        const uint64_t mixin,
        const std::shared_ptr<Nigel> daemon)
    {
        /* Sort our inputs by amount so they match up with the values we get
           back from the daemon */
        std::sort(sources.begin(), sources.end(), [](const auto &lhs, const auto &rhs)
                  { return lhs.input.amount < rhs.input.amount; });

        std::vector<wallet_types::ObscuredInput> result;

        const auto [error, fakeOuts] = getRingParticipants(mixin, daemon, sources);

        if (error)
        {
            return {error, result};
        }

        size_t i = 0;

        for (const auto walletAmount : sources)
        {
            wallet_types::GlobalIndexKey realOutput{
                walletAmount.input.globalOutputIndex.value(),
                walletAmount.input.key};

            wallet_types::ObscuredInput obscuredInput;

            /* The real public key of the transaction */
            obscuredInput.realTransactionPublicKey = walletAmount.input.transactionPublicKey;

            /* The real index of the transaction output index */
            obscuredInput.realOutputTransactionIndex = walletAmount.input.transactionIndex;

            /* The amount of the transaction */
            obscuredInput.amount = walletAmount.input.amount;

            obscuredInput.ownerPublicSpendKey = walletAmount.publicSpendKey;

            obscuredInput.ownerPrivateSpendKey = walletAmount.privateSpendKey;

            if (mixin != 0)
            {
                /* Add the fake outputs to the transaction */
                for (const auto fakeOut : fakeOuts[i].outs)
                {
                    /* This fake output is our output! Skip. */
                    if (walletAmount.input.globalOutputIndex == fakeOut.global_amount_index)
                    {
                        continue;
                    }

                    /* Add the fake output */
                    obscuredInput.outputs.push_back({fakeOut.global_amount_index, fakeOut.out_key});

                    /* Found enough fake outputs, we're done */
                    if (obscuredInput.outputs.size() >= mixin)
                    {
                        break;
                    }
                }
            }

            /* Didn't get enough fake outs to meet the mixin criteria */
            if (obscuredInput.outputs.size() < mixin)
            {
                std::stringstream error;

                error << "Failed to get enough matching outputs for amount "
                      << walletAmount.input.amount << " ("
                      << utilities::formatAmount(walletAmount.input.amount)
                      << "). Requested outputs: " << mixin
                      << ", found outputs: " << obscuredInput.outputs.size()
                      << ". Further explanation here: "
                      << "https://gist.github.com/zpalmtree/80b3e80463225bcfb8f8432043cb594c";

                return {Error(NOT_ENOUGH_FAKE_OUTPUTS, error.str()), result};
            }

            /* Find the position where our real input belongs, e.g. if the fake
               outputs have globalOutputIndexes of [1, 3, 5, 7], and our input
               has a globalOutputIndex of 6, we would place it like so:
               [1, 3, 5, 6, 7]. */
            const auto insertPosition = std::find_if(obscuredInput.outputs.begin(), obscuredInput.outputs.end(),
                                                     [&walletAmount](const auto output)
                                                     {
                                                         return output.index >= walletAmount.input.globalOutputIndex;
                                                     });

            /* Insert our real output among the fakes */
            auto newPosition = obscuredInput.outputs.insert(insertPosition, realOutput);

            /* Indicate which of the outputs is the real one, e.g. number 4 */
            obscuredInput.realOutput = newPosition - obscuredInput.outputs.begin();

            result.push_back(obscuredInput);

            i++;
        }

        return {SUCCESS, result};
    }

    std::tuple<cryptonote::KeyPair, crypto::KeyImage> genKeyImage(
        const wallet_types::ObscuredInput input,
        const crypto::SecretKey privateViewKey)
    {
        crypto::KeyDerivation derivation;

        /* Derive the key from the transaction public key, and our private
           view key */
        crypto::generate_key_derivation(
            input.realTransactionPublicKey, privateViewKey, derivation);

        cryptonote::KeyPair tmpKeyPair;

        /* Derive the public key of the tmp key pair */
        crypto::derive_public_key(
            derivation, input.realOutputTransactionIndex, input.ownerPublicSpendKey,
            tmpKeyPair.publicKey);

        /* Derive the secret key of the tmp key pair */
        crypto::derive_secret_key(
            derivation, input.realOutputTransactionIndex, input.ownerPrivateSpendKey,
            tmpKeyPair.secretKey);

        crypto::KeyImage keyImage;

        /* Generate the key image */
        crypto::generate_key_image(
            tmpKeyPair.publicKey, tmpKeyPair.secretKey, keyImage);

        return {tmpKeyPair, keyImage};
    }

    std::tuple<Error, std::vector<cryptonote::KeyInput>, std::vector<crypto::SecretKey>> setupInputs(
        const std::vector<wallet_types::ObscuredInput> inputsAndFakes,
        const crypto::SecretKey privateViewKey)
    {
        std::vector<cryptonote::KeyInput> inputs;

        std::vector<crypto::SecretKey> tmpSecretKeys;

        for (const auto input : inputsAndFakes)
        {
            const auto [tmpKeyPair, keyImage] = genKeyImage(input, privateViewKey);

            if (tmpKeyPair.publicKey != input.outputs[input.realOutput].key)
            {
                return {INVALID_GENERATED_KEYIMAGE, inputs, tmpSecretKeys};
            }

            tmpSecretKeys.push_back(tmpKeyPair.secretKey);

            cryptonote::KeyInput keyInput;

            keyInput.amount = input.amount;
            keyInput.keyImage = keyImage;

            /* Add each output index from the fake outs */
            std::transform(input.outputs.begin(), input.outputs.end(), std::back_inserter(keyInput.outputIndexes),
                           [](const auto output)
                           {
                               return static_cast<uint32_t>(output.index);
                           });

            /* Convert our indexes to relative indexes - for example, if we
               originally had [5, 10, 20, 21, 22], this would become
               [5, 5, 10, 1, 1]. Due to this, the indexes MUST be sorted - they
               are serialized as a uint32_t, so negative values will overflow! */
            keyInput.outputIndexes = cryptonote::absolute_output_offsets_to_relative(keyInput.outputIndexes);

            /* Store the key input */
            inputs.push_back(keyInput);
        }

        return {SUCCESS, inputs, tmpSecretKeys};
    }

    std::tuple<std::vector<wallet_types::KeyOutput>, cryptonote::KeyPair> setupOutputs(
        std::vector<wallet_types::TransactionDestination> destinations)
    {
        /* Sort the destinations by amount. Helps obscure which output belongs to
           which transaction */
        std::sort(destinations.begin(), destinations.end(), [](const auto &lhs, const auto &rhs)
                  { return lhs.amount < rhs.amount; });

        /* Generate a random key pair for the transaction - public key gets added
           to tx extra */
        cryptonote::KeyPair randomTxKey = cryptonote::generateKeyPair();

        /* Index of the output */
        uint32_t outputIndex = 0;

        std::vector<wallet_types::KeyOutput> outputs;

        for (const auto destination : destinations)
        {
            crypto::KeyDerivation derivation;

            /* Generate derivation from receiver public view key and random tx key */
            crypto::generate_key_derivation(
                destination.receiverPublicViewKey, randomTxKey.secretKey, derivation);

            crypto::PublicKey tmpPubKey;

            /* Derive the temporary public key */
            crypto::derive_public_key(
                derivation, outputIndex, destination.receiverPublicSpendKey, tmpPubKey);

            wallet_types::KeyOutput keyOutput;

            keyOutput.key = tmpPubKey;
            keyOutput.amount = destination.amount;

            outputs.push_back(keyOutput);

            outputIndex++;
        }

        return {outputs, randomTxKey};
    }

    std::tuple<Error, cryptonote::Transaction> generateRingSignatures(
        cryptonote::Transaction tx,
        const std::vector<wallet_types::ObscuredInput> inputsAndFakes,
        const std::vector<crypto::SecretKey> tmpSecretKeys)
    {
        crypto::Hash txPrefixHash;

        /* Hash the transaction prefix (Prefix is just a subset of transaction, so
           we can just do a cast here) */
        cryptonote::getObjectHash(
            static_cast<cryptonote::TransactionPrefix>(tx), txPrefixHash);

        size_t i = 0;

        /* Add the transaction signatures */
        for (const auto input : inputsAndFakes)
        {
            std::vector<crypto::PublicKey> publicKeys;

            /* Add all the fake outs public keys to a vector */
            for (const auto output : input.outputs)
            {
                publicKeys.push_back(output.key);
            }

            /* Generate the ring signatures - note - modifying the transaction
               post signature generation will invalidate the signatures. */
            const auto [success, signatures] = crypto::crypto_ops::generateRingSignatures(
                txPrefixHash, boost::get<cryptonote::KeyInput>(tx.inputs[i]).keyImage,
                publicKeys, tmpSecretKeys[i], input.realOutput);

            if (!success)
            {
                return {FAILED_TO_CREATE_RING_SIGNATURE, tx};
            }

            /* Add the signatures to the transaction */
            tx.signatures.push_back(signatures);

            i++;
        }

        i = 0;

        for (const auto input : inputsAndFakes)
        {
            std::vector<crypto::PublicKey> publicKeys;

            for (const auto output : input.outputs)
            {
                publicKeys.push_back(output.key);
            }

            if (!crypto::crypto_ops::checkRingSignature(
                    txPrefixHash,
                    boost::get<cryptonote::KeyInput>(tx.inputs[i]).keyImage,
                    publicKeys,
                    tx.signatures[i]))
            {
                return {FAILED_TO_CREATE_RING_SIGNATURE, tx};
            }

            i++;
        }

        return {SUCCESS, tx};
    }

    /* Split each amount into uniform amounts, e.g.
       1234567 = 1000000 + 200000 + 30000 + 4000 + 500 + 60 + 7 */
    std::vector<uint64_t> splitAmountIntoDenominations(uint64_t amount)
    {
        std::vector<uint64_t> splitAmounts;

        uint64_t multiplier = 1;

        while (amount > 0)
        {
            uint64_t denomination = multiplier * (amount % 10);

            /* If we have for example, 1010 - we want 1000 + 10,
               not 1000 + 0 + 10 + 0 */
            if (denomination != 0)
            {
                splitAmounts.push_back(denomination);
            }

            amount /= 10;

            multiplier *= 10;
        }

        return splitAmounts;
    }

    std::vector<cryptonote::TransactionInput> keyInputToTransactionInput(
        const std::vector<cryptonote::KeyInput> keyInputs)
    {
        std::vector<cryptonote::TransactionInput> result;

        for (const auto input : keyInputs)
        {
            result.push_back(input);
        }

        return result;
    }

    std::vector<cryptonote::TransactionOutput> keyOutputToTransactionOutput(
        const std::vector<wallet_types::KeyOutput> keyOutputs)
    {
        std::vector<cryptonote::TransactionOutput> result;

        for (const auto output : keyOutputs)
        {
            cryptonote::TransactionOutput tmpOutput;

            tmpOutput.amount = output.amount;

            cryptonote::KeyOutput tmpKey;

            tmpKey.key = output.key;

            tmpOutput.target = tmpKey;

            result.push_back(tmpOutput);
        }

        return result;
    }

    crypto::Hash getTransactionHash(cryptonote::Transaction tx)
    {
        std::vector<uint8_t> data = cryptonote::toBinaryArray(tx);
        return crypto::cn_fast_hash(data.data(), data.size());
    }

    TransactionResult makeTransaction(
        const uint64_t mixin,
        const std::shared_ptr<Nigel> daemon,
        const std::vector<wallet_types::TxInputAndOwner> ourInputs,
        const std::string paymentID,
        const std::vector<wallet_types::TransactionDestination> destinations,
        const std::shared_ptr<SubWallets> subWallets,
        const uint64_t unlockTime)
    {
        /* Mix our inputs with fake ones from the network to hide who we are */
        const auto [mixinError, inputsAndFakes] = prepareRingParticipants(
            ourInputs, mixin, daemon);

        TransactionResult result;

        if (mixinError)
        {
            result.error = mixinError;
            return result;
        }

        /* Setup the transaction inputs */
        const auto [inputError, transactionInputs, tmpSecretKeys] = setupInputs(
            inputsAndFakes, subWallets->getPrivateViewKey());

        if (inputError)
        {
            result.error = inputError;
            return result;
        }

        /* Setup the transaction outputs */
        std::tie(result.outputs, result.txKeyPair) = setupOutputs(destinations);

        std::vector<uint8_t> extra;

        if (paymentID != "")
        {
            cryptonote::createTxExtraWithPaymentId(paymentID, extra);
        }

        /* Append the transaction public key we generated earlier to the extra
           data */
        cryptonote::addTransactionPublicKeyToExtra(extra, result.txKeyPair.publicKey);

        cryptonote::Transaction setupTX;

        setupTX.version = cryptonote::CURRENT_TRANSACTION_VERSION;

        setupTX.unlockTime = unlockTime;

        /* Convert from key inputs to the boost uglyness */
        setupTX.inputs = keyInputToTransactionInput(transactionInputs);

        /* We can't really remove boost from here yet and simplify our data types
           since we take a hash of the transaction prefix. Once we've got this
           working, maybe we can work some magic. TODO */
        setupTX.outputs = keyOutputToTransactionOutput(result.outputs);

        /* Pubkey, payment ID */
        setupTX.extra = extra;

        /* Fill in the transaction signatures */
        /* NOTE: Do not modify the transaction after this, or the ring signatures
           will be invalidated */
        std::tie(result.error, result.transaction) = generateRingSignatures(
            setupTX, inputsAndFakes, tmpSecretKeys);

        return result;
    }

    bool verifyAmounts(const cryptonote::Transaction tx)
    {
        std::vector<uint64_t> amounts;

        /* Note - not verifying inputs as it's possible to have received inputs
           from another wallet which don't enforce this rule */
        for (const auto &output : tx.outputs)
        {
            amounts.push_back(output.amount);
        }

        return verifyAmounts(amounts);
    }

    /* Verify all amounts are valid amounts to send - that they are in PRETTY_AMOUNTS */
    bool verifyAmounts(const std::vector<uint64_t> amounts)
    {
        /* yeah... i don't want to type that every time */
        const auto prettyAmounts = cryptonote::Currency::PRETTY_AMOUNTS;

        for (const auto amount : amounts)
        {
            if (std::find(prettyAmounts.begin(), prettyAmounts.end(), amount) == prettyAmounts.end())
            {
                return false;
            }
        }

        return true;
    }

    bool verifyTransactionFee(const uint64_t expectedFee, const cryptonote::Transaction tx)
    {
        uint64_t inputTotal = 0;
        uint64_t outputTotal = 0;

        for (const auto input : tx.inputs)
        {
            inputTotal += boost::get<cryptonote::KeyInput>(input).amount;
        }

        for (const auto output : tx.outputs)
        {
            outputTotal += output.amount;
        }

        uint64_t actualFee = inputTotal - outputTotal;

        return expectedFee == actualFee;
    }

} // namespace send_transaction
