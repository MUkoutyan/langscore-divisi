#!/bin/bash

./start-http-server.sh mz_test

# 別のスクリプトを実行
echo "run MZ Test for NWjs"
npm run test-mz-nwjs

echo "run MZ Test for Browser"
npm run test-mz-browser

# http-serverを終了
./stop-http-server.sh