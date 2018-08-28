#  `theblacklist` Smart Contract

Blacklist smart contract for EOS Community.

This contract is designed to be used both for ECAF and for active Block Producers on EOS.

# Design

`theblacklist` contract has two tables: 

1.`theblacklist` table, used to store ECAF blacklist orders.

2.`producerhash` table, used to store active producers blacklist hash.

AND three actions: 

1.`setorder` for `ecafofficial` account to add order entries containing blacklist accounts to table `theblacklist`.

2.`sethash` for active BPs to add their blacklist sha-256 hash to table `producerhash`

3.`delhash` for BPs to remove their hash entry from table `producerhash`

## table scheme

### table `theblacklist`

```
id			uint64;
order_name		string;
order_url 		string;
order_hash 		string;
action			string;
type			string;
accounts		name[];
```

1.`id` is an auto incremental field as primary key.

2.`order_name` is used to store ECAF order name, eg. for ECAF Order 001, `order_name` would be `ECAF_Arbitrator_Order_2018-06-19-AO-001`

3.`order_url` is used to store ECAF order pdf link, eg. for ECAF Order 001, `order_url` would be `https://eoscorearbitration.io/wp-content/uploads/2018/07/ECAF_Arbitrator_Order_2018-06-19-AO-001.pdf`

4.`order_hash` is used to store SHA3-256 of order pdf file, eg. for ECAF Order 001, `order_hash` would be `a80df3e8cfa895a02161dc4d5d04392e3274bce917935c6c214cfe0f1f7e868a`

5.`type` is a choice field used for different types of black/white list, choices are:

```
actor-blacklist
actor-whitelist
contract-blacklist
contract-whitelist
action-blacklist
key-blacklist
```

6.`action` is a choice field, valid choices are `add` and `remove`, meaning add or remove accounts from certain types of black/white list. 

Currently, all the orders came from ECAF only requested to `add` certain accounts to `actor-blacklist`, but as some disputes resolved in the future, it is possible that ECAF will issue orders to release certain accounts from `actor-blacklist`. By then, ECAF can set a new entry with `action` field value `remove` to release accounts.

7.`accounts` is an array of account, coming from ECAF order.

### table `producerhash`

```
id				uint64;
producer				name;
hash				checksum256;
```

1.`id` is an auto incremental field as primary key.

2.`producer` to store BP account name.

3.`hash` to store sha-256 sum of BP's blacklist config.


# Usage


### 1. Set ECAF order to `theblacklist` table


to set an ECAF order, simply call `setorder` action with `ecafofficial` auth:

```
cleos push action theblacklist setorder '{"accounts": [], "order_name": "", "action": "add/remove", "type": "", "order_url": "", "order_hash": ""}' -p ecafofficial@active
```

for example, for ECAF order 003(https://eoscorearbitration.io/wp-content/uploads/2018/07/ECAF-Temporary-Freeze-Order-2018-07-13-AO-003.pdf), just call:

```
cleos push action theblacklist setorder '{"accounts": ["neverlandwal", "tseol5n52kmo", "potus1111111"], "order_name": "ECAF-Temporary-Freeze-Order-2018-07-13-AO-003", "action": "add", "type": "actor-blacklist", "order_url": "https://eoscorearbitration.io/wp-content/uploads/2018/07/ECAF-Temporary-Freeze-Order-2018-07-13-AO-003.pdf", "order_hash": "ca104c57af040b5b46ab6fb2bcb8455ed8f81402e5e586d8a50a47cfc2683a20"}' -p ecafofficial@active
```

NOTE: There is a script `insert_ecaf_orders.sh` which contains all ECAF orders.

### 2. Add/update BP's blacklist hash to `producerhash`

as a BP, everytime you update your blacklist config, you should add/update the blacklist hash to `producerjson` table

This repo comes with a bash script to easily generate blacklist hash `generate_blacklist_hash.sh`, just run it with your config file path:

```
./generate_blacklist_hash.sh /path/to/nodeos/config.ini

The sha256sum of your blacklist config is:

55ed0834aecb56c86a716064ecda2412de42f4a43df1dbff41d441d47725e1aa

You can publish your hash to theblacklist contract as follows(add your BP name before you submit):
cleos -u https://api.eoslaomao.com push action theblacklist sethash '{"producer": "", "hash": "55ed0834aecb56c86a716064ecda2412de42f4a43df1dbff41d441d47725e1aa"}'

```

then, you can submit your hash to `producerhash` table using `sethash` action:

```
cleos push action theblacklist sethash '{"producer": "BP_ACCOUNT", "hash": "55ed0834aecb56c86a716064ecda2412de42f4a43df1dbff41d441d47725e1aa"}' -p BP_ACCOUNT@active
```

### 3. Delete BP's blacklist hash entry

just call `delhash` action to delete your blacklist hash entry from `producerhash` json.

```
cleos push action the blacklist delhash '{"producer": "BP_ACCOUNT"}' -p BP_ACCOUNT@active
```

## Fetch data from contract

### 1. Fetch blacklist data

#### READ it from table `theblacklist` as json

```
cleos get table theblacklist theblacklist theblacklist

{
  "rows": [{
      "id": 0,
      "order_name": "ECAF_Arbitrator_Order_2018-06-19-AO-001",
      "order_url": "https://eoscorearbitration.io/wp-content/uploads/2018/07/ECAF_Arbitrator_Order_2018-06-19-AO-001.pdf",
      "order_hash": "a80df3e8cfa895a02161dc4d5d04392e3274bce917935c6c214cfe0f1f7e868a",
      "action": "add",
      "type": "actor-blacklist",
      "accounts": [
        "blacklistmee",
        "ge2dmmrqgene",
        "gu2timbsguge",
        "ge4tsmzvgege",
        "gezdonzygage",
        "ha4tkobrgqge",
        "ha4tamjtguge",
        "gq4dkmzzhege"
      ]
    }
    ......
  ],
  "more": false
}
```

#### READ it as nodeos config :)

just run `fetch.sh`, which will using `curl` to call an api endpoint(currently, hardcoded `https://api-kylin.eoslaomao.com` as `theblacklist` already deployed on Kylin Testnet), you will get a well documented nodeos config:

```
# from order: ECAF_Arbitrator_Order_2018-06-19-AO-001
actor-blacklist = blacklistmee
actor-blacklist = ge2dmmrqgene
actor-blacklist = gu2timbsguge
actor-blacklist = ge4tsmzvgege
actor-blacklist = gezdonzygage
actor-blacklist = ha4tkobrgqge
actor-blacklist = ha4tamjtguge
actor-blacklist = gq4dkmzzhege

# from order: ECAF_Arbitrator_Order_2018-06-22-AO-002
actor-blacklist = gu2teobyg4ge
actor-blacklist = gq4demryhage
actor-blacklist = q4dfv32fxfkx
actor-blacklist = ktl2qk5h4bor
actor-blacklist = haydqnbtgene
actor-blacklist = g44dsojygyge
actor-blacklist = guzdonzugmge
actor-blacklist = ha4doojzgyge
actor-blacklist = gu4damztgyge
actor-blacklist = haytanjtgige
actor-blacklist = exchangegdax
actor-blacklist = cmod44jlp14k
actor-blacklist = 2fxfvlvkil4e
actor-blacklist = yxbdknr3hcxt
actor-blacklist = yqjltendhyjp
actor-blacklist = pm241porzybu
actor-blacklist = xkc2gnxfiswe
actor-blacklist = ic433gs42nky
actor-blacklist = fueaji11lhzg
actor-blacklist = w1ewnn4xufob
actor-blacklist = ugunxsrux2a3
actor-blacklist = gz3q24tq3r21
actor-blacklist = u5rlltjtjoeo
actor-blacklist = k5thoceysinj
actor-blacklist = ebhck31fnxbi
actor-blacklist = pvxbvdkces1x
actor-blacklist = oucjrjjvkrom

# from order: ECAF-Temporary-Freeze-Order-2018-07-13-AO-003
actor-blacklist = neverlandwal
actor-blacklist = tseol5n52kmo
actor-blacklist = potus1111111

# from order: ECAF – Order of Emergency Protection – 2018-07-19-AO-004
actor-blacklist = craigspys211

# from order: ECAF-Order-of-Emergency-Protection-2018-07-19-AO-004-Reissue
actor-blacklist = craigspys211

# from order: ECAF-Order-of-Emergency-Protection-2018-08-07-AO-005
actor-blacklist = eosfomoplay1
```

you can easily compare this config with yours, expecially when your are a BP :)

you can also specify other API endpoints as the first param:

```
./fetch.sh https://api1.eosasia.one
```

### 2. Fetch producer hash data

#### Fetch producer hash data from table:

```
cleos get table theblacklist theblacklist producerhash

{
  "rows": [{
      "id": 0,
      "producer": "eoslaomaocom",
      "hash": "55ed0834aecb56c86a716064ecda2412de42f4a43df1dbff41d441d47725e1aa"
      },
      ......
      ]
}

```
### 3. Check blacklist
```
curl -sL https://raw.githubusercontent.com/EOSLaoMao/theblacklist/master/check_blacklist.sh | bash -s [api] [config.ini]
```

#### TODO: compare hash data in `producerhash` table

will provide a bash script to do easy compare and summary of hashes.

### Clear `theblacklist` table(DEBUG only)

`clear` action should be used for debug only, in production(or mainnet) `clear` action should be disabled.

```
cleos push action producerjson clear '' -p ecafofficial@active
```

# Current status
as a demo, we have deployed the contract using `theblacklist` account on Kylin Testnet: https://tools.cryptokylin.io/#/tx/theblacklist

also, all ECAF orders are set in `theblacklist` table.

check it out:

```
cleos -u https://api.eoslaomao.com get table theblacklist theblacklist theblacklist
```

# ECAF, feel free to use it!
lets push onchain governance one more step forward!


---

# How to build
```
./build.sh
```

# How to deploy

```
cleos set contract theblacklist theblacklist/ -p theblacklist@active
```

built with Love by EOSLaoMao Team.
