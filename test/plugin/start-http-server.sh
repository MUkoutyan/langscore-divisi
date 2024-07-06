#!/bin/bash

# 使用するポート番号を指定
FOLDER=$1
PORT=8180
DIRECTORY=/mnt/d/Programming/Github/langscore-divisi/test/plugin/$FOLDER

# http-serverが既に起動しているかチェックし、起動している場合は終了
if lsof -i:$PORT -t >/dev/null; then
  echo "Stopping existing http-server on port $PORT"
  lsof -i:$PORT -t | xargs kill
fi

echo "Starting http-server on port $PORT"

nohup http-server $DIRECTORY -p $PORT > http-server.log 2>&1 &
HTTP_SERVER_PID=$!

# サーバーが起動したことを確認するためにポートにアクセス可能になるまで待機
while ! nc -z localhost $PORT; do
sleep 1
done

# サーバーが起動したので、バックグラウンドに移動
echo $HTTP_SERVER_PID > http-server.pid
echo "run server"
  
