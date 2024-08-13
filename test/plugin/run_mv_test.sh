#!/bin/bash

rm -f ./mv_test/save/*

./start-http-server.sh mv_test

# 別のスクリプトを実行
echo "run MV Test for NWjs"
npm run test-mv-nwjs

echo "run MV Test for Browser"
npm run test-mv-browser

# http-serverを終了
./stop-http-server.sh