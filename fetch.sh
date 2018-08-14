curl https://api-kylin.eoslaomao.com/v1/chain/get_table_rows -X POST -d '{"scope":"theblacklist", "code":"theblacklist", "table":"theblacklist", "json": true, "limit": 100
}' | \
  python  -c "import sys, json; rows = json.load(sys.stdin)['rows'];result = ''; l = [('\n\n# from order: %s' % r['order_name'], [('# %s = %s' % (r['type'], a) if r['action'] != 'add' else '%s = %s' % (r['type'], a)) for a in r['accounts']])for r in rows]; a = [i[0] + '\n' + '\n'.join(i[1]) for i in l]; print ''.join(a);"
