#!/bin/bash
NAME=theblacklist

docker ps | grep $NAME-eos-dev
if [ $? -ne 0 ]; then
    echo "Run eos dev env "
    docker run --name $NAME-eos-dev -dit --rm -v  `(pwd)`:/$NAME eoslaomao/eos-dev
fi

docker exec $NAME-eos-dev eosiocpp -o /$NAME/$NAME.wast /$NAME/$NAME.cpp
docker exec $NAME-eos-dev eosiocpp -g /$NAME/$NAME.abi /$NAME/$NAME.cpp
echo ".................................build complete!"

docker cp ../$NAME nodeosd:/
