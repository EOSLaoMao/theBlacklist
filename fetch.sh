#!/bin/bash

if [ -z "$1" ];then
  API=https://api.eoslaomao.com/v1/chain/get_table_rows
else
  API=$1/v1/chain/get_table_rows
fi

echo
echo "# Gonna fetch blacklist from" $API
echo "# ===============blacklist config==============="

curl -s $API -X POST -d '{"scope":"theblacklist", "code":"theblacklist", "table":"theblacklist", "json": true, "limit": 100
}' | \
   python  -c "import sys, json; rows = json.load(sys.stdin)['rows'];result = ''; configs = []; [configs.extend([('# %s = %s|%s' % (r['type'], a, r['order_name']) if r['action'] != 'add' else '%s = %s|%s' % (r['type'], a, r['order_name'])) for a in r['accounts']])for r in rows]; dups = [configs[j] for i in range(len(configs)) for j in range(i+1, len(configs)) if('# ' + configs[i].split('|')[0] == configs[j].split('|')[0])]; dups_d = dict((i.split('|')[0], i.split('|')[1]) for i in dups); l = [('\n\n# from order: %s' % r['order_name'], [('# %s = %s // removed in order %s' % (r['type'], a, dups_d.get('# %s = %s' % (r['type'], a), r['order_name'])) if (r['action'] == 'remove' or '# %s = %s' % (r['type'], a) in [i.split('|')[0] for i in dups]) else '%s = %s' % (r['type'], a)) for a in r['accounts']])for r in rows]; a = [i[0] + '\n' + '\n'.join(i[1]) for i in l]; print ''.join(a);"
