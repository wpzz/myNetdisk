server: conn.o thread.o TcpServer.o NdiskServer.o cmydb.o
	g++ conn.o thread.o TcpServer.o NdiskServer.o -o server -levent 
#	g++ conn.o thread.o TcpServer.o NdiskServer.o cmydb.o -o server -levent -L/usr/lib/mysql -lmysqlclient

#lb: conn.o thread.o TcpServer.o NdiskLb.o
#	g++ conn.o thread.o TcpServer.o NdiskLb.o -o lb -levent

conn.o:../net/conn.cpp
	g++ -c ../net/conn.cpp

thread.o:../net/thread.cpp
	g++ -c ../net/thread.cpp

TcpServer.o:../net/TcpServer.cpp
	g++ -c ../net/TcpServer.cpp

NdiskServer.o:NdiskServer.cpp
	g++ -c NdiskServer.cpp

NdiskLb.o:NdiskLb.cpp
	g++ -c NdiskLb.cpp

cmydb.o:../mysql/cmydb.cpp
	g++ -c ../mysql/cmydb.cpp
clean:
	rm *.o
