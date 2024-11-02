// **********************************************************************
// This file was generated by a TARS parser!
// TARS version 3.0.3.
// **********************************************************************

#ifndef __TRANSACTION_H_
#define __TRANSACTION_H_

#include <bcos-cpp-sdk/utilities/tx/tars/tup/Tars.h>
#include <bcos-cpp-sdk/utilities/tx/tars/tup/TarsJson.h>
#include <bcos-crypto/interfaces/crypto/Hash.h>
#include <bcos-utilities/DataConvertUtility.h>
#include <boost/asio/detail/socket_ops.hpp>
#include <boost/endian/conversion.hpp>
#include <map>
#include <vector>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/obj_mac.h>
#include <openssl/ec.h>
#include <openssl/rand.h>
#include <iostream>
#include <iomanip>
#include <string>

#pragma GCC diagnostic ignored "-Wunused-parameter"

namespace bcostars
{
inline unsigned char* Chemoleonhash_inner(unsigned char md[SHA256_DIGEST_LENGTH], const std::string& _extraData);
struct TransactionData : public tars::TarsStructBase
{
public:
    static std::string className() { return "bcostars.TransactionData"; }
    static std::string MD5() { return "ea41d47be6b852a5c3edcfe7a805be85"; }
    TransactionData() { resetDefault(); }
    void resetDefault()
    {
        version = 0;
        chainID = "";
        groupID = "";
        blockLimit = 0;
        nonce = "";
        to = "";
        input.clear();
        abi = "";
    }
    template <typename WriterT>
    void writeTo(tars::TarsOutputStream<WriterT>& _os) const
    {
        _os.write(version, 1);
        _os.write(chainID, 2);
        _os.write(groupID, 3);
        _os.write(blockLimit, 4);
        _os.write(nonce, 5);
        if (to != "")
        {
            _os.write(to, 6);
        }
        _os.write(input, 7);
        if (abi != "")
        {
            _os.write(abi, 8);
        }
    }
    template <typename ReaderT>
    void readFrom(tars::TarsInputStream<ReaderT>& _is)
    {
        resetDefault();
        _is.read(version, 1, true);
        _is.read(chainID, 2, true);
        _is.read(groupID, 3, true);
        _is.read(blockLimit, 4, true);
        _is.read(nonce, 5, true);
        _is.read(to, 6, false);
        _is.read(input, 7, true);
        _is.read(abi, 8, false);
    }
    tars::JsonValueObjPtr writeToJson() const
    {
        tars::JsonValueObjPtr p = new tars::JsonValueObj();
        p->value["version"] = tars::JsonOutput::writeJson(version);
        p->value["chainID"] = tars::JsonOutput::writeJson(chainID);
        p->value["groupID"] = tars::JsonOutput::writeJson(groupID);
        p->value["blockLimit"] = tars::JsonOutput::writeJson(blockLimit);
        p->value["nonce"] = tars::JsonOutput::writeJson(nonce);
        p->value["to"] = tars::JsonOutput::writeJson(to);
        p->value["input"] = tars::JsonOutput::writeJson(bcos::toHexStringWithPrefix(input));
        p->value["abi"] = tars::JsonOutput::writeJson(abi);
        return p;
    }
    std::string writeToJsonString() const { return tars::TC_Json::writeValue(writeToJson()); }
    void readFromJson(const tars::JsonValuePtr& p, bool isRequire = true)
    {
        resetDefault();
        if (NULL == p.get() || p->getType() != tars::eJsonTypeObj)
        {
            char s[128];
            snprintf(s, sizeof(s), "read 'struct' type mismatch, get type: %d.",
                (p.get() ? p->getType() : 0));
            throw tars::TC_Json_Exception(s);
        }
        tars::JsonValueObjPtr pObj = tars::JsonValueObjPtr::dynamicCast(p);
        tars::JsonInput::readJson(version, pObj->value["version"], true);
        tars::JsonInput::readJson(chainID, pObj->value["chainID"], true);
        tars::JsonInput::readJson(groupID, pObj->value["groupID"], true);
        tars::JsonInput::readJson(blockLimit, pObj->value["blockLimit"], true);
        tars::JsonInput::readJson(nonce, pObj->value["nonce"], true);
        tars::JsonInput::readJson(to, pObj->value["to"], false);
        std::string inputHex{};
        tars::JsonInput::readJson(inputHex, pObj->value["input"], true);
        auto inputBytes = bcos::fromHexString(inputHex);
        std::copy(inputBytes->begin(), inputBytes->end(), std::back_inserter(input));
        tars::JsonInput::readJson(abi, pObj->value["abi"], false);
    }
    void readFromJsonString(const std::string& str) { readFromJson(tars::TC_Json::getValue(str)); }
    std::ostream& display(std::ostream& _os, int _level = 0) const
    {
        tars::TarsDisplayer _ds(_os, _level);
        _ds.display(version, "version");
        _ds.display(chainID, "chainID");
        _ds.display(groupID, "groupID");
        _ds.display(blockLimit, "blockLimit");
        _ds.display(nonce, "nonce");
        _ds.display(to, "to");
        _ds.display(input, "input");
        _ds.display(abi, "abi");
        return _os;
    }
    std::ostream& displaySimple(std::ostream& _os, int _level = 0) const
    {
        tars::TarsDisplayer _ds(_os, _level);
        _ds.displaySimple(version, true);
        _ds.displaySimple(chainID, true);
        _ds.displaySimple(groupID, true);
        _ds.displaySimple(blockLimit, true);
        _ds.displaySimple(nonce, true);
        _ds.displaySimple(to, true);
        _ds.displaySimple(input, true);
        _ds.displaySimple(abi, false);
        return _os;
    }

    bcos::crypto::HashType hash(bcos::crypto::Hash::Ptr _hashImpl, const std::string& _extraData) const
    {
        auto anyHasher = _hashImpl->hasher();
        bcos::crypto::HashType hashResult;
        std::visit(
            [this, &hashResult, &_extraData](auto& hasher) {
                int32_t networkVersion = boost::endian::native_to_big((int32_t)version);
                hasher.update(networkVersion);
                // encode chainID
                hasher.update(bcos::bytesConstRef((bcos::byte*)chainID.data(), chainID.size()));
                // encode groupID
                hasher.update(bcos::bytesConstRef((bcos::byte*)groupID.data(), groupID.size()));
                // encode blockLimit
                int64_t networkBlockLimit = boost::endian::native_to_big((int64_t)blockLimit);
                hasher.update(bcos::bytesConstRef((bcos::byte*)(&networkBlockLimit),
                    sizeof(networkBlockLimit) / sizeof(uint8_t)));
                // encode nonce
                hasher.update(bcos::bytesConstRef((bcos::byte*)nonce.data(), nonce.size()));
                // encode to
                hasher.update(bcos::bytesConstRef((bcos::byte*)to.data(), to.size()));
                // encode input
                hasher.update(bcos::bytesConstRef((bcos::byte*)input.data(), input.size()));
                // encode abi
                hasher.update(bcos::bytesConstRef((bcos::byte*)abi.data(), abi.size()));

                hasher.final(hashResult);

                // calculate chamoleonhash with Keccak256hash
                unsigned char md[SHA256_DIGEST_LENGTH];
                std::memcpy(md, hashResult.data(), SHA256_DIGEST_LENGTH);

                //calculate h = r + (s*g + y*H(m||r)) mod q;
                std::memcpy(hashResult.data(), Chemoleonhash_inner(md, _extraData), SHA256_DIGEST_LENGTH);
                
            },
            anyHasher);

        return hashResult;
    }

public:
    tars::Int32 version;
    std::string chainID;
    std::string groupID;
    tars::Int64 blockLimit;
    std::string nonce;
    std::string to;
    std::vector<tars::Char> input;
    std::string abi;
};

inline unsigned char* Chemoleonhash_inner(unsigned char md[SHA256_DIGEST_LENGTH], const std::string& _extraData){

    //get H(m) and change type into BIGNUM
    BIGNUM* hashbn = BN_new();
    BN_bin2bn(md, SHA256_DIGEST_LENGTH, hashbn);

    //create group G and get order q and the generator
    EC_GROUP* group = EC_GROUP_new_by_curve_name(NID_secp256k1);
    BIGNUM* order = BN_new();
    EC_GROUP_get_order(group, order, NULL);
    BN_CTX* ctx = BN_CTX_new();
    
    // BIGNUM* privateKey = BN_new();
    //calculate the final result h = r + (s*g + y*H(m||r))
    BIGNUM* r = BN_new();
    BIGNUM* s = BN_new();
    // BN_hex2bn(&privateKey, "1C78F04182D6B4F394697126A04A25D53904360D937FE6050A66D553358ABF36");
    // EC_POINT* publicKey = EC_POINT_new(group);
    // EC_POINT_mul(group, publicKey, privateKey, NULL, NULL, ctx);
    EC_POINT* publicKey = EC_POINT_new(group);
    BN_hex2bn(&r, _extraData.substr(0, 64).c_str());
    BN_hex2bn(&s, _extraData.substr(64, 128).c_str());
    EC_POINT_hex2point(group, _extraData.substr(128).c_str(), publicKey, ctx);

    EC_POINT* g_s = EC_POINT_new(group);
    EC_POINT* pk_Hm = EC_POINT_new(group);
    EC_POINT* g_s_xH = EC_POINT_new(group);
    EC_POINT_mul(group, g_s, s, NULL, NULL, ctx);
    EC_POINT_mul(group, pk_Hm, NULL, publicKey, hashbn, ctx);
    EC_POINT_add(group, g_s_xH, g_s, pk_Hm, ctx);
    BIGNUM* h = BN_new();
    BIGNUM* g_s_xH2bn = EC_POINT_point2bn(group, g_s_xH, POINT_CONVERSION_COMPRESSED, NULL, ctx);
    BN_mod_add(h, r, g_s_xH2bn, order, ctx);

    BN_bn2bin(h, md);

    BN_free(hashbn);
    BN_free(order);
    BN_CTX_free(ctx);
    EC_POINT_free(publicKey);
    EC_POINT_free(g_s);
    EC_POINT_free(pk_Hm);
    EC_POINT_free(g_s_xH);
    BN_free(r);
    BN_free(s);
    BN_free(h);
    BN_free(g_s_xH2bn);
    EC_GROUP_free(group);
    return md;
}

inline bool operator==(const TransactionData& l, const TransactionData& r)
{
    return l.version == r.version && l.chainID == r.chainID && l.groupID == r.groupID &&
           l.blockLimit == r.blockLimit && l.nonce == r.nonce && l.to == r.to &&
           l.input == r.input && l.abi == r.abi;
}
inline bool operator!=(const TransactionData& l, const TransactionData& r)
{
    return !(l == r);
}
inline std::ostream& operator<<(std::ostream& os, const TransactionData& r)
{
    os << r.writeToJsonString();
    return os;
}
inline std::istream& operator>>(std::istream& is, TransactionData& l)
{
    std::istreambuf_iterator<char> eos;
    std::string s(std::istreambuf_iterator<char>(is), eos);
    l.readFromJsonString(s);
    return is;
}

struct Transaction : public tars::TarsStructBase
{
public:
    static std::string className() { return "bcostars.Transaction"; }
    static std::string MD5() { return "1e037304f04d104276ebd5b7c4aebdea"; }
    Transaction() { resetDefault(); }
    void resetDefault()
    {
        data.resetDefault();
        dataHash.clear();
        signature.clear();
        importTime = 0;
        attribute = 0;
        sender.clear();
        extraData = "";
    }
    template <typename WriterT>
    void writeTo(tars::TarsOutputStream<WriterT>& _os) const
    {
        _os.write(data, 1);
        if (dataHash.size() > 0)
        {
            _os.write(dataHash, 2);
        }
        if (signature.size() > 0)
        {
            _os.write(signature, 3);
        }
        if (importTime != 0)
        {
            _os.write(importTime, 4);
        }
        if (attribute != 0)
        {
            _os.write(attribute, 5);
        }
        if (sender.size() > 0)
        {
            _os.write(sender, 7);
        }
        if (extraData != "")
        {
            _os.write(extraData, 8);
        }
    }
    template <typename ReaderT>
    void readFrom(tars::TarsInputStream<ReaderT>& _is)
    {
        resetDefault();
        _is.read(data, 1, false);
        _is.read(dataHash, 2, false);
        _is.read(signature, 3, false);
        _is.read(importTime, 4, false);
        _is.read(attribute, 5, false);
        _is.read(sender, 7, false);
        _is.read(extraData, 8, false);
    }
    tars::JsonValueObjPtr writeToJson() const
    {
        tars::JsonValueObjPtr p = new tars::JsonValueObj();
        p->value["data"] = tars::JsonOutput::writeJson(data);
        p->value["dataHash"] = tars::JsonOutput::writeJson(dataHash);
        p->value["signature"] = tars::JsonOutput::writeJson(signature);
        p->value["importTime"] = tars::JsonOutput::writeJson(importTime);
        p->value["attribute"] = tars::JsonOutput::writeJson(attribute);
        p->value["sender"] = tars::JsonOutput::writeJson(sender);
        p->value["extraData"] = tars::JsonOutput::writeJson(extraData);
        return p;
    }
    std::string writeToJsonString() const { return tars::TC_Json::writeValue(writeToJson()); }
    void readFromJson(const tars::JsonValuePtr& p, bool isRequire = true)
    {
        resetDefault();
        if (NULL == p.get() || p->getType() != tars::eJsonTypeObj)
        {
            char s[128];
            snprintf(s, sizeof(s), "read 'struct' type mismatch, get type: %d.",
                (p.get() ? p->getType() : 0));
            throw tars::TC_Json_Exception(s);
        }
        tars::JsonValueObjPtr pObj = tars::JsonValueObjPtr::dynamicCast(p);
        tars::JsonInput::readJson(data, pObj->value["data"], false);
        tars::JsonInput::readJson(dataHash, pObj->value["dataHash"], false);
        tars::JsonInput::readJson(signature, pObj->value["signature"], false);
        tars::JsonInput::readJson(importTime, pObj->value["importTime"], false);
        tars::JsonInput::readJson(attribute, pObj->value["attribute"], false);
        tars::JsonInput::readJson(sender, pObj->value["sender"], false);
        tars::JsonInput::readJson(extraData, pObj->value["extraData"], false);
    }
    void readFromJsonString(const std::string& str) { readFromJson(tars::TC_Json::getValue(str)); }
    std::ostream& display(std::ostream& _os, int _level = 0) const
    {
        tars::TarsDisplayer _ds(_os, _level);
        _ds.display(data, "data");
        _ds.display(dataHash, "dataHash");
        _ds.display(signature, "signature");
        _ds.display(importTime, "importTime");
        _ds.display(attribute, "attribute");
        _ds.display(sender, "sender");
        _ds.display(extraData, "extraData");
        return _os;
    }
    std::ostream& displaySimple(std::ostream& _os, int _level = 0) const
    {
        tars::TarsDisplayer _ds(_os, _level);
        _ds.displaySimple(data, true);
        _ds.displaySimple(dataHash, true);
        _ds.displaySimple(signature, true);
        _ds.displaySimple(importTime, true);
        _ds.displaySimple(attribute, true);
        _ds.displaySimple(sender, true);
        _ds.displaySimple(extraData, false);
        return _os;
    }

public:
    bcostars::TransactionData data;
    std::vector<tars::Char> dataHash;
    std::vector<tars::Char> signature;
    tars::Int64 importTime;
    tars::Int32 attribute;
    std::vector<tars::Char> sender;
    std::string extraData;
};
inline bool operator==(const Transaction& l, const Transaction& r)
{
    return l.data == r.data && l.dataHash == r.dataHash && l.signature == r.signature &&
           l.importTime == r.importTime && l.attribute == r.attribute && l.sender == r.sender &&
           l.extraData == r.extraData;
}
inline bool operator!=(const Transaction& l, const Transaction& r)
{
    return !(l == r);
}
inline std::ostream& operator<<(std::ostream& os, const Transaction& r)
{
    os << r.writeToJsonString();
    return os;
}
inline std::istream& operator>>(std::istream& is, Transaction& l)
{
    std::istreambuf_iterator<char> eos;
    std::string s(std::istreambuf_iterator<char>(is), eos);
    l.readFromJsonString(s);
    return is;
}

using TransactionDataPtr = std::shared_ptr<TransactionData>;
using TransactionDataUniquePtr = std::unique_ptr<TransactionData>;
using TransactionDataConstPtr = std::shared_ptr<const TransactionData>;
using TransactionPtr = std::shared_ptr<Transaction>;
using TransactionUniquePtr = std::unique_ptr<Transaction>;
using TransactionConstPtr = std::shared_ptr<const Transaction>;

}  // namespace bcostars


#endif
