g++ receiver.cpp -o receiver -lrt `pkg-config --cflags --libs opencv4`

g++ -std=c++11 -I../lib/libcsp/include csp_server_posix.cpp csp_server.cpp -o server -L../lib/libcsp/buildir -lcsp

g++ -std=c++11 -I../lib/libcsp/include csp_client_posix.cpp csp_client.cpp -o client -L../lib/libcsp/buildir -lcsp

gcc -c zmqproxy.c -o proxy.o \
 gcc proxy.o -o proxy -lcsp -lzmq

 ./client -z localhost -C 2 -a 10