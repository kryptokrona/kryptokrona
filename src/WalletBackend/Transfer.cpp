// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

///////////////////////////////////
#include <WalletBackend/Transfer.h>
///////////////////////////////////

#include <config/WalletConfig.h>

#include <CryptoNoteCore/CryptoNoteTools.h>
#include <CryptoNoteCore/Mixins.h>
#include <CryptoNoteCore/TransactionExtra.h>

#include <future>

#include <NodeRpcProxy/NodeErrors.h>

#include <WalletBackend/Utilities.h>
#include <WalletBackend/ValidateParameters.h>
#include <WalletBackend/WalletBackend.h>

namespace SendTransaction
{

/* A basic send transaction, the most common transaction, one destination,
   default fee, default mixin, default change address
   
   WARNING: This is NOT suitable for multi wallet containers, as the change
   address is undefined - it will return to one of the subwallets, but it
   is not defined which, since they are not ordered by creation time or
   anything like that.
   
   If you want to return change to a specific wallet, use
   sendTransactionAdvanced() */
std::tuple<WalletError, Crypto::Hash> sendTransactionBasic(
    std::string destination,
    const uint64_t amount,
    std::string paymentID,
    const std::shared_ptr<CryptoNote::NodeRpcProxy> daemon,
    const std::shared_ptr<SubWallets> subWallets)
{
    std::vector<std::pair<std::string, uint64_t>> destinations = {
        {destination, amount}
    };

    const uint64_t mixin = CryptoNote::Mixins::getDefaultMixin(
        daemon->getLastKnownBlockHeight()
    );

    const uint64_t fee = WalletConfig::defaultFee;

    /* Assumes the container has at least one subwallet - this is true as long
       as the static constructors were used */
    const std::string changeAddress = subWallets->getDefaultChangeAddress();

    return sendTransactionAdvanced(
        destinations, mixin, fee, paymentID, {}, changeAddress, daemon,
        subWallets
    );
}

std::tuple<WalletError, Crypto::Hash> sendTransactionAdvanced(
    std::vector<std::pair<std::string, uint64_t>> addressesAndAmounts,
    const uint64_t mixin,
    const uint64_t fee,
    std::string paymentID,
    const std::vector<std::string> addressesToTakeFrom,
    std::string changeAddress,
    const std::shared_ptr<CryptoNote::NodeRpcProxy> daemon,
    const std::shared_ptr<SubWallets> subWallets)
{
    if (changeAddress == "")
    {
        changeAddress = subWallets->getDefaultChangeAddress();
    }

    /* Validate the transaction input parameters */
    WalletError error = validateTransaction(
        addressesAndAmounts, mixin, fee, paymentID, addressesToTakeFrom,
        changeAddress, subWallets, daemon->getLastKnownBlockHeight()
    );

    if (error)
    {
        return {error, Crypto::Hash()};
    }

    /* Convert integrated addresses to standard address + paymentID, if
       present. We have already validated they are valid integrated addresses
       in validateTransaction(), and the paymentID's do not conflict. */
    for (auto &[address, amount] : addressesAndAmounts)
    {
        if (address.length() != WalletConfig::integratedAddressLength)
        {
            continue;
        }

        auto [extractedAddress, extractedPaymentID]
            = Utilities::extractIntegratedAddressData(address);

        address = extractedAddress;
        paymentID = extractedPaymentID;
    }

    /* If no address to take from is given, we will take from all available. */
    const bool takeFromAllSubWallets = addressesToTakeFrom.empty();

    /* The total amount we are sending */
    const uint64_t totalAmount = Utilities::getTransactionSum(addressesAndAmounts) + fee;

    /* Convert the addresses to public spend keys */
    const std::vector<Crypto::PublicKey> subWalletsToTakeFrom
        = Utilities::addressesToSpendKeys(addressesToTakeFrom);

    /* The transaction 'inputs' - key images we have previously received, plus
       their sum. The sumOfInputs is sometimes (most of the time) greater than
       the amount we want to send, so we need to send some back to ourselves
       as change. */
    auto [ourInputs, sumOfInputs] = subWallets->getTransactionInputsForAmount(
        totalAmount, takeFromAllSubWallets, subWalletsToTakeFrom
    );

    /* If the sum of inputs is > total amount, we need to send some back to
       ourselves. */
    uint64_t changeRequired = sumOfInputs - totalAmount;

    /* Split the transfers up into an amount, a public spend+view key */
    const auto destinations = setupDestinations(
        addressesAndAmounts, changeRequired, changeAddress
    );

    const auto [creationError, tx] = makeTransaction(
        mixin, daemon, ourInputs, paymentID, destinations, subWallets
    );

    error = isTransactionTooBig(tx, daemon->getLastKnownBlockHeight());

    if (error)
    {
        return {error, Crypto::Hash()};
    }

    const auto [sendError, txHash] = relayTransaction(tx, daemon);

    if (sendError)
    {
        return {sendError, Crypto::Hash()};
    }

    /* Store the unconfirmed transaction, update our balance */
    storeSentTransaction(
        txHash, fee, paymentID, ourInputs, changeAddress, changeRequired,
        subWallets
    );

    /* Lock the input for spending till it is confirmed as spent in a block */
    for (const auto input : ourInputs)
    {
        subWallets->markInputAsLocked(
            input.input.keyImage, input.publicSpendKey
        );
    }

    return {SUCCESS, txHash};
}

WalletError isTransactionTooBig(
    const CryptoNote::Transaction tx,
    const uint64_t currentHeight)
{
    const uint64_t txSize = CryptoNote::toBinaryArray(tx).size();

    const uint64_t maxTxSize = Utilities::getMaxTxSize(currentHeight);

    if (txSize > maxTxSize)
    {
        std::stringstream errorMsg; 

        errorMsg << "Transaction is too large: (" 
                 << Utilities::prettyPrintBytes(txSize)
                 << "). Max allowed size is "
                 << Utilities::prettyPrintBytes(maxTxSize)
                 << ". Decrease the amount you are sending, or perform some "
                 << "fusion transactions.";

        return WalletError(
            TOO_MANY_INPUTS_TO_FIT_IN_BLOCK,
            errorMsg.str()
        );
    }

    return SUCCESS;
}

void storeSentTransaction(
    const Crypto::Hash hash,
    const uint64_t fee,
    const std::string paymentID,
    const std::vector<WalletTypes::TxInputAndOwner> ourInputs,
    const std::string changeAddress,
    const uint64_t changeRequired,
    const std::shared_ptr<SubWallets> subWallets)
{
    std::unordered_map<Crypto::PublicKey, int64_t> transfers;

    /* Loop through each input, and minus that from the transfers array */
    for (const auto input : ourInputs)
    {
        transfers[input.publicSpendKey] -= input.input.amount;
    }

    /* Grab the subwallet the change address points to */
    const auto [spendKey, viewKey] = Utilities::addressToKeys(changeAddress);

    /* Increment the change address with the amount we returned to ourselves */
    transfers[spendKey] += changeRequired;

    /* Not initialized till it's in a block */
    const uint64_t timestamp(0), blockHeight(0), unlockTime(0);

    const bool isConfirmed = false;

    /* Create the unconfirmed transaction (Will be overwritten by the
       confirmed transaction later) */
    WalletTypes::Transaction tx(
        transfers, hash, fee, timestamp, blockHeight, paymentID, isConfirmed,
        unlockTime
    );

    subWallets->addTransaction(tx);
}

std::tuple<WalletError, Crypto::Hash> relayTransaction(
    const CryptoNote::Transaction tx,
    const std::shared_ptr<CryptoNote::NodeRpcProxy> daemon)
{
    std::promise<std::error_code> errorPromise = std::promise<std::error_code>();

    auto callback = [&errorPromise](auto e) { errorPromise.set_value(e); };

    daemon->relayTransaction(tx, callback);

    auto error = errorPromise.get_future().get();

    if (error)
    {
        if (error == make_error_code(CryptoNote::NodeError::CONNECT_ERROR) ||
            error == make_error_code(CryptoNote::NodeError::NETWORK_ERROR) ||
            error == make_error_code(CryptoNote::NodeError::NODE_BUSY))
        {
            return {DAEMON_OFFLINE, Crypto::Hash()};
        }

        return {DAEMON_ERROR, Crypto::Hash()};
    }

    return {SUCCESS, getTransactionHash(tx)};
}

std::vector<WalletTypes::TransactionDestination> setupDestinations(
    std::vector<std::pair<std::string, uint64_t>> addressesAndAmounts,
    const uint64_t changeRequired,
    const std::string changeAddress)
{
    /* Need to send change back to our own address */
    if (changeRequired != 0)
    {
        addressesAndAmounts.push_back({changeAddress, changeRequired});
    }

    std::vector<WalletTypes::TransactionDestination> destinations;

    for (const auto [address, amount] : addressesAndAmounts)
    {
        /* Grab the public keys from the receiver address */
        const auto [publicSpendKey, publicViewKey] = Utilities::addressToKeys(address);

        /* Split transfer into denominations and create an output for each */
        for (const auto denomination : splitAmountIntoDenominations(amount))
        {
            WalletTypes::TransactionDestination destination;

            destination.amount = denomination;
            destination.receiverPublicSpendKey = publicSpendKey;
            destination.receiverPublicViewKey = publicViewKey;

            destinations.push_back(destination);
        }
    }

    return destinations;
}

std::tuple<WalletError, std::vector<CryptoNote::RandomOuts>> getFakeOuts(
    const uint64_t mixin,
    const std::shared_ptr<CryptoNote::NodeRpcProxy> daemon,
    const std::vector<WalletTypes::TxInputAndOwner> sources)
{
    /* Request one more than our mixin, then if we get our output as one of
       the mixin outs, we can skip it and still form the transaction */
    uint64_t requestedOuts = mixin + 1;

    std::vector<CryptoNote::RandomOuts> fakeOuts;

    if (mixin == 0)
    {
        return {SUCCESS, fakeOuts};
    }

    std::promise<std::error_code> errorPromise = std::promise<std::error_code>();

    auto callback = [&errorPromise](auto e) { errorPromise.set_value(e); };

    std::vector<uint64_t> amounts;

    /* Add each amount to the request vector */
    std::transform(sources.begin(), sources.end(), std::back_inserter(amounts), [](const auto destination)
    {
        return destination.input.amount;
    });

    daemon->getRandomOutsByAmounts(
        std::move(amounts), requestedOuts, fakeOuts, callback
    );

    /* Wait for the call to complete */
    if (errorPromise.get_future().get())
    {
        return {CANT_GET_FAKE_OUTPUTS, fakeOuts};
    }

    /* Check we have at least mixin outputs for each fake out. We *may* need
       mixin+1 outs for some, in case our real output gets included. This is
       unlikely though, and so we will error out down the line instead of here. */
    bool enoughOuts = std::all_of(fakeOuts.begin(), fakeOuts.end(), [&mixin](const auto fakeOut)
    {
        return fakeOut.outs.size() >= mixin;
    });

    if (!enoughOuts)
    {
        return {NOT_ENOUGH_FAKE_OUTPUTS, fakeOuts};
    }

    for (auto fakeOut : fakeOuts)
    {
        /* Sort the fake outputs by their indexes (don't want there to be an
           easy way to determine which output is the real one) */
        std::sort(fakeOut.outs.begin(), fakeOut.outs.end(), [](const auto &lhs, const auto &rhs)
        {
            return lhs.global_amount_index < rhs.global_amount_index;
        });
    }

    return {SUCCESS, fakeOuts};
}

/* Take our inputs and pad them with fake inputs, based on our mixin value */
std::tuple<WalletError, std::vector<WalletTypes::ObscuredInput>> setupFakeInputs(
    std::vector<WalletTypes::TxInputAndOwner> sources,
    const uint64_t mixin,
    const std::shared_ptr<CryptoNote::NodeRpcProxy> daemon)
{
    /* Sort our inputs by amount so they match up with the values we get
       back from the daemon */
    std::sort(sources.begin(), sources.end(), [](const auto &lhs, const auto &rhs)
    {
        return lhs.input.amount < rhs.input.amount;
    });

    std::vector<WalletTypes::ObscuredInput> result;

    const auto [error, fakeOuts] = getFakeOuts(mixin, daemon, sources);

    if (error)
    {
        return {error, result};
    }

    size_t i = 0;

    for (const auto walletAmount : sources)
    {
        WalletTypes::GlobalIndexToKey realOutput {
            walletAmount.input.globalOutputIndex,
            walletAmount.input.key
        };

        WalletTypes::ObscuredInput obscuredInput;

        /* The real public key of the transaction */
        obscuredInput.realTransactionPublicKey = walletAmount.input.transactionPublicKey;

        /* The real index of the transaction output index */
        obscuredInput.realOutputTransactionIndex = walletAmount.input.transactionIndex;

        /* The amount of the transaction */
        obscuredInput.amount = walletAmount.input.amount;

        obscuredInput.ownerPublicSpendKey = walletAmount.publicSpendKey;

        obscuredInput.ownerPrivateSpendKey = walletAmount.privateSpendKey;

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

        /* Didn't get enough fake outs to meet the mixin criteria */
        if (obscuredInput.outputs.size() < mixin)
        {
            return {NOT_ENOUGH_FAKE_OUTPUTS, result};
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

std::tuple<CryptoNote::KeyPair, Crypto::KeyImage> genKeyImage(
    const WalletTypes::ObscuredInput input,
    const Crypto::SecretKey privateViewKey)
{
    Crypto::KeyDerivation derivation;

    /* Derive the key from the transaction public key, and our private
       view key */
    Crypto::generate_key_derivation(
        input.realTransactionPublicKey, privateViewKey, derivation
    );

    CryptoNote::KeyPair tmpKeyPair;

    /* Derive the public key of the tmp key pair */
    Crypto::derive_public_key(
        derivation, input.realOutputTransactionIndex, input.ownerPublicSpendKey,
        tmpKeyPair.publicKey
    );

    /* Derive the secret key of the tmp key pair */
    Crypto::derive_secret_key(
        derivation, input.realOutputTransactionIndex, input.ownerPrivateSpendKey,
        tmpKeyPair.secretKey
    );

    Crypto::KeyImage keyImage;

    /* Generate the key image */
    Crypto::generate_key_image(
        tmpKeyPair.publicKey, tmpKeyPair.secretKey, keyImage
    );

    return {tmpKeyPair, keyImage};
}

std::tuple<WalletError, std::vector<CryptoNote::KeyInput>, std::vector<Crypto::SecretKey>> setupInputs(
    const std::vector<WalletTypes::ObscuredInput> inputsAndFakes,
    const Crypto::SecretKey privateViewKey)
{
    std::vector<CryptoNote::KeyInput> inputs;

    std::vector<Crypto::SecretKey> tmpSecretKeys;

    for (const auto input : inputsAndFakes)
    {
        const auto [tmpKeyPair, keyImage] = genKeyImage(input, privateViewKey);

        if (tmpKeyPair.publicKey != input.outputs[input.realOutput].key)
        {
            return {INVALID_GENERATED_KEYIMAGE, inputs, tmpSecretKeys};
        }

        tmpSecretKeys.push_back(tmpKeyPair.secretKey);

        CryptoNote::KeyInput keyInput;

        keyInput.amount = input.amount;
        keyInput.keyImage = keyImage;

        /* Add each output index from the fake outs */
        std::transform(input.outputs.begin(), input.outputs.end(), std::back_inserter(keyInput.outputIndexes),
        [](const auto output)
        {
            return output.index;
        });
        
        /* Convert our indexes to relative indexes - for example, if we
           originally had [5, 10, 20, 21, 22], this would become
           [5, 5, 10, 1, 1]. Due to this, the indexes MUST be sorted - they
           are serialized as a uint32_t, so negative values will overflow! */
        keyInput.outputIndexes = CryptoNote::absolute_output_offsets_to_relative(keyInput.outputIndexes);

        /* Store the key input */
        inputs.push_back(keyInput);
    }

    return {SUCCESS, inputs, tmpSecretKeys};
}

std::tuple<std::vector<WalletTypes::KeyOutput>, Crypto::PublicKey> setupOutputs(
    std::vector<WalletTypes::TransactionDestination> destinations)
{
    /* Sort the destinations by amount. Helps obscure which output belongs to
       which transaction */
    std::sort(destinations.begin(), destinations.end(), [](const auto &lhs, const auto &rhs)
    {
        return lhs.amount < rhs.amount;
    });

    /* Generate a random key pair for the transaction - public key gets added
       to tx extra */
    CryptoNote::KeyPair randomTxKey = CryptoNote::generateKeyPair();

    /* Index of the output */
    uint32_t outputIndex = 0;

    std::vector<WalletTypes::KeyOutput> outputs;

    for (const auto destination : destinations)
    {
        Crypto::KeyDerivation derivation;

        /* Generate derivation from receiver public view key and random tx key */
        Crypto::generate_key_derivation(
            destination.receiverPublicViewKey, randomTxKey.secretKey, derivation
        );

        Crypto::PublicKey tmpPubKey;
            
        /* Derive the temporary public key */
        Crypto::derive_public_key(
            derivation, outputIndex, destination.receiverPublicSpendKey, tmpPubKey
        );

        WalletTypes::KeyOutput keyOutput;

        keyOutput.key = tmpPubKey;
        keyOutput.amount = destination.amount;

        outputs.push_back(keyOutput);

        outputIndex++;
    }

    return {outputs, randomTxKey.publicKey};
}

std::tuple<WalletError, CryptoNote::Transaction> generateRingSignatures(
    CryptoNote::Transaction tx,
    const std::vector<WalletTypes::ObscuredInput> inputsAndFakes,
    const std::vector<Crypto::SecretKey> tmpSecretKeys)
{
    Crypto::Hash txPrefixHash;

    /* Hash the transaction prefix (Prefix is just a subset of transaction, so
       we can just do a cast here) */
    CryptoNote::getObjectHash(
        static_cast<CryptoNote::TransactionPrefix>(tx), txPrefixHash
    );

    size_t i = 0;
    
    /* Add the transaction signatures */
    for (const auto input : inputsAndFakes)
    {
        std::vector<Crypto::PublicKey> publicKeys;

        /* Add all the fake outs public keys to a vector */
        for (const auto output : input.outputs)
        {
            publicKeys.push_back(output.key);
        }

        /* Generate the ring signatures - note - modifying the transaction
           post signature generation will invalidate the signatures. */
        const auto [success, signatures] = Crypto::crypto_ops::generateRingSignatures(
            txPrefixHash, boost::get<CryptoNote::KeyInput>(tx.inputs[i]).keyImage,
            publicKeys, tmpSecretKeys[i], input.realOutput
        );

        if (!success)
        {
            return {FAILED_TO_CREATE_RING_SIGNATURE, tx};
        }

        /* Add the signatures to the transaction */
        tx.signatures.push_back(signatures);

        i++;
    }

    return {SUCCESS, tx};
}

/* Split each amount into uniform amounts, e.g.
   1234567 = 1000000 + 200000 + 30000 + 4000 + 500 + 60 + 7 */
std::vector<uint64_t> splitAmountIntoDenominations(uint64_t amount)
{
    std::vector<uint64_t> splitAmounts;

    int multiplier = 1;

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

std::vector<CryptoNote::TransactionInput> keyInputToTransactionInput(
    const std::vector<CryptoNote::KeyInput> keyInputs)
{
    std::vector<CryptoNote::TransactionInput> result;

    for (const auto input : keyInputs)
    {
        result.push_back(input);
    }

    return result;
}

std::vector<CryptoNote::TransactionOutput> keyOutputToTransactionOutput(
    const std::vector<WalletTypes::KeyOutput> keyOutputs)
{
    std::vector<CryptoNote::TransactionOutput> result;

    for (const auto output : keyOutputs)
    {
        CryptoNote::TransactionOutput tmpOutput;

        tmpOutput.amount = output.amount;

        CryptoNote::KeyOutput tmpKey;

        tmpKey.key = output.key;

        tmpOutput.target = tmpKey;

        result.push_back(tmpOutput);
    }

    return result;
}

Crypto::Hash getTransactionHash(CryptoNote::Transaction tx)
{
    std::vector<uint8_t> data = CryptoNote::toBinaryArray(tx);
    return Crypto::cn_fast_hash(data.data(), data.size());
}

std::tuple<WalletError, CryptoNote::Transaction> makeTransaction(
    const uint64_t mixin,
    const std::shared_ptr<CryptoNote::NodeRpcProxy> daemon,
    const std::vector<WalletTypes::TxInputAndOwner> ourInputs,
    const std::string paymentID,
    const std::vector<WalletTypes::TransactionDestination> destinations,
    const std::shared_ptr<SubWallets> subWallets)
{
    /* Mix our inputs with fake ones from the network to hide who we are */
    const auto [mixinError, inputsAndFakes] = setupFakeInputs(
        ourInputs, mixin, daemon
    );

    if (mixinError)
    {
        return {mixinError, CryptoNote::Transaction()};
    }

    /* Setup the transaction inputs */
    const auto [inputError, transactionInputs, tmpSecretKeys] = setupInputs(
        inputsAndFakes, subWallets->getPrivateViewKey()
    );

    if (inputError)
    {
        return {inputError, CryptoNote::Transaction()};
    }

    /* Setup the transaction outputs */
    const auto [transactionOutputs, transactionPublicKey] = setupOutputs(destinations);

    std::vector<uint8_t> extra;

    if (paymentID != "")
    {
        CryptoNote::createTxExtraWithPaymentId(paymentID, extra);
    }

    /* Append the transaction public key we generated earlier to the extra
       data */
    CryptoNote::addTransactionPublicKeyToExtra(extra, transactionPublicKey);

    CryptoNote::Transaction setupTX;

    setupTX.version = CryptoNote::CURRENT_TRANSACTION_VERSION;

    setupTX.unlockTime = 0;

    /* Convert from key inputs to the boost uglyness */
    setupTX.inputs = keyInputToTransactionInput(transactionInputs);

    /* We can't really remove boost from here yet and simplify our data types
       since we take a hash of the transaction prefix. Once we've got this
       working, maybe we can work some magic. TODO */
    setupTX.outputs = keyOutputToTransactionOutput(transactionOutputs);

    /* Pubkey, payment ID */
    setupTX.extra = extra;

    /* Fill in the transaction signatures */
    /* NOTE: Do not modify the transaction after this, or the ring signatures
       will be invalidated */
    return generateRingSignatures(setupTX, inputsAndFakes, tmpSecretKeys);
}

} // namespace SendTransaction
