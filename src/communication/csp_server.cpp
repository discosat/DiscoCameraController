#include "csp_server.hpp"
#include "common.hpp"
#include "csp_parser.hpp"
#include <csp/csp_debug.h>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <functional>
#include <pthread.h>
#include <csp/csp.h>
#include <csp/drivers/usart.h>
#include <csp/drivers/can_socketcan.h>
#include <csp/interfaces/csp_if_zmqhub.h>
#include "message_queue.hpp"
#include "camera_controller.hpp"
#include <memory>


/* These three functions must be provided in arch specific way */
int router_start(void);
int server_start(void);

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
std::shared_ptr<char> shared_packet;
std::shared_ptr<int> server_port;

void server(void) {
    csp_print("Server task started\n");

    csp_socket_t sock = {0};
    csp_bind(&sock, CSP_ANY);
    csp_listen(&sock, 10);

    while (1) {
        csp_conn_t *conn;
        if ((conn = csp_accept(&sock, 10000)) == NULL) {
            /* timeout */
            continue;
        }

        csp_packet_t *packet;
        while ((packet = csp_read(conn, 50)) != NULL) {
            if(csp_conn_dport(conn) == *server_port.get()){
				pthread_mutex_lock(&mutex);
                
                shared_packet = std::shared_ptr<char>(new char[packet->length], std::default_delete<char[]>()); //std::make_shared<char>(packet->length);
                memcpy(shared_packet.get(), packet->data, packet->length*sizeof(uint8_t));
                
                pthread_cond_signal(&condition);
                pthread_mutex_unlock(&mutex);
                
                csp_buffer_free(packet); 
			} else {
				csp_service_handler(packet);
			}
        }

        /* Close current connection */
        csp_close(conn);

    }

    return;

}

/* Initialization of CSP */
void server_init(std::function<void(CaptureMessage params, CameraController*, MessageQueue*, std::vector<VmbCPP::CameraPtr>)> callback,
                CameraController* vmbProvider, MessageQueue* mq, std::vector<VmbCPP::CameraPtr> cameras, CSPInterface interfaceConfig) {
    csp_print("Initialising CSP\n");
    csp_init();
    router_start();
    csp_iface_t * default_iface = NULL;

    csp_print("Connection table\r\n");
    csp_conn_print_table();

    csp_print("Interfaces\r\n");
    csp_iflist_print();

    int error = CSP_ERR_NONE;
    switch (interfaceConfig.Interface)
    {
    case CSPInterfaceType::ZMQ:
		error = csp_zmqhub_init(interfaceConfig.Node, interfaceConfig.Device.c_str(), 0, &default_iface);
        break;
    case CSPInterfaceType::CAN:
	    error = csp_can_socketcan_open_and_add_interface(interfaceConfig.Device.c_str(), CSP_IF_CAN_DEFAULT_NAME, 
														interfaceConfig.Node, 1000000, true, &default_iface);
        break;
    case CSPInterfaceType::KISS:
			csp_usart_conf_t conf = {
				.device = interfaceConfig.Device.c_str(),
				.baudrate = 115200, /* supported on all platforms */
				.databits = 8,
				.stopbits = 1,
				.paritysetting = 0,
			};
			int error = csp_usart_open_and_add_kiss_interface(&conf, CSP_IF_KISS_DEFAULT_NAME,  &default_iface);
			default_iface->addr = interfaceConfig.Node;
        break;
    }

    if (error != CSP_ERR_NONE) {
        csp_print("failed to add interface [%s], error: %d\n", interfaceConfig.Device, error);
        exit(1);
    }

    default_iface->is_default = 1;
	server_port = std::make_shared<int>(interfaceConfig.Port);

    server_start();

    while (true) {
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&condition, &mutex);

        csp_print("Packet received on SERVER_PORT: %s\n", (char *) shared_packet.get());
        if(shared_packet.get() != nullptr){
            CaptureMessage msg;
            std::string input(shared_packet.get());
            parseMessage(input, msg);
            shared_packet.reset();

            callback(msg, vmbProvider, mq, cameras); // callback to capture images
        }
        pthread_mutex_unlock(&mutex);
    }
}
