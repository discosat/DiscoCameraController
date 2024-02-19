#include <csp/csp_debug.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>

#include <csp/csp.h>
#include <csp/drivers/usart.h>
#include <csp/drivers/can_socketcan.h>
#include <csp/interfaces/csp_if_zmqhub.h>


/* These three functions must be provided in arch specific way */
int router_start(void);
int server_start(void);

#define SERVER_PORT	10
#define ZMQ_ADDRESS "localhost"
#define SERVER_ADDRESS 2

/* Server task - handles requests from clients */
void server(void) {

	csp_print("Server task started\n");

	/* Create socket with no specific socket options, e.g. accepts CRC32, HMAC, etc. if enabled during compilation */
	csp_socket_t sock = {0};

	/* Bind socket to all ports, e.g. all incoming connections will be handled here */
	csp_bind(&sock, CSP_ANY);

	/* Create a backlog of 10 connections, i.e. up to 10 new connections can be queued */
	csp_listen(&sock, 10);

	/* Wait for connections and then process packets on the connection */
	while (1) {

		/* Wait for a new connection, 10000 mS timeout */
		csp_conn_t *conn;
		if ((conn = csp_accept(&sock, 10000)) == NULL) {
			/* timeout */
			continue;
		}

		/* Read packets on connection, timout is 100 mS */
		csp_packet_t *packet;
		while ((packet = csp_read(conn, 50)) != NULL) {
			switch (csp_conn_dport(conn)) {
			case SERVER_PORT:
				/* Process packet here */
				csp_print("Packet received on SERVER_PORT: %s\n", (char *) packet->data);
				csp_buffer_free(packet); 
				break;

			default:
				/* Call the default CSP service handler, handle pings, buffer use, etc. */
				csp_service_handler(packet);
				break;
			}
		}

		/* Close current connection */
		csp_close(conn);

	}

	return;

}

/* Initialization of CSP */
int main() {
    csp_print("Initialising CSP\n");

    /* Init CSP */
    csp_init();

    /* Start router */
    router_start();

    /* Add interface(s) */
    csp_iface_t * default_iface = NULL;

    csp_print("Connection table\r\n");
    csp_conn_print_table();

    csp_print("Interfaces\r\n");
    csp_iflist_print();

    int error = csp_zmqhub_init(SERVER_ADDRESS, ZMQ_ADDRESS, 0, &default_iface);
    if (error != CSP_ERR_NONE) {
        csp_print("failed to add ZMQ interface [%s], error: %d\n", ZMQ_ADDRESS, error);
        exit(1);
    }
    default_iface->is_default = 1;

    server_start();

    while(1) {
        sleep(3);
    }

    return 0;
}
