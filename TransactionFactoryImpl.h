/**
 *  Copyright (C) 2021 FISCO BCOS.
 *  SPDX-License-Identifier: Apache-2.0
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 * @brief tars implementation for TransactionFactory
 * @file TransactionFactoryImpl.h
 * @author: ancelmo
 * @date 2021-04-20
 */
#pragma once
#include "../impl/TarsHashable.h"
#include "TransactionImpl.h"
#include <bcos-concepts/Hash.h>
#include <bcos-framework/protocol/TransactionFactory.h>
#include <fstream>
#include <stdexcept>
#include <utility>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/obj_mac.h>
#include <openssl/ec.h>
#include <openssl/rand.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>

namespace bcostars::protocol
{
class TransactionFactoryImpl : public bcos::protocol::TransactionFactory
{
public:
    using TransactionType = TransactionImpl;

    TransactionFactoryImpl(bcos::crypto::CryptoSuite::Ptr cryptoSuite)
      : m_cryptoSuite(std::move(cryptoSuite))
    {}
    TransactionFactoryImpl(const TransactionFactoryImpl&) = default;
    TransactionFactoryImpl(TransactionFactoryImpl&&) = default;
    TransactionFactoryImpl& operator=(const TransactionFactoryImpl&) = default;
    TransactionFactoryImpl& operator=(TransactionFactoryImpl&&) = default;
    ~TransactionFactoryImpl() override = default;

    inline char* Chemoleonhash_inner(unsigned char md[SHA256_DIGEST_LENGTH], const std::string& _extraData)
    {
        BIGNUM* hashbn = BN_new();
        BN_bin2bn(md, SHA256_DIGEST_LENGTH, hashbn);

        //create group G and get order q and the generator
        EC_GROUP* group = EC_GROUP_new_by_curve_name(NID_secp256k1);
        BIGNUM* order = BN_new();
        EC_GROUP_get_order(group, order, NULL);
        BN_CTX* ctx = BN_CTX_new();

        //calculate the final result H = R - (s * g + PK * H(m||r))
        EC_POINT* R = EC_POINT_new(group);
        BIGNUM* s = BN_new();
        EC_POINT* publicKey = EC_POINT_new(group);

        EC_POINT_hex2point(group, _extraData.substr(0, 66).c_str(), R, ctx);
        BN_hex2bn(&s, _extraData.substr(66, 130).c_str());
        EC_POINT_hex2point(group, _extraData.substr(130).c_str(), publicKey, ctx);

        EC_POINT* sG_pkH = EC_POINT_new(group);
        EC_POINT* H = EC_POINT_new(group);
        BIGNUM* H_bn = BN_new();
        EC_POINT_mul(group, sG_pkH, s, publicKey, hashbn, ctx);
        EC_POINT_invert(group, sG_pkH, ctx);
        EC_POINT_add(group, H, R, sG_pkH, ctx);
        EC_POINT_point2bn(group, H, POINT_CONVERSION_UNCOMPRESSED, H_bn, ctx);
        BN_mod(H_bn, H_bn, order, ctx);

        BN_free(hashbn);
        BN_free(order);
        BN_CTX_free(ctx);
        EC_POINT_free(publicKey);
        EC_POINT_free(sG_pkH);
        EC_POINT_free(R);
        EC_POINT_free(H);
        BN_free(s);
        BN_free(H_bn);
        EC_GROUP_free(group);
        return BN_bn2hex(H_bn);
    }

    bcos::protocol::Transaction::Ptr createTransaction(
        bcos::bytesConstRef txData, bool checkSig = true, bool checkHash = false) override
    {
        auto transaction = std::make_shared<TransactionImpl>(
            [m_transaction = bcostars::Transaction()]() mutable { return &m_transaction; });

        transaction->decode(txData);

        auto originDataHash = std::move(transaction->mutableInner().dataHash);
        transaction->mutableInner().dataHash.clear();
        transaction->calculateHash(m_cryptoSuite->hashImpl()->hasher());

        unsigned char md[SHA256_DIGEST_LENGTH];
        std::memcpy(md, transaction->mutableInner().dataHash.data(), SHA256_DIGEST_LENGTH);

        transaction->mutableInner().dataHash.clear();

        std::string chemoleonHash(Chemoleonhash_inner(md, transaction->mutableInner().extraData));
        std::vector<char> result;
        for(size_t i = 0; i < chemoleonHash.size(); i += 2){
            std::string byteString = chemoleonHash.substr(i, 2);
            char byte = static_cast<char>(std::stoi(byteString, nullptr, 16));
            result.push_back(byte);
        }

        transaction->mutableInner().dataHash = result;
        
        // check if hash matching
        if (checkHash && !originDataHash.empty() &&
            (originDataHash != transaction->mutableInner().dataHash)) [[unlikely]]
        {
            bcos::crypto::HashType originHashResult(
                (bcos::byte*)originDataHash.data(), originDataHash.size());
            bcos::crypto::HashType hashResult(
                (bcos::byte*)transaction->mutableInner().dataHash.data(),
                transaction->mutableInner().dataHash.size());

            BCOS_LOG(WARNING) << LOG_DESC("the transaction hash does not match")
                              << LOG_KV("originHash", originHashResult.hex())
                              << LOG_KV("realHash", hashResult.hex());
            BOOST_THROW_EXCEPTION(std::invalid_argument("transaction hash mismatching"));
        }

        if (checkSig)
        {
            transaction->verify(*m_cryptoSuite->hashImpl(), *m_cryptoSuite->signatureImpl());
        }
        return transaction;
    }

    bcos::protocol::Transaction::Ptr createTransaction(int32_t _version, std::string _to,
        bcos::bytes const& _input, std::string const& _nonce, int64_t _blockLimit,
        std::string _chainId, std::string _groupId, int64_t _importTime) override
    {
        auto transaction = std::make_shared<bcostars::protocol::TransactionImpl>(
            [m_transaction = bcostars::Transaction()]() mutable { return &m_transaction; });
        auto& inner = transaction->mutableInner();
        inner.data.version = _version;
        inner.data.to = std::move(_to);
        inner.data.input.assign(_input.begin(), _input.end());
        inner.data.blockLimit = _blockLimit;
        inner.data.chainID = std::move(_chainId);
        inner.data.groupID = std::move(_groupId);
        inner.data.nonce = boost::lexical_cast<std::string>(_nonce);
        inner.importTime = _importTime;

        // Update the hash field
        bcos::concepts::hash::calculate(m_cryptoSuite->hashImpl()->hasher(), inner, inner.dataHash);

        return transaction;
    }

    bcos::protocol::Transaction::Ptr createTransaction(int32_t _version, std::string _to,
        bcos::bytes const& _input, std::string const& _nonce, int64_t _blockLimit,
        std::string _chainId, std::string _groupId, int64_t _importTime,
        bcos::crypto::KeyPairInterface::Ptr keyPair) override
    {
        auto tx = createTransaction(_version, std::move(_to), _input, _nonce, _blockLimit,
            std::move(_chainId), std::move(_groupId), _importTime);
        auto sign = m_cryptoSuite->signatureImpl()->sign(*keyPair, tx->hash(), true);

        auto tarsTx = std::dynamic_pointer_cast<bcostars::protocol::TransactionImpl>(tx);
        auto& inner = tarsTx->mutableInner();
        inner.signature.assign(sign->begin(), sign->end());

        return tx;
    }

    void setCryptoSuite(bcos::crypto::CryptoSuite::Ptr cryptoSuite)
    {
        m_cryptoSuite = std::move(cryptoSuite);
    }
    bcos::crypto::CryptoSuite::Ptr cryptoSuite() override { return m_cryptoSuite; }

private:
    bcos::crypto::CryptoSuite::Ptr m_cryptoSuite;
};

}  // namespace bcostars::protocol