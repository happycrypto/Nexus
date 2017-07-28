/*******************************************************************************************
 
			Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2014] ++
   
 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef __CRYPTER_H__
#define __CRYPTER_H__

#include "../Util/include/allocators.h" /* for SecureString */
#include "key.h"
#include "../Util/templates/serialize.h"

namespace Wallet
{
	const unsigned int WALLET_CRYPTO_KEY_SIZE = 72;
	const unsigned int WALLET_CRYPTO_SALT_SIZE = 18;

	/**
	Private key encryption is done based on a CMasterKey,
	which holds a salt and random encryption key.

	CMasterKeys are encrypted using AES-256-CBC using a key
	derived using derivation method nDerivationMethod
	(0 == EVP_sha512()) and derivation iterations nDeriveIterations.
	vchOtherDerivationParameters is provided for alternative algorithms
	which may require more parameters (such as scrypt).

	Wallet Private Keys are then encrypted using AES-256-CBC
	with the SK576 of the public key as the IV, and the
	master key's key as the encryption key (see keystore.[ch]).
	*/

	/** Master key for wallet encryption */
	class CMasterKey
	{
	public:
		std::vector<unsigned char> vchCryptedKey;
		std::vector<unsigned char> vchSalt;
		// 0 = EVP_sha512()
		// 1 = scrypt()
		unsigned int nDerivationMethod;
		unsigned int nDeriveIterations;
		// Use this for more parameters to key derivation,
		// such as the various parameters to scrypt
		std::vector<unsigned char> vchOtherDerivationParameters;

		IMPLEMENT_SERIALIZE
		(
			READWRITE(vchCryptedKey);
			READWRITE(vchSalt);
			READWRITE(nDerivationMethod);
			READWRITE(nDeriveIterations);
			READWRITE(vchOtherDerivationParameters);
		)
		CMasterKey()
		{
			// 25000 rounds is just under 0.1 seconds on a 1.86 GHz Pentium M
			// ie slightly lower than the lowest hardware we need bother supporting
			nDeriveIterations = 25000;
			nDerivationMethod = 0;
			vchOtherDerivationParameters = std::vector<unsigned char>(0);
		}
	};

	typedef std::vector<unsigned char, secure_allocator<unsigned char> > CKeyingMaterial;

	/** Encryption/decryption context with key information */
	class CCrypter
	{
	private:
		unsigned char chKey[WALLET_CRYPTO_KEY_SIZE];
		unsigned char chIV[WALLET_CRYPTO_KEY_SIZE];
		bool fKeySet;

	public:
		bool SetKeyFromPassphrase(const SecureString &strKeyData, const std::vector<unsigned char>& chSalt, const unsigned int nRounds, const unsigned int nDerivationMethod);
		bool Encrypt(const CKeyingMaterial& vchPlaintext, std::vector<unsigned char> &vchCiphertext);
		bool Decrypt(const std::vector<unsigned char>& vchCiphertext, CKeyingMaterial& vchPlaintext);
		bool SetKey(const CKeyingMaterial& chNewKey, const std::vector<unsigned char>& chNewIV);

		void CleanKey()
		{
			memset(&chKey, 0, sizeof chKey);
			memset(&chIV, 0, sizeof chIV);
			munlock(&chKey, sizeof chKey);
			munlock(&chIV, sizeof chIV);
			fKeySet = false;
		}

		CCrypter()
		{
			fKeySet = false;
		}

		~CCrypter()
		{
			CleanKey();
		}
	};

	bool EncryptSecret(CKeyingMaterial& vMasterKey, const CSecret &vchPlaintext, const uint576& nIV, std::vector<unsigned char> &vchCiphertext);
	bool DecryptSecret(const CKeyingMaterial& vMasterKey, const std::vector<unsigned char> &vchCiphertext, const uint576& nIV, CSecret &vchPlaintext);

}
#endif