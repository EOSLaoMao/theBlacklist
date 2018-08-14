#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/multi_index.hpp>
#include <eosio.system/eosio.system.hpp>

using eosio::asset;
using std::string;

//static const account_name ecaf_account = N(ecafofficial);
static const account_name ecaf_account = N(theblacklist);
static const account_name code_account = N(theblacklist);

class theblacklist_contract : public eosio::contract {
 public:
  theblacklist_contract(account_name self) :
    eosio::contract(self),
    theblacklist(_self, _self) {}

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

  // set blacklisted accounts.
  void set(const string order_name,
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

  // for DEBUG only, should be removed when deployed online: Delete all blacklist entries.
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
    // checksum256                 order_tx; // transaction id contained ECAF Order signed by account ecafofficial.
    string                      action; // action is a choice field, valid choices are 'add' and 'remove', meaning add or remove accounts for certain type.
    string                      type; // type is a choice field, valid choices are: 'actor-blacklist', 'actor-whitelist', 'contract-blacklist', 'contract-whitelist', 'action-blacklist', 'key-blacklist'.
    std::vector<account_name>   accounts;

    auto primary_key() const {  return id;  }
    EOSLIB_SERIALIZE(theblacklist, (id)(order_name)(order_url)(order_hash)(action)(type)(accounts))
  };
  typedef eosio::multi_index<N(theblacklist), theblacklist> theblacklist_table;
  theblacklist_table theblacklist;
};

EOSIO_ABI(theblacklist_contract, (set)(clear))
