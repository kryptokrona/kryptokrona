// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
//
// This file is part of Bytecoin.
//
// Bytecoin is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Bytecoin is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Bytecoin.  If not, see <http://www.gnu.org/licenses/>.

#include "wallet_utils.h"

#include "cryptonote.h"
#include "crypto/crypto.h"
#include "wallet/wallet_errors.h"

namespace cryptonote
{
    uint64_t getDefaultMixinByHeight(const uint64_t height)
    {
        if (height >= cryptonote::parameters::MIXIN_LIMITS_V3_HEIGHT)
        {
            return cryptonote::parameters::DEFAULT_MIXIN_V3;
        }
        if (height >= cryptonote::parameters::MIXIN_LIMITS_V2_HEIGHT)
        {
            return cryptonote::parameters::DEFAULT_MIXIN_V2;
        }
        else if (height >= cryptonote::parameters::MIXIN_LIMITS_V1_HEIGHT)
        {
            return cryptonote::parameters::DEFAULT_MIXIN_V1;
        }
        else
        {
            return cryptonote::parameters::DEFAULT_MIXIN_V0;
        }
    }

    void throwIfKeysMismatch(const crypto::SecretKey& secretKey, const crypto::PublicKey& expectedPublicKey, const std::string& message) {
      crypto::PublicKey pub;
      bool r = crypto::secret_key_to_public_key(secretKey, pub);
      if (!r || expectedPublicKey != pub) {
        throw std::system_error(make_error_code(cryptonote::error::WRONG_PASSWORD), message);
      }
    }

    bool validateAddress(const std::string& address, const cryptonote::Currency& currency) {
      cryptonote::AccountPublicAddress ignore;
      return currency.parseAccountAddressString(address, ignore);
    }

    std::ostream& operator<<(std::ostream& os, cryptonote::WalletTransactionState state) {
      switch (state) {
      case cryptonote::WalletTransactionState::SUCCEEDED:
        os << "SUCCEEDED";
        break;
      case cryptonote::WalletTransactionState::FAILED:
        os << "FAILED";
        break;
      case cryptonote::WalletTransactionState::CANCELLED:
        os << "CANCELLED";
        break;
      case cryptonote::WalletTransactionState::CREATED:
        os << "CREATED";
        break;
      case cryptonote::WalletTransactionState::DELETED:
        os << "DELETED";
        break;
      default:
        os << "<UNKNOWN>";
      }

      return os << " (" << static_cast<int>(state) << ')';
    }

    std::ostream& operator<<(std::ostream& os, cryptonote::WalletTransferType type) {
      switch (type) {
      case cryptonote::WalletTransferType::USUAL:
        os << "USUAL";
        break;
      case cryptonote::WalletTransferType::DONATION:
        os << "DONATION";
        break;
      case cryptonote::WalletTransferType::CHANGE:
        os << "CHANGE";
        break;
      default:
        os << "<UNKNOWN>";
      }

      return os << " (" << static_cast<int>(type) << ')';
    }

    std::ostream& operator<<(std::ostream& os, cryptonote::WalletGreen::WalletState state) {
      switch (state) {
      case cryptonote::WalletGreen::WalletState::INITIALIZED:
        os << "INITIALIZED";
        break;
      case cryptonote::WalletGreen::WalletState::NOT_INITIALIZED:
        os << "NOT_INITIALIZED";
        break;
      default:
        os << "<UNKNOWN>";
      }

      return os << " (" << static_cast<int>(state) << ')';
    }

    std::ostream& operator<<(std::ostream& os, cryptonote::WalletGreen::WalletTrackingMode mode) {
      switch (mode) {
      case cryptonote::WalletGreen::WalletTrackingMode::TRACKING:
        os << "TRACKING";
        break;
      case cryptonote::WalletGreen::WalletTrackingMode::NOT_TRACKING:
        os << "NOT_TRACKING";
        break;
      case cryptonote::WalletGreen::WalletTrackingMode::NO_ADDRESSES:
        os << "NO_ADDRESSES";
        break;
      default:
        os << "<UNKNOWN>";
      }

      return os << " (" << static_cast<int>(mode) << ')';
    }

    TransferListFormatter::TransferListFormatter(const cryptonote::Currency& currency, const WalletGreen::TransfersRange& range) :
      m_currency(currency),
      m_range(range) {
    }

    void TransferListFormatter::print(std::ostream& os) const {
      for (auto it = m_range.first; it != m_range.second; ++it) {
        os << '\n' << std::setw(21) << m_currency.formatAmount(it->second.amount) <<
          ' ' << (it->second.address.empty() ? "<UNKNOWN>" : it->second.address) <<
          ' ' << it->second.type;
      }
    }

    std::ostream& operator<<(std::ostream& os, const TransferListFormatter& formatter) {
      formatter.print(os);
      return os;
    }

    WalletOrderListFormatter::WalletOrderListFormatter(const cryptonote::Currency& currency, const std::vector<cryptonote::WalletOrder>& walletOrderList) :
      m_currency(currency),
      m_walletOrderList(walletOrderList) {
    }

    void WalletOrderListFormatter::print(std::ostream& os) const {
      os << '{';

      if (!m_walletOrderList.empty()) {
        os << '<' << m_currency.formatAmount(m_walletOrderList.front().amount) << ", " << m_walletOrderList.front().address << '>';

        for (auto it = std::next(m_walletOrderList.begin()); it != m_walletOrderList.end(); ++it) {
          os << '<' << m_currency.formatAmount(it->amount) << ", " << it->address << '>';
        }
      }

      os << '}';
    }

    std::ostream& operator<<(std::ostream& os, const WalletOrderListFormatter& formatter) {
      formatter.print(os);
      return os;
    }
}
