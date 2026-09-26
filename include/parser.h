#ifndef SERIAL_READER_PARSER_H
#define SERIAL_READER_PARSER_H

typedef enum {
    PARITY_NONE,
    PARITY_EVEN,
    PARITY_ODD
} parity_t;

typedef enum {
    MODE_READ,
    MODE_SCAN
} program_mode_t;

typedef enum {
    DEVICE_TYPE_TTYACM,
    DEVICE_TYPE_TTYUSB,
    DEVICE_TYPE_ALL
} device_type_t;

typedef struct {
    program_mode_t mode;
    device_type_t device_type;

    const char *device;
    int baudrate;
    int data_bits;
    parity_t parity;
    int stop_bits;
} serial_config_t;

int parse_args(int argc, char *argv[], serial_config_t *config);
void print_usage(const char *program_name);

#endif