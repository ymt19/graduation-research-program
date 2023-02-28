# research_2021

- Masterノードの実行方法
meke master
./exe/master prop|conv
出力：log_0.csv


- Slaveノードの実行方法
make slave
./exe/slave replica_id prop|conv
出力：log_(replica_id).csv

- Clientの実行方法
make client_time
./exe/client_time execute_time
出力:result/result.csv
フォーマット；送信時間, 受信時間, 待機時間, レプリカID, クライアントID, トランザクションID, 割り当てられたEPOCHID

- logについて(ノード間の通信のみを記録)
送受信フラグ, 計測時間, メッセージサイズ, メッセージタイプ, 送信先(元)ID, BatchID

- 送受信フラグ
0 送信
1 受信
2 バッチ生成(2, -1, -1, -1, -1, BatchID)

メッセージタイプ(MESSAGE TYPE)
(0 ERROR MESSAGE)
(1 REQUEST TERMINATE)
(2 RESPOINSE TERMINATE)
(3 REQUEST TX)
(4 RESPONSE TX)
5 ADD TO BATCH
6 ACK ADD TO BATCH
7 BATCH
8 ACK BATCH
9 REQUEST BATCH