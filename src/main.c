#include "parser.h"
#include "serial.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>

static volatile sig_atomic_t running = 1;

static void handle_signal(int signal) {
    (void)signal; // Unused parameter
    running = 0;
}

int main(int argc, char *argv[]) {
    serial_config_t config;

    if (parse_args(argc, argv, &config) != 0) {
        return 1;
    }

    /*
    printf("Device:    %s\n", config.device);
    printf("Baud rate: %d\n", config.baudrate);
    printf("Data bits: %d\n", config.data_bits);
    printf("Parity:    %d\n", config.parity);
    printf("Stop bits: %d\n", config.stop_bits);
    */

    serial_port_t port;

    if (serial_start_connection(&port, &config) != 0) {
        perror("serial_start_connection");
        return 1;
    }

    printf("Connected to %s\n", config.device);

    /*
     * Handle Ctrl+C (SIGINT).
     */
    struct sigaction sa = {0};

    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) != 0) {
        perror("sigaction");
        serial_end_connection(&port);
        return 1;
    }

    char buffer[256];

    while (running) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(port.fd, &read_fds);

        int result = select(
            port.fd + 1, 
            &read_fds, 
            NULL, 
            NULL, 
            NULL
        );

        if (result < 0) {
            if (errno == EINTR) {
                // Interrupted by signal, continue
                continue;
            }
            perror("select");
            break;
        }

        if (FD_ISSET(port.fd, &read_fds)) {
            ssize_t bytes_read = serial_read(
                &port, 
                buffer, 
                sizeof(buffer)
            );

            if (bytes_read > 0) {
                fwrite(buffer, 1, bytes_read, stdout);
                fflush(stdout);
            } else if (bytes_read < 0 &&
                    errno != EAGAIN && 
                    errno != EWOULDBLOCK) {
                perror("serial_read");
                break;
            }
        }
    }

    printf("\nClosing connection...\n");

    if (serial_end_connection(&port) != 0) {
        perror("serial_end_connection");
        return 1;
    }

    return 0;
}
