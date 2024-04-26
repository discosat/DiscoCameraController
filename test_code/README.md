g++ -std=c++17 -o receiver receiver.cpp ../src/message_queue/metadata.pb.cpp \
    -I/usr/include/google/protobuf/ \
    -I../include/message_queue/ \
    /usr/local/lib/libprotobuf.so.30 \
    -lrt `pkg-config --cflags --libs opencv4`


g++ -std=c++17 -o test flir_capture.cpp -lrt `pkg-config --cflags --libs opencv4`

g++ -std=c++11 -I../lib/libcsp/include csp_server_posix.cpp csp_server.cpp -o server -L../lib/libcsp/buildir -lcsp

g++ -std=c++11 -I../lib/libcsp/include csp_client_posix.cpp csp_client.cpp -o client -L../lib/libcsp/buildir -lcsp

gcc -c zmqproxy.c -o proxy.o \
 gcc proxy.o -o proxy -lcsp -lzmq

 ./client -z localhost -C 2 -a 10


 gcc -o camera camera_control.c -lm