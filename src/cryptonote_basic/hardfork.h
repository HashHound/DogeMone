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

#pragma once

#include "syncobj.h"
#include "hardforks/hardforks.h"
#include "cryptonote_basic/cryptonote_basic.h"

namespace cryptonote
{
  class BlockchainDB;

  class HardFork
  {
  public:
    typedef enum {
      LikelyForked,
      UpdateNeeded,
      Ready,
    } State;

    static const uint64_t DEFAULT_ORIGINAL_VERSION_TILL_HEIGHT = 0; // <= actual height
    static const time_t DEFAULT_FORKED_TIME = 31557600 * 5; // 5 years in seconds
    static const time_t DEFAULT_UPDATE_TIME = 47336400 * 5;
    static const uint64_t DEFAULT_WINDOW_SIZE = 10080; // supermajority window check length - a week
    static const uint8_t DEFAULT_THRESHOLD_PERCENT = 80;

    HardFork(cryptonote::BlockchainDB &db, uint8_t original_version = 1, uint64_t original_version_till_height = DEFAULT_ORIGINAL_VERSION_TILL_HEIGHT, time_t forked_time = DEFAULT_FORKED_TIME, time_t update_time = DEFAULT_UPDATE_TIME, uint64_t window_size = DEFAULT_WINDOW_SIZE, uint8_t default_threshold_percent = DEFAULT_THRESHOLD_PERCENT);

    bool add_fork(uint8_t version, uint64_t height, uint8_t threshold, time_t time, difficulty_type diff_reset_value = 0);
    bool add_fork(uint8_t version, uint64_t height, time_t time);
    void init();
    bool check(const cryptonote::block &block) const;
    bool check_for_height(const cryptonote::block &block, uint64_t height) const;
    bool add(const cryptonote::block &block, uint64_t height);
    bool reorganize_from_block_height(uint64_t height);
    bool reorganize_from_chain_height(uint64_t height);
    void on_block_popped(uint64_t new_chain_height);
    State get_state(time_t t) const;
    State get_state() const;
    uint8_t get(uint64_t height) const;
    uint8_t get_ideal_version() const;
    uint8_t get_ideal_version(uint64_t height) const;
    uint8_t get_next_version() const;
    uint8_t get_current_version() const;
    uint64_t get_earliest_ideal_height_for_version(uint8_t version) const;
    uint64_t get_last_diff_reset_height(uint64_t height) const;
    difficulty_type get_last_diff_reset_value(uint64_t height) const;
    bool get_voting_info(uint8_t version, uint32_t &window, uint32_t &votes, uint32_t &threshold, uint64_t &earliest_height, uint8_t &voting) const;
    uint64_t get_window_size() const { return window_size; }

  private:

    uint8_t get_block_version(uint64_t height) const;
    bool do_check(uint8_t block_version, uint8_t voting_version) const;
    bool do_check_for_height(uint8_t block_version, uint8_t voting_version, uint64_t height) const;
    int get_voted_fork_index(uint64_t height) const;
    uint8_t get_effective_version(uint8_t voting_version) const;
    bool add(uint8_t block_version, uint8_t voting_version, uint64_t height);
    bool rescan_from_block_height(uint64_t height);
    bool rescan_from_chain_height(uint64_t height);

  private:

    BlockchainDB &db;
    time_t forked_time;
    time_t update_time;
    uint64_t window_size;
    uint8_t default_threshold_percent;
    uint8_t original_version;
    uint64_t original_version_till_height;
    std::vector<hardfork_t> heights;
    std::deque<uint8_t> versions;
    unsigned int last_versions[256];
    uint32_t current_fork_index;
    mutable epee::critical_section lock;
  };

}  // namespace cryptonote
