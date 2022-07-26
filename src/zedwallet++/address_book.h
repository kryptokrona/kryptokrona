// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

#include "json.hpp"

#include <string>

#include <vector>

#include <WalletBackend/WalletBackend.h>

using nlohmann::json;

struct AddressBookEntry
{
    AddressBookEntry() {}

    /* Used for quick comparison with strings */
    AddressBookEntry(const std::string friendlyName) : friendlyName(friendlyName) {}

    AddressBookEntry(
        const std::string friendlyName,
        const std::string address,
        const std::string paymentID) :
        friendlyName(friendlyName),
        address(address),
        paymentID(paymentID) {}

    /* Friendly name for this address book entry */
    std::string friendlyName;

    /* The wallet address of this entry */
    std::string address;

    /* The payment ID associated with this address */
    std::string paymentID;

    /* Only compare via name as we don't really care about the contents */
    bool operator==(const AddressBookEntry &rhs) const
    {
        return rhs.friendlyName == friendlyName;
    }
};

void addToAddressBook();

void sendFromAddressBook(const std::shared_ptr<WalletBackend> walletBackend);

void deleteFromAddressBook();

void listAddressBook();

const std::tuple<bool, AddressBookEntry> getAddressBookEntry(
    const std::vector<AddressBookEntry> addressBook);

const std::string getAddressBookName(const std::vector<AddressBookEntry> addressBook);

std::vector<AddressBookEntry> getAddressBook();

bool saveAddressBook(const std::vector<AddressBookEntry> addressBook);

bool isAddressBookEmpty(const std::vector<AddressBookEntry> addressBook);

void to_json(json &j, const AddressBookEntry &a);

void from_json(const json &j, AddressBookEntry &a);
