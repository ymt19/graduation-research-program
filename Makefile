CC 				= gcc
.PHONY: master slave client_time client_txnum

master: master/master.c batch/batch.c batch/batch_list.c utils/bytes.c client_interface/client_interface.c utils/configuration.c utils/log.c master/master_sequencer.c utils/message.c \
        scheduler/read_scheduler.c utils/storage.c network/tcp/tcp_client.c network/tcp/tcp_server.c utils/timer.c tx/transaction.c tx/transaction_list.c scheduler/write_scheduler.c \
        network/udp/udp_server.c network/udp/udp_client.c utils/configuration.h
	$(CC) -o exe/master -pthread master/master.c batch/batch.c batch/batch_list.c utils/bytes.c client_interface/client_interface.c utils/configuration.c utils/log.c \
	master/master_sequencer.c utils/message.c scheduler/read_scheduler.c utils/storage.c network/tcp/tcp_client.c network/tcp/tcp_server.c utils/timer.c tx/transaction.c \
	tx/transaction_list.c scheduler/write_scheduler.c network/udp/udp_server.c network/udp/udp_client.c

slave: slave/slave.c batch/batch.c batch/batch_list.c utils/bytes.c client_interface/client_interface.c utils/configuration.c utils/log.c slave/slave_sequencer.c utils/message.c \
       scheduler/read_scheduler.c utils/storage.c network/tcp/tcp_client.c network/tcp/tcp_server.c utils/timer.c tx/transaction.c tx/transaction_list.c scheduler/write_scheduler.c \
       network/udp/udp_client.c network/udp/udp_server.c utils/configuration.h
	$(CC)  -o exe/slave -pthread slave/slave.c batch/batch.c batch/batch_list.c utils/bytes.c client_interface/client_interface.c utils/configuration.c utils/log.c slave/slave_sequencer.c \
	utils/message.c scheduler/read_scheduler.c utils/storage.c network/tcp/tcp_client.c network/tcp/tcp_server.c utils/timer.c tx/transaction.c tx/transaction_list.c \
	scheduler/write_scheduler.c network/udp/udp_client.c network/udp/udp_server.c

client_time: client/client_time.c tx/transaction.c utils/message.c utils/bytes.c network/tcp/tcp_client.c batch/batch.c utils/configuration.c utils/storage.c utils/timer.c utils/log.c \
             tx/transaction_list.c batch/batch_list.c utils/configuration.h network/udp/udp_client.c network/udp/udp_server.c
	$(CC)  -o exe/client_time -pthread client/client_time.c tx/transaction.c utils/message.c utils/bytes.c network/tcp/tcp_client.c batch/batch.c utils/configuration.c utils/storage.c utils/timer.c utils/log.c tx/transaction_list.c batch/batch_list.c network/udp/udp_client.c network/udp/udp_server.c

client_txnum: client/client_txnum.c tx/transaction.c utils/message.c utils/bytes.c network/tcp/tcp_client.c batch/batch.c utils/configuration.c utils/storage.c utils/timer.c utils/log.c \
              tx/transaction_list.c batch/batch_list.c utils/configuration.h
	$(CC)  -o exe/client_txnum -pthread client/client_txnum.c tx/transaction.c utils/message.c utils/bytes.c network/tcp/tcp_client.c batch/batch.c utils/configuration.c utils/storage.c utils/timer.c utils/log.c tx/transaction_list.c batch/batch_list.c