// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

#pragma once

#include <string>

/* Note: Putting the number of the error is not needed, as they auto increment,
   however, it makes it easier to see at a glance what error you got, whilst
   developing */
enum WalletError
{
    /* No error, operation suceeded. */
    SUCCESS = 0,

    /* The wallet filename given does not exist or the program does not have
       permission to view it */
    FILENAME_NON_EXISTENT = 1,

    /* The output filename was unable to be opened for saving, probably due
       to invalid characters */
    INVALID_WALLET_FILENAME = 2,

    /* The wallet does not have the wallet identifier prefix */
    NOT_A_WALLET_FILE = 3,

    /* The file has the correct wallet file prefix, but is corrupted in some
       other way, such as a missing IV */
    WALLET_FILE_CORRUPTED = 4,

    /* Either the AES decryption failed due to wrong padding, or the decrypted
       data does not have the correct prefix indicating the password is
       correct. */
    WRONG_PASSWORD = 5,

    /* The wallet file is using a different version than the version supported
       by this version of the software. (Also could be potential corruption.) */
    UNSUPPORTED_WALLET_FILE_FORMAT_VERSION = 6,

    /* The mnemonic seed is invalid for some reason, for example, it has the
       wrong length, or an invalid checksum */
    INVALID_MNEMONIC = 7,

    /* Trying to create a wallet file which already exists */
    WALLET_FILE_ALREADY_EXISTS = 8,

    /* The call to NodeRpcProxy::init() failed */
    FAILED_TO_INIT_DAEMON = 9,

    /* The address given does not exist in the wallet container */
    ADDRESS_NOT_FOUND = 10,

    /* Not enough funds were found for the transaction */
    NOT_ENOUGH_FUNDS = 11,

    /* The address is the wrong length - neither a standard, nor an integrated
       address */
    ADDRESS_WRONG_LENGTH = 12,

    /* The address does not have the correct prefix, e.g. does not begin with
       TRTL (or whatever is specified in WalletConfig::addressPrefix) */
    ADDRESS_WRONG_PREFIX = 13,

    /* The address is not fully comprised of base58 characters */
    ADDRESS_NOT_BASE58 = 14,

    /* The address is invalid for some other reason (possibly checksum) */
    ADDRESS_NOT_VALID = 15,

    /* The payment ID encoded in the integrated address is not valid */ 
    INTEGRATED_ADDRESS_PAYMENT_ID_INVALID = 16,

    /* The fee given is lower than the CryptoNote::parameters::MINIMUM_FEE */
    FEE_TOO_SMALL = 17,

    /* The address given does not exist in this container, and it's required,
       for example you specified it as the address to get the balance from */
    ADDRESS_NOT_IN_WALLET = 18,

    /* The destinations array is empty */
    NO_DESTINATIONS_GIVEN = 19,

    /* One of the destination parameters has an amount given of zero. */
    AMOUNT_IS_ZERO = 20,

    /* Amount + fee is greater than the total balance available in the
       subwallets specified (or all wallets, if not specified) */
    NOT_ENOUGH_BALANCE = 21,

    /* The mixin given is too low for the current height known by the wallet */
    MIXIN_TOO_SMALL = 22,

    /* The mixin given is too large for the current height known by the wallet */
    MIXIN_TOO_BIG = 23,

    /* Payment ID is not 64 chars */
    PAYMENT_ID_WRONG_LENGTH = 24,

    /* The payment ID is not hex */
    PAYMENT_ID_INVALID = 25,

    /* The address is an integrated address - but integrated addresses aren't
       valid for this parameter, for example, change address */
    ADDRESS_IS_INTEGRATED = 26,

    /* Conflicting payment ID's were found, due to integrated addresses. These
       could mean an integrated address + payment ID were given, where they
       are not the same, or that multiple integrated addresses with different
       payment IDs were given */
    CONFLICTING_PAYMENT_IDS = 27,

    /* Can't get mixin/fake outputs from the daemon, and mixin is not zero */
    CANT_GET_FAKE_OUTPUTS = 28,

    /* We got mixin/fake outputs from the daemon, but not enough. E.g. using a
       mixin of 3, we only got one fake output -> can't form transaction.
       This is most likely to be encountered on new networks, where not
       enough outputs have been created, or if you have a very large output
       that not enough have been created of.

       Try resending the transaction with a mixin of zero, if that is an option
       on your network. */
    NOT_ENOUGH_FAKE_OUTPUTS = 29,

    /* The key image generated was not valid. This is most likely a programmer
       error. */
    INVALID_GENERATED_KEYIMAGE = 30,

    /* Could not contact the daemon to complete the request. Ensure it is
       online and not frozen */
    DAEMON_OFFLINE = 31,

    /* An error occured whilst the daemon processed the request. Possibly our
       software is outdated, the daemon is faulty, or there is a programmer
       error. Check your daemon logs for more info (set_log 4) */
    DAEMON_ERROR = 32,

    /* Something went wrong creating the ring signatures. Probably a programmer
       error */
    FAILED_TO_CREATE_RING_SIGNATURE = 33,
};

std::string getErrorMessage(WalletError error);
