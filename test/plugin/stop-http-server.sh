#!/bin/bash

# 使用するポート番号を指定
PORT=8180

# http-serverが起動しているかをチェックし、終了させる
if [ -f http-server.pid ]; then
  HTTP_SERVER_PID=$(cat http-server.pid)
  echo "Stopping http-server with PID $HTTP_SERVER_PID"
  kill $HTTP_SERVER_PID
  rm http-server.pid
else
  echo "http-server is not running"
fi