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
    const std::string changeAddress,
    const std::shared_ptr<CryptoNote::NodeRpcProxy> daemon,
    const std::shared_ptr<SubWallets> subWallets)
{
    /* Validate the transaction input parameters */
    const WalletError error = validateTransaction(
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

    /* TODO: Split into separate function here */

    /* Mix our inputs with fake ones from the network to hide who we are */
    const auto inputsAndFakes = setupFakeInputs(ourInputs, mixin, daemon);

    /* Setup the transaction inputs */
    const auto [transactionInputs, tmpSecretKeys] = setupInputs(
        inputsAndFakes, subWallets->getPrivateViewKey()
    );

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
    const CryptoNote::Transaction finalTransaction = generateRingSignatures(
        setupTX, inputsAndFakes, tmpSecretKeys
    );

    /* TODO: Maybe another function here */
    std::promise<std::error_code> errorPromise;

    auto callback = [&errorPromise](std::error_code e)
    {
        errorPromise.set_value(e);
    };

    errorPromise = std::promise<std::error_code>();

    /* TODO */
    auto error1 = errorPromise.get_future();

    daemon->relayTransaction(finalTransaction, callback);

    auto err = error1.get();

    Crypto::Hash transactionHash = getTransactionHash(finalTransaction);

    if (err)
    {
        std::cout << "Failed to send transaction: " << err << ", "
                  << err.message() << std::endl;

        /* TODO: Handle error here */
        return {SUCCESS, transactionHash};
    }

    /* TODO: Deduct balance, remove inputs, etc */

    return {SUCCESS, transactionHash};
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

/* Take our inputs and pad them with fake inputs, based on our mixin value */
std::vector<WalletTypes::ObscuredInput> setupFakeInputs(
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

    /* TODO: Better rpc method? */
    std::vector<CryptoNote::COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS::outs_for_amount> fakeOuts;

    /* TODO: Separate function */
    std::promise<std::error_code> errorPromise;

    auto callback = [&errorPromise](std::error_code e)
    {
        errorPromise.set_value(e);
    };

    errorPromise = std::promise<std::error_code>();

    auto error = errorPromise.get_future();

    std::vector<uint64_t> amounts;

    for (const auto destination : sources)
    {
        amounts.push_back(destination.input.amount);
    }

    daemon->getRandomOutsByAmounts(std::move(amounts), mixin, fakeOuts, callback);

    auto err = error.get();

    if (err)
    {
        std::cout << "Failed to get random outputs: " << err << ", "
                  << err.message() << std::endl;
    }

    std::vector<WalletTypes::ObscuredInput> result;

    size_t i = 0;

    /* TODO: Doesn't this need to be sorted so we select the correct fake
       outs? */
    for (const auto walletAmount : sources)
    {
        WalletTypes::GlobalIndexToKey realOutput;

        realOutput.index = walletAmount.input.globalOutputIndex;
        realOutput.key = walletAmount.input.key;

        WalletTypes::ObscuredInput obscuredInput;

        /* The real public key of the transaction */
        obscuredInput.realTransactionPublicKey = walletAmount.input.transactionPublicKey;

        /* The real index of the transaction output index */
        obscuredInput.realOutputTransactionIndex = walletAmount.input.transactionIndex;

        /* The amount of the transaction */
        obscuredInput.amount = walletAmount.input.amount;

        obscuredInput.ownerPublicSpendKey = walletAmount.publicSpendKey;

        obscuredInput.ownerPrivateSpendKey = walletAmount.privateSpendKey;

        /* Sort the fake outputs by their indexes (don't want there to be an
           easy way to determine which output is the real one) */
        std::sort(fakeOuts[i].outs.begin(), fakeOuts[i].outs.end(),
        [](const auto &lhs, const auto &rhs)
        {
            return lhs.global_amount_index < rhs.global_amount_index;
        });

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

    return result;
}

std::tuple<std::vector<CryptoNote::KeyInput>, std::vector<Crypto::SecretKey>> setupInputs(
    const std::vector<WalletTypes::ObscuredInput> inputsAndFakes,
    const Crypto::SecretKey privateViewKey)
{
    std::vector<CryptoNote::KeyInput> inputs;

    std::vector<Crypto::SecretKey> tmpSecretKeys;

    for (const auto input : inputsAndFakes)
    {
        /* TODO: Do this in another function? */
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

        /* Store the tmp secret keys for later, so we can generate the ring
           signatures */
        tmpSecretKeys.push_back(tmpKeyPair.secretKey);

        Crypto::KeyImage keyImage;

        /* Generate the key image */
        Crypto::generate_key_image(
            tmpKeyPair.publicKey, tmpKeyPair.secretKey, keyImage
        );

        /* TODO: Log this or throw an exception? */
        if (tmpKeyPair.publicKey != input.outputs[input.realOutput].key)
        {
            std::cout << "Invalid transaction..." << std::endl;
        }

        CryptoNote::KeyInput keyInput;

        keyInput.amount = input.amount;
        keyInput.keyImage = keyImage;

        /* Add each output index from the fake outs */
        for (const auto output : input.outputs)
        {
            keyInput.outputIndexes.push_back(output.index);
        }
        
        /* Convert our indexes to relative indexes - for example, if we
           originally had [5, 10, 20, 21, 22], this would become
           [5, 5, 10, 1, 1]. Due to this, the indexes MUST be sorted - they
           are serialized as a uint32_t, so negative values will overflow! */
        keyInput.outputIndexes = CryptoNote::absolute_output_offsets_to_relative(keyInput.outputIndexes);

        /* Store the key input */
        inputs.push_back(keyInput);
    }

    return {inputs, tmpSecretKeys};
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

CryptoNote::Transaction generateRingSignatures(
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
        /* Make a vector of signatures large enough to hold this set of
           signatures */
        std::vector<Crypto::Signature> signatures(input.outputs.size());

        std::vector<const Crypto::PublicKey *> publicKeys;

        /* Add all the fake outs public keys to a vector */
        /* TODO: Do this in a less horrible pointerifc way along with
           generate_ring_signature */
        for (const auto &output : input.outputs)
        {
            publicKeys.push_back(&output.key);
        }

        /* Generate the ring signature, result is placed in signatures */
        bool r = Crypto::generate_ring_signature(
            txPrefixHash, boost::get<CryptoNote::KeyInput>(tx.inputs[i]).keyImage,
            publicKeys, tmpSecretKeys[i], input.realOutput, signatures.data()
        );

        /* TODO: Log this or throw an exception? */
        if (!r)
        {
            std::cout << "Failed to create ring signature..." << std::endl;
        }

        /* Add the signatures to the transaction */
        tx.signatures.push_back(signatures);

        i++;
    }

    return tx;
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

} // namespace SendTransaction
