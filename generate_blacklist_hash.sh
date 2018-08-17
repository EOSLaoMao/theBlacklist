#!/bin/bash

echo
if [ -z "$1" ];then
  echo ERROR:""
  echo "Please specify your config path: ./generate_blacklist_hash.sh PATH_TO_CONFIG"
  exit
else
  CONFIG=$1
fi

UNAME=`uname`

if [[ $UNAME == 'Darwin' ]]; then
  md5=`cat $CONFIG | grep actor-black | grep -v "#" | sort | uniq | tr -d " " | shasum -a 256 | awk '{print $1}'`
else
  md5=`cat $CONFIG | grep actor-black | grep -v "#" | sort | uniq | tr -d " " | sha256sum | awk '{print $1}'`
fi
echo "The sha256sum of your blacklist config is:"
echo $md5
echo
echo "You can publish your hash to theblacklist contract as follows(add your BP name before you submit):"
echo "cleos -u https://api.eoslaomao.com push action theblacklist sethash '{\"producer\": \"\", \"hash\": \"$md5\"}'"
echo
