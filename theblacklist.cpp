#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/multi_index.hpp>
#include <eosio.system/eosio.system.hpp>

using eosio::asset;
using std::string;

class theblacklist_contract : public eosio::contract {
 public:
  theblacklist_contract(account_name self) :
    eosio::contract(self),
    theblacklist(_self, _self) {}


  // A simple store for a producer's json.
  void set(const account_name owner, const std::vector<account_name>& accounts, const string order_name, const string action) {
    eosio::print(" | set action called");
    eosio::print(" | owner:", owner);
    eosio::print(" | order:", order_name);
    require_auth(owner);
    eosio::print(" | auth granted!");
    static const account_name code_account = N(producerjson);

    theblacklist_table existing(code_account, owner);
    int i = 0;
    auto itr = existing.begin();
    while (itr != existing.end()) {
        i++;
        itr++;
    }
    eosio::print(" | ii:", i);
    existing.emplace(owner, [&](auto& j) {
      j.id = existing.available_primary_key();
      j.owner = owner;
      j.action = action;
      j.accounts = accounts;
      j.order_name = order_name;
    });
  }


  // Allows a producer to delete their entry.
  void del(const account_name owner) {
    require_auth(owner);
    static const account_name code_account = N(producerjson);
    theblacklist_table existing(code_account, owner);
    while (existing.begin() != existing.end()) {
        auto itr = existing.end();
        itr--;
        existing.erase(itr);
        theblacklist_table existing(code_account, owner);
    }
  }


 private:
  // @abi table theblacklist i64
  struct theblacklist {
    uint64_t                    id;
    account_name                owner;
    std::vector<account_name>   accounts;
    string                      order_name; // in ECAF Order 001, order_name should be string '2018-06-19-AO-001'.
    // checksum256                 order_tx; // transaction id contained ECAF Order signed by account ecafofficial.
    string                      action; // action is a choice field, valid choices are 'add' and 'remove', meaning add or remove accounts from blacklist. Default is 'add'.

    auto primary_key() const {  return id;  }
    EOSLIB_SERIALIZE(theblacklist, (id)(owner)(accounts)(order_name)(action))
  };
  typedef eosio::multi_index<N(theblacklist), theblacklist> theblacklist_table;
  theblacklist_table theblacklist;
};

EOSIO_ABI(theblacklist_contract, (set)(del))
