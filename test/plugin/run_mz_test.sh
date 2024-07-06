#!/bin/bash

cd /mnt/d/Programming/Github/langscore-divisi/test/plugin/mz_test

PORT=8180
# http-serverをバックグラウンドで実行し、PIDを取得
http-server -s -p $PORT &

# http-serverのPIDを変数に保存
HTTP_SERVER_PID=$!

# サーバーが起動したことを確認するためにポートにアクセス可能になるまで待機
while ! nc -z localhost $PORT; do
  sleep 1 # 1秒待機して再チェック
done


cd ../

# 別のスクリプトを実行
npm run test-mz-nwjs
npm run test-mz-browser

# http-serverを終了
kill $HTTP_SERVER_PID