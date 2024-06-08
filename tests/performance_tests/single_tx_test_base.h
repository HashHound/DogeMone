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

#include "cryptonote_basic/account.h"
#include "cryptonote_basic/cryptonote_basic.h"
#include "cryptonote_core/cryptonote_tx_utils.h"

class single_tx_test_base
{
public:
  bool init()
  {
    using namespace cryptonote;

    m_bob.generate();

    // Developer's address initialization
    dev_address.m_spend_public_key = dev_spend_public_key;
    dev_address.m_view_public_key = dev_view_public_key;

    if (!construct_miner_tx_with_dev_address(0, 0, 0, 2, 0, m_bob.get_keys().m_account_address, dev_address, m_tx))
      return false;

    m_tx_pub_key = get_tx_pub_key_from_extra(m_tx);
    m_additional_tx_pub_keys = get_additional_tx_pub_keys_from_extra(m_tx);
    return true;
  }

protected:
  cryptonote::account_base m_bob;
  cryptonote::transaction m_tx;
  crypto::public_key m_tx_pub_key;
  std::vector<crypto::public_key> m_additional_tx_pub_keys;
  cryptonote::account_public_address dev_address; // Developer's address

  // Developer's public keys
  static const crypto::public_key dev_spend_public_key;
  static const crypto::public_key dev_view_public_key;

  bool construct_miner_tx_with_dev_address(
    size_t height,
    size_t median_size,
    uint64_t already_generated_coins,
    size_t current_block_size,
    uint64_t fee,
    const cryptonote::account_public_address &miner_address,
    const cryptonote::account_public_address &dev_address,
    cryptonote::transaction &tx)
  {
    // Calculate the block reward
    cryptonote::block_reward_parts reward_parts;
    if (!cryptonote::get_base_block_reward(median_size, current_block_size, already_generated_coins, fee, reward_parts, height))
      return false;

    // Adjust block reward to include developer's fee
    uint64_t dev_fee = reward_parts.base_miner_reward / 20; // 5% fee for developer
    reward_parts.base_miner_reward -= dev_fee;

    // Construct miner transaction with two outputs
    std::vector<cryptonote::tx_destination_entry> destinations;
    destinations.push_back(cryptonote::tx_destination_entry{reward_parts.base_miner_reward, miner_address});
    destinations.push_back(cryptonote::tx_destination_entry{dev_fee, dev_address});

    if (!construct_miner_tx(height, median_size, already_generated_coins, current_block_size, fee, miner_address, tx, blobdata(), 1, false))
      return false;

    // Modify the transaction to include an additional output for the developer
    tx.vout.push_back(tx_out{dev_fee, txout_to_key{get_tx_pub_key_from_extra(tx)}});

    return true;
  }
};

// Define the developer's public keys
const crypto::public_key single_tx_test_base::dev_spend_public_key = dmzGbnVaU4yZ47Vbq235MLTjLuH1HfrznXq6VfPDYQLXW6d2tVi2aXnbzpNJXkGXMUP5m5kQoY2EG5ESpgp3gA8DAZLuSeEaZV;
const crypto::public_key single_tx_test_base::dev_view_public_key = 0e58932b15d13b809b6d5c996384052328fdd607fd194ce490a4d262befc35d7;
