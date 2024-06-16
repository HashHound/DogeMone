#include "include_base_utils.h"
using namespace epee;

#include "cryptonote_basic_impl.h"
#include "string_tools.h"
#include "serialization/binary_utils.h"
#include "serialization/container.h"
#include "cryptonote_format_utils.h"
#include "cryptonote_config.h"
#include "misc_language.h"
#include "common/base58.h"
#include "crypto/hash.h"
#include "common/int-util.h"
#include "common/util.h"
#include "common/dns_utils.h"
#include "cryptonote_basic/account.h"
#include "ringct/rctOps.h" // Include rctOps for rct operations

#undef MONERO_DEFAULT_LOG_CATEGORY
#define MONERO_DEFAULT_LOG_CATEGORY "cn"

namespace cryptonote {

  struct integrated_address {
    account_public_address adr;
    crypto::hash8 payment_id;

    BEGIN_SERIALIZE_OBJECT()
      FIELD(adr)
      FIELD(payment_id)
    END_SERIALIZE()

    BEGIN_KV_SERIALIZE_MAP()
      KV_SERIALIZE(adr)
      KV_SERIALIZE(payment_id)
    END_KV_SERIALIZE_MAP()
  };

  /************************************************************************/
  /* Cryptonote helper functions                                          */
  /************************************************************************/
  //-----------------------------------------------------------------------------------------------
  size_t get_min_block_weight(uint8_t version)
  {
    return CRYPTONOTE_BLOCK_GRANTED_FULL_REWARD_ZONE;
  }
  //-----------------------------------------------------------------------------------------------
  size_t get_max_tx_size()
  {
    return CRYPTONOTE_MAX_TX_SIZE;
  }
  //-----------------------------------------------------------------------------------------------
  bool get_block_reward(size_t median_weight, size_t current_block_weight, uint64_t already_generated_coins, uint64_t &reward, uint8_t version) {
    const int target = DIFFICULTY_TARGET;
    const int target_minutes = 2; // genesis tx based on 120s
    const int emission_speed_factor = EMISSION_SPEED_FACTOR_PER_MINUTE - (target_minutes - 1);

    uint64_t base_reward = (MONEY_SUPPLY - already_generated_coins) >> emission_speed_factor >> 3;
    if (base_reward < (FINAL_SUBSIDY_PER_MINUTE * target_minutes) >> 3) {
      base_reward = (FINAL_SUBSIDY_PER_MINUTE * target_minutes) >> 3;
    }

    if (already_generated_coins == 0) {
      base_reward = MONEY_SUPPLY >> emission_speed_factor;
    }

    uint64_t full_reward_zone = get_min_block_weight(version);

    // Make it soft
    if (median_weight < full_reward_zone) {
      median_weight = full_reward_zone;
    }

    if (current_block_weight <= median_weight) {
      reward = base_reward;
      return true;
    }

    if (current_block_weight > 2 * median_weight) {
      MERROR("Block cumulative weight is too big: " << current_block_weight << ", expected less than " << 2 * median_weight);
      return false;
    }

    uint64_t product_hi;
    uint64_t multiplicand = 2 * median_weight - current_block_weight;
    multiplicand *= current_block_weight;
    uint64_t product_lo = mul128(base_reward, multiplicand, &product_hi);

    uint64_t reward_hi;
    uint64_t reward_lo;
    div128_64(product_hi, product_lo, median_weight, &reward_hi, &reward_lo, NULL, NULL);
    div128_64(reward_hi, reward_lo, median_weight, &reward_hi, &reward_lo, NULL, NULL);
    assert(0 == reward_hi);
    assert(reward_lo < base_reward);

    reward = reward_lo;
    return true;
  }
  //------------------------------------------------------------------------------------
  uint8_t get_account_address_checksum(const public_address_outer_blob& bl)
  {
    const unsigned char* pbuf = reinterpret_cast<const unsigned char*>(&bl);
    uint8_t summ = 0;
    for (size_t i = 0; i != sizeof(public_address_outer_blob) - 1; i++) {
      summ += pbuf[i];
    }

    return summ;
  }
  //------------------------------------------------------------------------------------
  uint8_t get_account_integrated_address_checksum(const public_integrated_address_outer_blob& bl)
  {
    const unsigned char* pbuf = reinterpret_cast<const unsigned char*>(&bl);
    uint8_t summ = 0;
    for (size_t i = 0; i != sizeof(public_integrated_address_outer_blob) - 1; i++) {
      summ += pbuf[i];
    }

    return summ;
  }
  //-----------------------------------------------------------------------
  std::string get_account_address_as_str(
      network_type nettype
    , bool subaddress
    , const account_public_address& adr
    )
  {
    uint64_t address_prefix = subaddress ? get_config(nettype).CRYPTONOTE_PUBLIC_SUBADDRESS_BASE58_PREFIX : get_config(nettype).CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX;

    return tools::base58::encode_addr(address_prefix, t_serializable_object_to_blob(adr));
  }
  //-----------------------------------------------------------------------
  std::string get_account_integrated_address_as_str(
      network_type nettype
    , const account_public_address& adr
    , const crypto::hash8& payment_id
    )
  {
    uint64_t integrated_address_prefix = get_config(nettype).CRYPTONOTE_PUBLIC_INTEGRATED_ADDRESS_BASE58_PREFIX;

    integrated_address iadr = {
      adr, payment_id
    };
    return tools::base58::encode_addr(integrated_address_prefix, t_serializable_object_to_blob(iadr));
  }
  //-----------------------------------------------------------------------
  bool is_coinbase(const transaction& tx)
  {
    if (tx.vin.size() != 1) {
      return false;
    }

    if (tx.vin[0].type() != typeid(txin_gen)) {
      return false;
    }

    return true;
  }
  //-----------------------------------------------------------------------
  bool get_account_address_from_str(
      address_parse_info& info
    , network_type nettype
    , const std::string& str
    )
  {
    uint64_t address_prefix = get_config(nettype).CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX;
    uint64_t integrated_address_prefix = get_config(nettype).CRYPTONOTE_PUBLIC_INTEGRATED_ADDRESS_BASE58_PREFIX;
    uint64_t subaddress_prefix = get_config(nettype).CRYPTONOTE_PUBLIC_SUBADDRESS_BASE58_PREFIX;

    if (str.size() < 2 * sizeof(public_address_outer_blob)) {
      blobdata data;
      uint64_t prefix;
      if (!tools::base58::decode_addr(str, prefix, data)) {
        LOG_PRINT_L2("Invalid address format");
        return false;
      }

      if (integrated_address_prefix == prefix) {
        info.is_subaddress = false;
        info.has_payment_id = true;
      } else if (address_prefix == prefix) {
        info.is_subaddress = false;
        info.has_payment_id = false;
      } else if (subaddress_prefix == prefix) {
        info.is_subaddress = true;
        info.has_payment_id = false;
      } else {
        LOG_PRINT_L1("Wrong address prefix: " << prefix << ", expected " << address_prefix
          << " or " << integrated_address_prefix
          << " or " << subaddress_prefix);
        return false;
      }

      if (info.has_payment_id) {
        integrated_address iadr;
        if (!::serialization::parse_binary(data, iadr)) {
          LOG_PRINT_L1("Account public address keys can't be parsed");
          return false;
        }
        info.address = iadr.adr;
        info.payment_id = iadr.payment_id;
      } else {
        if (!::serialization::parse_binary(data, info.address)) {
          LOG_PRINT_L1("Account public address keys can't be parsed");
          return false;
        }
      }

      if (!crypto::check_key(info.address.m_spend_public_key) || !crypto::check_key(info.address.m_view_public_key)) {
        LOG_PRINT_L1("Failed to validate address keys");
        return false;
      }
    } else {
      // Old address format
      std::string buff;
      if (!string_tools::parse_hexstr_to_binbuff(str, buff)) {
        return false;
      }

      if (buff.size() != sizeof(public_address_outer_blob)) {
        LOG_PRINT_L1("Wrong address length: " << buff.size() << ", expected " << sizeof(public_address_outer_blob));
        return false;
      }

      public_address_outer_blob blob;
      std::copy(buff.begin(), buff.end(), reinterpret_cast<char*>(&blob));

      if (blob.m_ver > CRYPTONOTE_PUBLIC_ADDRESS_TEXTBLOB_VER) {
        LOG_PRINT_L1("Wrong address format");
        return false;
      }

      if (blob.check_sum != get_account_address_checksum(blob)) {
        LOG_PRINT_L1("Invalid checksum");
        return false;
      }

      info.address = blob.m_address;
      info.is_subaddress = false;
      info.has_payment_id = false;

      if (!crypto::check_key(info.address.m_spend_public_key) || !crypto::check_key(info.address.m_view_public_key)) {
        LOG_PRINT_L1("Failed to validate address keys");
        return false;
      }
    }

    return true;
  }
  //-----------------------------------------------------------------------
  bool get_account_address_from_str_or_url(
      address_parse_info& info
    , network_type nettype
    , const std::string& str_or_url
    , std::function<std::string(const std::string&)> dns_confirm
    )
  {
    if (get_account_address_from_str(info, nettype, str_or_url)) {
      return true;
    }

    std::vector<std::string> addresses = { str_or_url };
    if (!tools::dns_utils::load_txt_records_from_dns(addresses, addresses)) {
      return false;
    }

    for (const auto& addr : addresses) {
      if (get_account_address_from_str(info, nettype, addr)) {
        return true;
      }
    }

    return false;
  }

  bool operator ==(const cryptonote::transaction& a, const cryptonote::transaction& b) {
    return cryptonote::get_transaction_hash(a) == cryptonote::get_transaction_hash(b);
  }

  bool operator ==(const cryptonote::block& a, const cryptonote::block& b) {
    return cryptonote::get_block_hash(a) == cryptonote::get_block_hash(b);
  }

  bool construct_miner_tx(
    size_t height,
    size_t median_weight,
    uint64_t already_generated_coins,
    size_t current_block_weight,
    uint64_t fee,
    const account_public_address& miner_address,
    transaction& tx,
    const blobdata& extra_nonce,
    size_t max_outs,
    uint8_t hf_version
  ) {
    tx.vin.clear();
    tx.vout.clear();
    tx.extra.clear();

    keypair txkey = keypair::generate(hw::get_device("default"));
    add_tx_pub_key_to_extra(tx, txkey.pub);
    if (!extra_nonce.empty())
      if (!add_extra_nonce_to_tx_extra(tx.extra, extra_nonce))
        return false;

    txin_gen in;
    in.height = height;
    tx.vin.push_back(in);

    uint64_t block_reward;
    if (!get_block_reward(median_weight, current_block_weight, already_generated_coins, block_reward, hf_version)) {
      LOG_PRINT_L0("Block is too big");
      return false;
    }

    block_reward += fee;

    std::vector<uint64_t> out_amounts;
    decompose_amount_into_digits(block_reward, 0,
      [&out_amounts](uint64_t a_chunk) { out_amounts.push_back(a_chunk); },
      [](uint64_t) {});

    const uint8_t HF_VERSION_REDUCED_BLOCK_REWARD = 10; // Example version, change as needed
    if (max_outs != 0 && hf_version >= HF_VERSION_REDUCED_BLOCK_REWARD) {
      while (out_amounts.size() > max_outs) {
        out_amounts[out_amounts.size() - 2] += out_amounts.back();
        out_amounts.pop_back();
      }
    }

    uint64_t summary_amounts = 0;
    for (auto &da : out_amounts) {
      summary_amounts += da;
      txout_to_key tk;
      tk.key = rct::rct2pk(rct::scalarmultKey(rct::pk2rct(miner_address.m_spend_public_key), rct::sk2rct(txkey.sec)));
      tx_out out;
      out.amount = da;
      out.target = tk;
      tx.vout.push_back(out);
    }

    if (summary_amounts != block_reward) {
      LOG_PRINT_L0("Decomposed amounts sum is not equal to expected amount: " << summary_amounts << " != " << block_reward);
      return false;
    }

    tx.version = CURRENT_TRANSACTION_VERSION;
    tx.unlock_time = height + CRYPTONOTE_MINED_MONEY_UNLOCK_WINDOW;
    return true;
  }
}

//--------------------------------------------------------------------------------
bool parse_hash256(const std::string &str_hash, crypto::hash& hash)
{
  std::string buf;
  bool res = epee::string_tools::parse_hexstr_to_binbuff(str_hash, buf);
  if (!res || buf.size() != sizeof(crypto::hash)) {
    MERROR("invalid hash format: " << str_hash);
    return false;
  } else {
    buf.copy(reinterpret_cast<char *>(&hash), sizeof(crypto::hash));
    return true;
  }
}
