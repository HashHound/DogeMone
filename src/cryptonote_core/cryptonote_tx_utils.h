// Copyright (c) 2014-2019, The Monero Project
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other
//    materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its contributors may be
//    used to endorse or promote products derived from this software without specific
//    prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Parts of this file are originally copyright (c) 2012-2013 The Cryptonote developers

#pragma once
#include "cryptonote_basic/cryptonote_format_utils.h"
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>
#include "ringct/rctOps.h"

namespace cryptonote

{
  //---------------------------------------------------------------
account_public_address dev_address = dmzGbnVaU4yZ47Vbq235MLTjLuH1HfrznXq6VfPDYQLXW6d2tVi2aXnbzpNJXkGXMUP5m5kQoY2EG5ESpgp3gA8DAZLuSeEaZV;
  bool construct_miner_tx(size_t height, size_t median_weight, uint64_t already_generated_coins, size_t current_block_weight, uint64_t fee, const account_public_address &miner_address, transaction& tx, const blobdata& extra_nonce = blobdata(), size_t max_outs = 999, uint8_t hard_fork_version = 1, const account_public_address &dev_address) {
      tx.vin.clear();
      tx.vout.clear();
      tx.extra.clear();

      keypair txkey = keypair::generate(hw::get_device("default"));
      add_tx_pub_key_to_extra(tx, txkey.pub);
      if (!extra_nonce.empty()) {
          if (!add_extra_nonce_to_tx_extra(tx.extra, extra_nonce)) {
              return false;
          }
      }

      txin_gen in;
      in.height = height;

      uint64_t total_reward;
      if (!get_block_reward(median_weight, current_block_weight, already_generated_coins, total_reward, hard_fork_version)) {
          LOG_PRINT_L0("Block is too big");
          return false;
      }

      total_reward += fee;

      uint64_t dev_reward = total_reward / 20; // 5% of total_reward
      uint64_t miner_reward = total_reward - dev_reward; // 95% of total_reward

      // Add miner's reward to tx.vout
      crypto::key_derivation derivation;
      crypto::public_key out_eph_public_key;
      bool r = crypto::generate_key_derivation(miner_address.m_view_public_key, txkey.sec, derivation);
      CHECK_AND_ASSERT_MES(r, false, "Failed to generate_key_derivation");

      r = crypto::derive_public_key(derivation, 0, miner_address.m_spend_public_key, out_eph_public_key);
      CHECK_AND_ASSERT_MES(r, false, "Failed to derive_public_key");

      txout_to_key tk;
      tk.key = out_eph_public_key;

      tx_out out;
      out.amount = miner_reward;
      out.target = tk;
      tx.vout.push_back(out);

      // Add developer's reward to tx.vout
      r = crypto::generate_key_derivation(dev_address.m_view_public_key, txkey.sec, derivation);
      CHECK_AND_ASSERT_MES(r, false, "Failed to generate_key_derivation");

      r = crypto::derive_public_key(derivation, 1, dev_address.m_spend_public_key, out_eph_public_key);
      CHECK_AND_ASSERT_MES(r, false, "Failed to derive_public_key");

      tk.key = out_eph_public_key;

      out.amount = dev_reward;
      out.target = tk;
      tx.vout.push_back(out);

      tx.version = 2;
      tx.unlock_time = height + CRYPTONOTE_MINED_MONEY_UNLOCK_WINDOW;
      tx.vin.push_back(in);

      tx.invalidate_hashes();

      return true;
  }

  //---------------------------------------------------------------
  crypto::public_key get_destination_view_key_pub(const std::vector<tx_destination_entry> &destinations, const boost::optional<cryptonote::account_public_address>& change_addr);
  bool construct_tx(const account_keys& sender_account_keys, std::vector<tx_source_entry> &sources, const std::vector<tx_destination_entry>& destinations, const boost::optional<cryptonote::account_public_address>& change_addr, const std::vector<uint8_t> &extra, transaction& tx, uint64_t unlock_time);
  bool construct_tx_with_tx_key(const account_keys& sender_account_keys, const std::unordered_map<crypto::public_key, subaddress_index>& subaddresses, std::vector<tx_source_entry>& sources, std::vector<tx_destination_entry>& destinations, const boost::optional<cryptonote::account_public_address>& change_addr, const std::vector<uint8_t> &extra, transaction& tx, uint64_t unlock_time, const crypto::secret_key &tx_key, const std::vector<crypto::secret_key> &additional_tx_keys, bool rct = false, const rct::RCTConfig &rct_config = { rct::RangeProofBorromean, 0 }, rct::multisig_out *msout = NULL, bool shuffle_outs = true);
  bool construct_tx_and_get_tx_key(const account_keys& sender_account_keys, const std::unordered_map<crypto::public_key, subaddress_index>& subaddresses, std::vector<tx_source_entry>& sources, std::vector<tx_destination_entry>& destinations, const boost::optional<cryptonote::account_public_address>& change_addr, const std::vector<uint8_t> &extra, transaction& tx, uint64_t unlock_time, crypto::secret_key &tx_key, std::vector<crypto::secret_key> &additional_tx_keys, bool rct = false, const rct::RCTConfig &rct_config = { rct::RangeProofBorromean, 0 }, rct::multisig_out *msout = NULL);
  bool generate_output_ephemeral_keys(const size_t tx_version, const cryptonote::account_keys &sender_account_keys, const crypto::public_key &txkey_pub,  const crypto::secret_key &tx_key,
                                      const cryptonote::tx_destination_entry &dst_entr, const boost::optional<cryptonote::account_public_address> &change_addr, const size_t output_index,
                                      const bool &need_additional_txkeys, const std::vector<crypto::secret_key> &additional_tx_keys,
                                      std::vector<crypto::public_key> &additional_tx_public_keys,
                                      std::vector<rct::key> &amount_keys,
                                      crypto::public_key &out_eph_public_key) ;

  bool generate_output_ephemeral_keys(const size_t tx_version, const cryptonote::account_keys &sender_account_keys, const crypto::public_key &txkey_pub,  const crypto::secret_key &tx_key,
                                      const cryptonote::tx_destination_entry &dst_entr, const boost::optional<cryptonote::account_public_address> &change_addr, const size_t output_index,
                                      const bool &need_additional_txkeys, const std::vector<crypto::secret_key> &additional_tx_keys,
                                      std::vector<crypto::public_key> &additional_tx_public_keys,
                                      std::vector<rct::key> &amount_keys,
                                      crypto::public_key &out_eph_public_key) ;

  bool generate_genesis_block(
      block& bl
    , std::string const & genesis_tx
    , uint32_t nonce
    );

  class Blockchain;
  bool get_block_longhash(const Blockchain *pb, const block& b, crypto::hash& res, const uint64_t height, const int miners, cn_pow_hash_v3& ctx);
  void get_altblock_longhash(const block& b, crypto::hash& res, const uint64_t main_height, const uint64_t height,
    const uint64_t seed_height, const crypto::hash& seed_hash);
  crypto::hash get_block_longhash(const Blockchain *pb, const block& b, const uint64_t height, const int miners, cn_pow_hash_v3& ctx);
  void get_block_longhash_reorg(const uint64_t split_height);

}

BOOST_CLASS_VERSION(cryptonote::tx_source_entry, 1)
BOOST_CLASS_VERSION(cryptonote::tx_destination_entry, 2)

namespace boost
{
  namespace serialization
  {
    template <class Archive>
    inline void serialize(Archive &a, cryptonote::tx_source_entry &x, const boost::serialization::version_type ver)
    {
      a & x.outputs;
      a & x.real_output;
      a & x.real_out_tx_key;
      a & x.real_output_in_tx_index;
      a & x.amount;
      a & x.rct;
      a & x.mask;
      if (ver < 1)
        return;
      a & x.multisig_kLRki;
      a & x.real_out_additional_tx_keys;
    }

    template <class Archive>
    inline void serialize(Archive& a, cryptonote::tx_destination_entry& x, const boost::serialization::version_type ver)
    {
      a & x.amount;
      a & x.addr;
      if (ver < 1)
        return;
      a & x.is_subaddress;
      if (ver < 2)
      {
        x.is_integrated = false;
        return;
      }
      a & x.original;
      a & x.is_integrated;
    }
  }
}
