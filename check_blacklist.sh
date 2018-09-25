#!/bin/bash

default_bp="eoslaomaocom"
default_api="https://api1.eosasia.one"
default_config="./config.ini"
api=${default_api}
config=${default_config}
bp=${default_bp}

UNAME=`uname`

if [[ $UNAME == 'Darwin' ]]; then
  checker='shasum -a 256'
else
  checker='sha256sum'
fi

_help() {
    echo ""
    echo "Usage: $0 [-b BP] [-a API] [-c CONFIG]"
    echo "          -b bp account"
    echo "          -a api"
    echo "          -c config file"
    echo "Examples:"
    echo "  $0 -b bp ${default_bp} -a ${default_api} -c ${default_config}"
    echo ""
}

get_chain_actor_list() {
    url="${api}/v1/chain/get_table_rows"
    chain_actor_list=`curl -s ${url} -X POST -d '{"scope":"theblacklist", "code":"theblacklist", "table":"theblacklist", "json": true, "limit": 100}' | python  -c "import sys, json; rows = json.load(sys.stdin)['rows'];result = ''; configs = []; [configs.extend([('# %s = %s' % (r['type'], a) if r['action'] != 'add' else '%s = %s' % (r['type'], a)) for a in r['accounts']])for r in rows]; dups = [configs[j] for i in range(len(configs)) for j in range(i+1, len(configs)) if('# ' + configs[i] == configs[j])]; actors=[]; l = [(a if (r['action'] != 'remove' or '# %s = %s' % (r['type'], a) not in dups) else '' for a in r['accounts']) for r in rows]; [actors.extend(list(l)) for l in l]; actors = list(set([a for a in actors if len(a)])); actors.sort(); print '\n'.join(actors)"`
    [ "${chain_actor_list}" == "" ] && echo "please check api(${api})." && exit 1
}

get_local_actor_list() {
    local_actor_list=`cat ${config} | grep actor-black | grep -v "#" |egrep -o '\w+$'| sort | uniq`
}

check_diff() {
    diff <(echo "${chain_actor_list}") <(echo "${local_actor_list}")| sed 's/</chain -/g' | sed 's/>/local -/g' | egrep 'chain -|local -'
}

# check local and theblacklist actor-blacklist hash
check_hash() {
    local_hash=`cat ${config} | grep actor-black | grep -v "#" | sort | uniq | tr -d " " | ${checker}`
    # get hash from table theblacklist
    chain_hash=`echo "${chain_actor_list}" | sed 's/^/actor-blacklist = /g' | tr -d " " | ${checker}`
    if [ "${local_hash}" == "${chain_hash}" ];then
        echo "local success: ${chain_hash}"
    else
        echo "local: ${local_hash}"
        echo "chain: ${chain_hash}"
    fi
}

# check producerhash and theblacklist actor-blacklist hash
check_producerhash() {
    producer_hash=`curl -s ${url} POST -d '{"scope":"theblacklist", "code":"theblacklist", "table":"producerhash", "json": true, "limit": 600}' | jq -r --arg BP "${bp}" '.rows[]|select(.producer == $BP)|.hash'`
    if [ "${producer_hash}  -" == "${chain_hash}" ];then
        echo "producer_hash success: ${chain_hash}"
    else
        echo "producer hash: ${producer_hash}"
        echo "chain: ${chain_hash}"
    fi
}

process_args() {
    while getopts :hb:a:c: opt
    do
        case "${opt}" in
            h) _help && exit 0 ;;
            b) bp="${OPTARG}" ;;
            a) api="${OPTARG}" ;;
            c) config="${OPTARG}" ;;
            :) echo "Error: Option \"-${OPTARG}\" requires an argument." && _help && exit 1;;
            \?) echo "Error: Unknown option: \"-${OPTARG}\"." && _help && exit 1;;
        esac
    done

    shift $(( ${OPTIND} - 1 ))
    [ ! -f ${config} ] && echo "please check config path(${config})" && exit 1
}

main() {
    process_args $@
    get_chain_actor_list
    get_local_actor_list
    check_diff
    check_hash
    check_producerhash
}

main $@