// Copyright (c) 2026, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.
//
// ---------------------------------------------------------------------------
// Atomic-swap spike: the Kryptokrona (XKR) side of a BTC <-> XKR atomic swap.
//
// This is a *proof of concept*, not production code. It validates, against
// Kryptokrona's real crypto primitives (we link the actual `crypto` library),
// that the "shared 2-of-2 output" construction used by the Monero side of the
// COMIT / UnstoppableSwap xmr-btc-swap protocol works unchanged on XKR.
//
// The claim we are proving:
//
//   * Two parties A and B each hold a spend key share (b_A, B_A), (b_B, B_B)
//     and a view key share (v_A, V_A), (v_B, V_B).
//   * They form a *shared address* with spend pubkey  B = B_A + B_B  and view
//     secret  v = v_A + v_B  (both parties know v, so both can watch; neither
//     alone knows the spend secret b = b_A + b_B, so neither alone can spend).
//   * XKR sent to that address creates an ordinary one-time output
//         P = H_s(r*V)*G + B .
//   * When one party learns *both* spend shares it reconstructs
//         x = H_s(r*V) + (b_A + b_B)   with   x*G == P ,
//     and can sweep the output with a completely ordinary single-signer
//     CryptoNote ring signature.
//
// Crucially, no interactive/collaborative ring signing is required: the swap's
// atomicity lives entirely on the (scripted) Bitcoin side. That is why the XKR
// side needs *no* new consensus-critical crypto -- only key arithmetic that the
// existing library already supports.
//
// If every assertion below passes, the XKR-side cryptographic path for the swap
// is sound on Kryptokrona's own primitives.
// ---------------------------------------------------------------------------

#undef NDEBUG

#include <cassert>
#include <cstring>
#include <iostream>
#include <vector>

#include "crypto/crypto.h"
#include "crypto_types.h"
#include "common/string_tools.h"
#include "common/base58.h"
#include "config/cryptonote_config.h"

// Low-level ref10 curve ops (C linkage), same include style as crypto.cpp.
extern "C"
{
#include "crypto/crypto-ops.h"
}

using namespace crypto;

// ---------------------------------------------------------------------------
// Small helpers for the two operations the shared-output scheme needs but that
// the high-level crypto.h wrappers don't expose directly: point addition
// (combining public keys) and scalar addition mod l (combining secret keys).
// ---------------------------------------------------------------------------

// C = A + B on the ed25519 group (public-key addition).
static PublicKey addPublicKeys(const PublicKey &a, const PublicKey &b)
{
    ge_p3 A, B, sum;
    ge_cached Bcached;
    ge_p1p1 tmp;

    if (ge_frombytes_vartime(&A, reinterpret_cast<const unsigned char *>(&a)) != 0)
    {
        throw std::runtime_error("addPublicKeys: left key is not a valid point");
    }
    if (ge_frombytes_vartime(&B, reinterpret_cast<const unsigned char *>(&b)) != 0)
    {
        throw std::runtime_error("addPublicKeys: right key is not a valid point");
    }

    ge_p3_to_cached(&Bcached, &B);
    ge_add(&tmp, &A, &Bcached);
    ge_p1p1_to_p3(&sum, &tmp);

    PublicKey out;
    ge_p3_tobytes(reinterpret_cast<unsigned char *>(&out), &sum);
    return out;
}

// c = a + b (mod l) for two secret scalars.
static SecretKey addSecretKeys(const SecretKey &a, const SecretKey &b)
{
    SecretKey out;
    sc_add(
        reinterpret_cast<unsigned char *>(&out),
        reinterpret_cast<const unsigned char *>(&a),
        reinterpret_cast<const unsigned char *>(&b));
    return out;
}

static bool equalKeys(const PublicKey &a, const PublicKey &b)
{
    return std::memcmp(&a, &b, sizeof(PublicKey)) == 0;
}

// Encode a standard CryptoNote address string for the given prefix. A standard
// AccountPublicAddress serializes as exactly `spendPub(32) || viewPub(32)`, and
// tools::base58::encode_addr prepends the varint prefix and appends the 4-byte
// keccak checksum -- the same payload cryptonote::getAccountAddressAsStr feeds
// the encoder, reproduced here so the spike need only link `crypto` + `common`.
static std::string encodeAddress(uint64_t prefix, const PublicKey &spendPub, const PublicKey &viewPub)
{
    std::string payload;
    payload.append(reinterpret_cast<const char *>(&spendPub), sizeof(PublicKey));
    payload.append(reinterpret_cast<const char *>(&viewPub), sizeof(PublicKey));
    return tools::base58::encode_addr(prefix, payload);
}

template <typename T>
static std::string hex(const T &pod)
{
    return common::podToHex(pod);
}

// A simple CryptoNote key pair (spend or view).
struct KeyShare
{
    PublicKey pub;
    SecretKey sec;

    static KeyShare generate()
    {
        KeyShare k;
        generate_keys(k.pub, k.sec);
        return k;
    }
};

// Machine-readable key emission for the live testnet swap test. Generates two
// spend/view shares, and prints the shared address plus the *combined* secret
// keys (b_A+b_B, v_A+v_B) that reconstruct the shared wallet. A wallet imported
// from these combined secrets derives an address identical to SHARED_ADDRESS_*,
// which is exactly the additivity claim the live test proves on-chain.
static int emitKeygen()
{
    KeyShare spendA = KeyShare::generate();
    KeyShare spendB = KeyShare::generate();
    KeyShare viewA = KeyShare::generate();
    KeyShare viewB = KeyShare::generate();

    PublicKey sharedSpendPub = addPublicKeys(spendA.pub, spendB.pub);
    PublicKey sharedViewPub = addPublicKeys(viewA.pub, viewB.pub);
    SecretKey combinedSpend = addSecretKeys(spendA.sec, spendB.sec);
    SecretKey combinedView = addSecretKeys(viewA.sec, viewB.sec);

    const uint64_t prefixSekr = cryptonote::parameters::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX;
    const uint64_t prefixXkr = cryptonote::parameters::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX_ALT;

    std::cout << "SHARED_ADDRESS_SEKR=" << encodeAddress(prefixSekr, sharedSpendPub, sharedViewPub) << "\n";
    std::cout << "SHARED_ADDRESS_XKR=" << encodeAddress(prefixXkr, sharedSpendPub, sharedViewPub) << "\n";
    std::cout << "SHARED_SPEND_PUBLIC=" << hex(sharedSpendPub) << "\n";
    std::cout << "SHARED_VIEW_PUBLIC=" << hex(sharedViewPub) << "\n";
    std::cout << "COMBINED_SPEND_KEY=" << hex(combinedSpend) << "\n";
    std::cout << "COMBINED_VIEW_KEY=" << hex(combinedView) << "\n";
    std::cout << "SPEND_SHARE_A=" << hex(spendA.sec) << "\n";
    std::cout << "SPEND_SHARE_B=" << hex(spendB.sec) << "\n";
    std::cout << "VIEW_SHARE_A=" << hex(viewA.sec) << "\n";
    std::cout << "VIEW_SHARE_B=" << hex(viewB.sec) << "\n";
    return 0;
}

int main(int argc, char **argv)
{
    try
    {
        // `swap_spike keygen` emits machine-readable key material for the live
        // testnet test; with no args it runs the self-contained crypto proof.
        if (argc >= 2 && std::string(argv[1]) == "keygen")
        {
            return emitKeygen();
        }

        std::cout << "== Kryptokrona atomic-swap spike: shared 2-of-2 output ==\n\n";

        // -------------------------------------------------------------------
        // 1. Each party generates its spend + view key shares.
        // -------------------------------------------------------------------
        KeyShare spendA = KeyShare::generate();
        KeyShare spendB = KeyShare::generate();
        KeyShare viewA = KeyShare::generate();
        KeyShare viewB = KeyShare::generate();

        std::cout << "[1] Party A spend pub: " << hex(spendA.pub) << "\n";
        std::cout << "    Party B spend pub: " << hex(spendB.pub) << "\n";

        // -------------------------------------------------------------------
        // 2. Form the shared address.
        //      shared spend pubkey  B = B_A + B_B
        //      shared view secret   v = v_A + v_B   (both parties learn this)
        //      shared view pubkey   V = v*G = V_A + V_B
        // -------------------------------------------------------------------
        PublicKey sharedSpendPub = addPublicKeys(spendA.pub, spendB.pub);

        SecretKey sharedViewSec = addSecretKeys(viewA.sec, viewB.sec);
        PublicKey sharedViewPub;
        assert(secret_key_to_public_key(sharedViewSec, sharedViewPub));

        // Sanity: the summed view secret's pubkey equals the summed view pubkeys.
        assert(equalKeys(sharedViewPub, addPublicKeys(viewA.pub, viewB.pub)));

        std::cout << "\n[2] Shared spend pub (B_A+B_B): " << hex(sharedSpendPub) << "\n";
        std::cout << "    Shared view  pub (V_A+V_B): " << hex(sharedViewPub) << "\n";
        std::cout << "    Shared view  sec (v_A+v_B): " << hex(sharedViewSec) << "\n";

        // -------------------------------------------------------------------
        // 2b. Encode the shared keys as a REAL, fundable Kryptokrona address,
        //     then round-trip through decode_addr to prove it is a valid
        //     Base58 address (correct checksum) carrying exactly our keys.
        // -------------------------------------------------------------------
        const uint64_t prefixSekr = cryptonote::parameters::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX;
        const uint64_t prefixXkr = cryptonote::parameters::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX_ALT;

        std::string sharedAddressSekr = encodeAddress(prefixSekr, sharedSpendPub, sharedViewPub);
        std::string sharedAddressXkr = encodeAddress(prefixXkr, sharedSpendPub, sharedViewPub);

        // Round-trip: decode must recover the prefix and the 64-byte key payload.
        uint64_t decodedPrefix = 0;
        std::string decodedPayload;
        assert(tools::base58::decode_addr(sharedAddressSekr, decodedPrefix, decodedPayload));
        assert(decodedPrefix == prefixSekr);
        assert(decodedPayload.size() == 2 * sizeof(PublicKey));
        assert(std::memcmp(decodedPayload.data(), &sharedSpendPub, sizeof(PublicKey)) == 0);
        assert(std::memcmp(decodedPayload.data() + sizeof(PublicKey), &sharedViewPub, sizeof(PublicKey)) == 0);

        std::cout << "\n[2b] Fundable shared address (SEKR): " << sharedAddressSekr << "\n";
        std::cout << "     Fundable shared address (Xkr):  " << sharedAddressXkr << "\n";
        std::cout << "     Decodes back to the same keys. OK\n";

        // -------------------------------------------------------------------
        // 3. Simulate a deposit into the shared address.
        //    A sender picks a random tx key r (R = r*G, published in the tx),
        //    then for output index i creates the one-time output pubkey
        //        P = H_s(r*V) * G + B .
        // -------------------------------------------------------------------
        const size_t outputIndex = 0;

        KeyShare txKey = KeyShare::generate(); // r = txKey.sec, R = txKey.pub

        KeyDerivation senderDerivation;
        // derivation = r * V   (generate_key_derivation(pub, sec) computes sec*pub)
        assert(generate_key_derivation(sharedViewPub, txKey.sec, senderDerivation));

        PublicKey oneTimeOutputKey;
        assert(derive_public_key(senderDerivation, outputIndex, sharedSpendPub, oneTimeOutputKey));

        std::cout << "\n[3] Deposit simulated. Tx pub R: " << hex(txKey.pub) << "\n";
        std::cout << "    One-time output P: " << hex(oneTimeOutputKey) << "\n";

        // -------------------------------------------------------------------
        // 4. Either party scans the output using the shared view secret.
        //        derivation' = v * R   must equal   r * V
        // -------------------------------------------------------------------
        KeyDerivation scanDerivation;
        assert(generate_key_derivation(txKey.pub, sharedViewSec, scanDerivation));
        assert(std::memcmp(&scanDerivation, &senderDerivation, sizeof(KeyDerivation)) == 0);
        std::cout << "\n[4] Recipient re-derived the shared secret from (R, v). OK\n";

        // -------------------------------------------------------------------
        // 5. Neither party can spend alone.
        //    With only its own spend share, a party derives
        //        x_A = H_s(derivation) + b_A ,  and  x_A * G != P .
        // -------------------------------------------------------------------
        SecretKey xA;
        derive_secret_key(scanDerivation, outputIndex, spendA.sec, xA);
        PublicKey xApub;
        assert(secret_key_to_public_key(xA, xApub));
        assert(!equalKeys(xApub, oneTimeOutputKey));
        std::cout << "\n[5] Party A alone CANNOT spend (x_A*G != P). OK\n";

        // -------------------------------------------------------------------
        // 6. Reveal: the winning party learns both spend shares and
        //    reconstructs the full one-time secret
        //        x = H_s(derivation) + (b_A + b_B) ,   x * G == P .
        // -------------------------------------------------------------------
        SecretKey sharedSpendSec = addSecretKeys(spendA.sec, spendB.sec);

        SecretKey oneTimeSecret;
        derive_secret_key(scanDerivation, outputIndex, sharedSpendSec, oneTimeSecret);

        PublicKey reconstructedPub;
        assert(secret_key_to_public_key(oneTimeSecret, reconstructedPub));
        assert(equalKeys(reconstructedPub, oneTimeOutputKey));

        // And equivalently x == x_A + b_B (linearity), a useful cross-check.
        SecretKey oneTimeSecretViaLinearity = addSecretKeys(xA, spendB.sec);
        assert(std::memcmp(&oneTimeSecret, &oneTimeSecretViaLinearity, sizeof(SecretKey)) == 0);

        std::cout << "\n[6] Reconstructed one-time secret x with x*G == P. OK\n";

        // -------------------------------------------------------------------
        // 7. Sweep: build an ordinary ring signature spending P, exactly as a
        //    normal wallet would. Key image is well-defined and the signature
        //    verifies -- proving the output is spendable with no special path.
        // -------------------------------------------------------------------
        KeyImage keyImage;
        generate_key_image(oneTimeOutputKey, oneTimeSecret, keyImage);

        const size_t ringSize = 4;
        const uint64_t realIndex = 2;

        std::vector<PublicKey> ring(ringSize);
        for (size_t i = 0; i < ringSize; i++)
        {
            ring[i] = KeyShare::generate().pub; // decoys
        }
        ring[realIndex] = oneTimeOutputKey; // the real output at the chosen index

        // Any 32-byte value stands in for the tx prefix hash being signed.
        Hash prefixHash;
        std::memcpy(&prefixHash, &txKey.pub, sizeof(Hash));

        auto [ok, signatures] = crypto_ops::generateRingSignatures(
            prefixHash, keyImage, ring, oneTimeSecret, realIndex);
        assert(ok);

        bool verified = crypto_ops::checkRingSignature(prefixHash, keyImage, ring, signatures);
        assert(verified);
        std::cout << "\n[7] Sweep ring signature verifies. OK\n";

        // -------------------------------------------------------------------
        // 8. Negative control: a tampered message must fail verification.
        // -------------------------------------------------------------------
        Hash tampered = prefixHash;
        reinterpret_cast<unsigned char *>(&tampered)[0] ^= 0x01;
        assert(!crypto_ops::checkRingSignature(tampered, keyImage, ring, signatures));
        std::cout << "[8] Tampered message rejected. OK\n";

        std::cout << "\n== ALL CHECKS PASSED ==\n";
        std::cout << "The XKR side of a BTC<->XKR swap needs only shared-key\n"
                     "arithmetic + an ordinary sweep -- no new consensus crypto.\n";
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "\nSPIKE FAILED: " << e.what() << "\n";
        return 1;
    }
}
