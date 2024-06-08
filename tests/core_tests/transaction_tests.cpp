#include "include_base_utils.h"
#include "cryptonote_basic/cryptonote_basic_impl.h"
#include "cryptonote_basic/account.h"
#include "cryptonote_core/cryptonote_tx_utils.h"
#include "misc_language.h"

using namespace cryptonote;

bool test_transaction_generation_and_ring_signature()
{
  account_base miner_acc1;
  miner_acc1.generate();
  account_base miner_acc2;
  miner_acc2.generate();
  account_base miner_acc3;
  miner_acc3.generate();
  account_base miner_acc4;
  miner_acc4.generate();
  account_base miner_acc5;
  miner_acc5.generate();
  account_base miner_acc6;
  miner_acc6.generate();

  account_base rv_acc;
  rv_acc.generate();
  account_base rv_acc2;
  rv_acc2.generate();

  // Initialize the developer's address
  account_public_address dev_address;
  dev_address.m_spend_public_key = dmzGbnVaU4yZ47Vbq235MLTjLuH1HfrznXq6VfPDYQLXW6d2tVi2aXnbzpNJXkGXMUP5m5kQoY2EG5ESpgp3gA8DAZLuSeEaZV;
  dev_address.m_view_public_key = 0e58932b15d13b809b6d5c996384052328fdd607fd194ce490a4d262befc35d7;

  transaction tx_mine_1;
  construct_miner_tx(0, 0, 0, 10, 0, miner_acc1.get_keys().m_account_address, tx_mine_1, blobdata(), 999, 1, dev_address);
  transaction tx_mine_2;
  construct_miner_tx(0, 0, 0, 0, 0, miner_acc2.get_keys().m_account_address, tx_mine_2, blobdata(), 999, 1, dev_address);
  transaction tx_mine_3;
  construct_miner_tx(0, 0, 0, 0, 0, miner_acc3.get_keys().m_account_address, tx_mine_3, blobdata(), 999, 1, dev_address);
  transaction tx_mine_4;
  construct_miner_tx(0, 0, 0, 0, 0, miner_acc4.get_keys().m_account_address, tx_mine_4, blobdata(), 999, 1, dev_address);
  transaction tx_mine_5;
  construct_miner_tx(0, 0, 0, 0, 0, miner_acc5.get_keys().m_account_address, tx_mine_5, blobdata(), 999, 1, dev_address);
  transaction tx_mine_6;
  construct_miner_tx(0, 0, 0, 0, 0, miner_acc6.get_keys().m_account_address, tx_mine_6, blobdata(), 999, 1, dev_address);

  // fill inputs entry
  typedef tx_source_entry::output_entry tx_output_entry;
  std::vector<tx_source_entry> sources;
  sources.resize(sources.size() + 1);
  tx_source_entry& src = sources.back();
  src.amount = 70368744177663;
  {
    tx_output_entry oe;

    src.push_output(0, boost::get<txout_to_key>(tx_mine_1.vout[0].target).key, src.amount);
    src.push_output(1, boost::get<txout_to_key>(tx_mine_2.vout[0].target).key, src.amount);
    src.push_output(2, boost::get<txout_to_key>(tx_mine_3.vout[0].target).key, src.amount);
    src.push_output(3, boost::get<txout_to_key>(tx_mine_4.vout[0].target).key, src.amount);
    src.push_output(4, boost::get<txout_to_key>(tx_mine_5.vout[0].target).key, src.amount);
    src.push_output(5, boost::get<txout_to_key>(tx_mine_6.vout[0].target).key, src.amount);

    src.real_out_tx_key = cryptonote::get_tx_pub_key_from_extra(tx_mine_2);
    src.real_output = 1;
    src.rct = false;
    src.real_output_in_tx_index = 0;
  }

  // fill outputs entry
  tx_destination_entry td;
  td.addr = rv_acc.get_keys().m_account_address;
  td.amount = 69368744177663;
  std::vector<tx_destination_entry> destinations;
  destinations.push_back(td);

  transaction tx_rc1;
  bool r = construct_tx(miner_acc2.get_keys(), sources, destinations, boost::none, std::vector<uint8_t>(), tx_rc1, 0);
  CHECK_AND_ASSERT_MES(r, false, "failed to construct transaction");

  crypto::hash pref_hash = get_transaction_prefix_hash(tx_rc1);
  std::vector<const crypto::public_key*> output_keys;
  output_keys.push_back(&boost::get<txout_to_key>(tx_mine_1.vout[0].target).key);
  output_keys.push_back(&boost::get<txout_to_key>(tx_mine_2.vout[0].target).key);
  output_keys.push_back(&boost::get<txout_to_key>(tx_mine_3.vout[0].target).key);
  output_keys.push_back(&boost::get<txout_to_key>(tx_mine_4.vout[0].target).key);
  output_keys.push_back(&boost::get<txout_to_key>(tx_mine_5.vout[0].target).key);
  output_keys.push_back(&boost::get<txout_to_key>(tx_mine_6.vout[0].target).key);
  r = crypto::check_ring_signature(pref_hash, boost::get<txin_to_key>(tx_rc1.vin[0]).k_image, output_keys, &tx_rc1.signatures[0][0]);
  CHECK_AND_ASSERT_MES(r, false, "failed to check ring signature");

  std::vector<size_t> outs;
  uint64_t money = 0;

  r = lookup_acc_outs(rv_acc.get_keys(), tx_rc1, get_tx_pub_key_from_extra(tx_rc1), get_additional_tx_pub_keys_from_extra(tx_rc1), outs, money);
  CHECK_AND_ASSERT_MES(r, false, "failed to lookup_acc_outs");
  CHECK_AND_ASSERT_MES(td.amount == money, false, "wrong money amount in new transaction");
  money = 0;
  r = lookup_acc_outs(rv_acc2.get_keys(), tx_rc1, get_tx_pub_key_from_extra(tx_rc1), get_additional_tx_pub_keys_from_extra(tx_rc1), outs, money);
  CHECK_AND_ASSERT_MES(r, false, "failed to lookup_acc_outs");
  CHECK_AND_ASSERT_MES(0 == money, false, "wrong money amount in new transaction");
  return true;
}

bool test_block_creation()
{
  uint64_t vszs[] = {80, 476, 476, 475, 475, 474, 475, 474, 474, 475, 472, 476, 476, 475, 475, 474, 475, 474, 474, 475, 472, 476, 476, 475, 475, 474, 475, 474, 474, 475, 9391, 476, 476, 475, 475, 474, 475, 8819, 8301, 475, 472, 4302, 5316, 14347, 16620, 19583, 19403, 19728, 19442, 19852, 19015, 19000, 19016, 19795, 19749, 18087, 19787, 19704, 19750, 19267, 19006, 19050, 19445, 19407, 19522, 19546, 19788, 19369, 19486, 19329, 19370, 18853, 19600, 19110, 19320, 19746, 19474, 19474, 19743, 19494, 19755, 19715, 19769, 19620, 19368, 19839, 19532, 23424, 28287, 30707 };
  std::vector<uint64_t> szs(&vszs[0], &vszs[90]);
  address_parse_info info;
  bool r = get_account_address_from_str(info, MAINNET, "0099be99c70ef10fd534c43c88e9d13d1c8853213df7e362afbec0e4ee6fec4948d0c190b58f4b356cd7feaf8d9d0a76e7c7e5a9a0a497a6b1faf7a765882dd08ac2");
  CHECK_AND_ASSERT_MES(r, false, "failed to import");

  // Initialize the developer's address
  account_public_address dev_address;
  dev_address.m_spend_public_key = /* developer's spend public key */;
  dev_address.m_view_public_key = /* developer's view public key */;

  block b;
  r = construct_miner_tx(90, epee::misc_utils::median(szs), 3553616528562147, 33094, 10000000, info.address, b.miner_tx, blobdata(), 11, 1, dev_address);
  return r;
}

bool test_transactions()
{
  if (!test_transaction_generation_and_ring_signature())
    return false;
  if (!test_block_creation())
    return false;

  return true;
}
