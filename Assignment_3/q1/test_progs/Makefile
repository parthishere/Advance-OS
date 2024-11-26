all:
	gcc -o test_memory test_memory.c
	gcc -o test_cpu test_cpu.c
	gcc -o test_io test_io.c
	gcc -o test_network test_network.c
	gcc -o test_filesystem test_filesystem.c
	gcc -o test_tcp_server test_tcp_server.c
	gcc -o test_tcp_client test_tcp_client.c
	- rm ../test_progs.zip
	zip -r ../test_progs.zip ../test_progs/

run:
	sudo ./test_tcp_server
	
