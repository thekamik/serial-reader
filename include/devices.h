#ifndef DEVICES_H
#define DEVICES_H

#include <stddef.h>

#include "parser.h"

#define DEVICE_PATH_MAX 256
#define DEVICE_INFO_MAX 256
#define MAX_SERIAL_DEVICES 32

typedef struct {
    char port[DEVICE_PATH_MAX];
    char vendor[DEVICE_INFO_MAX];
    char model[DEVICE_INFO_MAX];
    char serial[DEVICE_INFO_MAX];
} serial_device_info_t;

int find_serial_devices(
    device_type_t device_type,
    serial_device_info_t *devices,
    size_t max_devices,
    size_t *device_count
);

#endif