#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/multi_index.hpp>
#include <eosio.system/eosio.system.hpp>

using eosio::asset;
using eosio::key256;
using std::string;

static const account_name ecaf_account = N(ecafofficial);
static const account_name code_account = N(theblacklist);

class theblacklist_contract : public eosio::contract {
 public:
  theblacklist_contract(account_name self) :
    eosio::contract(self){}

  std::vector<string> actions = {"add", "remove"};

  std::vector<string> types = {"actor-blacklist",
                               "actor-whitelist",
                               "contract-blacklist",
                               "contract-whitelist",
                               "key-blacklist",
                               "action-blacklist"};

  bool contains(const std::vector<string>& array,
                const string item) {
    bool result = (std::find(array.begin(), array.end(), item) != array.end());
    return result;
  }

  // ACTION: setorder
  // add ECAF order to theblacklist table
  void setorder(const string order_name,
                const string order_url,
                const string order_hash,
                const string action,
                const string type,
                const std::vector<account_name>& accounts) {
    require_auth(ecaf_account);
    eosio_assert(contains(actions, action) == true, "invalid action");
    eosio_assert(contains(types, type) == true, "invalid type");
    theblacklist_table existing(code_account, code_account);
    existing.emplace(code_account, [&](auto& j) {
      j.id = existing.available_primary_key();
      j.order_name = order_name;
      j.order_url = order_url;
      j.order_hash = order_hash;
      j.action = action;
      j.type = type;
      j.accounts = accounts;
    });
  }

  // ACTION: sethash
  // add/update producer blacklist hash entry
  void sethash(const account_name producer,
                const checksum256& hash) {

    require_auth(producer);

    // Check producer info table. producer should exist (has called regproducer) and be activated.
    typedef eosio::multi_index<N(producers), eosiosystem::producer_info> producer_info_t;
    producer_info_t _producers(N(eosio), N(eosio));
    auto prod = _producers.get(producer, "user is not a producer");
    eosio_assert(prod.is_active == true, "user is not an active producer");
    eosio_assert(prod.total_votes > 0.0, "user is not an active producer");

    // Add/Update hash entry
    producerhash_table hashtable(_self, _self);
    auto idx = hashtable.get_index<N(producer)>();
    auto itr = idx.find(producer);
    if (itr != idx.end()) {
      idx.modify(itr, producer, [&](auto& prod_hash) {
        prod_hash.hash = hash;
      });
    } else {
      hashtable.emplace(_self, [&](auto& prod_hash) {
        prod_hash.id = hashtable.available_primary_key();
        prod_hash.producer = producer;
        prod_hash.hash = hash;
      });
    }
  }

  // ACTION: delhash
  // delete producer blacklist hash entry
  void delhash(const account_name producer) {

    require_auth(producer);

    producerhash_table hashtable(_self, _self);
    auto idx = hashtable.get_index<N(producer)>();
    auto itr = idx.find(producer);
    // delete entry if exist
    if (itr != idx.end()) {
      idx.erase(itr);
    } 
  }


  // for DEBUG only, should be removed when deployed online.
  // Delete all ECAF blacklist entries.
  void clear() {
    require_auth(ecaf_account);
    theblacklist_table existing(code_account, code_account);
    while (existing.begin() != existing.end()) {
        auto itr = existing.end();
        itr--;
        existing.erase(itr);
        theblacklist_table existing(code_account, code_account);
    }
  }

 private:
  // @abi table theblacklist i64
  struct theblacklist {
    uint64_t                    id;
    string                      order_name; // order name, eg. for ECAF Order 001, order_name should be 'ECAF_Arbitrator_Order_2018-06-19-AO-001'.
    string                      order_url; // order pdf link, eg. for ECAF Order 001, order_url should be 'https://eoscorearbitration.io/wp-content/uploads/2018/07/ECAF_Arbitrator_Order_2018-06-19-AO-001.pdf'.
    string                      order_hash; // SHA3-256 hash, eg. for ECAF Order 001, order_hash should be 'a80df3e8cfa895a02161dc4d5d04392e3274bce917935c6c214cfe0f1f7e868a'.
    string                      action; // action is a choice field, valid choices are 'add' and 'remove', meaning add or remove accounts for certain type.
    string                      type; // type is a choice field, valid choices are: 'actor-blacklist', 'actor-whitelist', 'contract-blacklist', 'contract-whitelist', 'action-blacklist', 'key-blacklist'.
    std::vector<account_name>   accounts;

    auto primary_key() const {  return id;  }
    EOSLIB_SERIALIZE(theblacklist, (id)(order_name)(order_url)(order_hash)(action)(type)(accounts))
  };
  typedef eosio::multi_index<N(theblacklist_table), theblacklist> theblacklist_table;
  //theblacklist_table theblacklist;

  // @abi table producerhash i64
  struct producerhash {
    uint64_t                    id;
    account_name                producer;
    checksum256                 hash; // hash of blacklist conf running `calculate_blacklist_hash.sh`

    auto primary_key() const {  return id;  }
    account_name get_producer() const { return producer; }
    EOSLIB_SERIALIZE(producerhash, (id)(producer)(hash))
  };
  typedef eosio::multi_index<N(producerhash_table), producerhash, eosio::indexed_by< N(producer), eosio::const_mem_fun<producerhash, account_name, &producerhash::get_producer>>> producerhash_table;
};

EOSIO_ABI(theblacklist_contract, (setorder)(clear)(sethash)(delhash))
