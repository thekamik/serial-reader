#include "parser.h"

#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parse_positive_int(const char *str, int *value)
{
    char *endptr;
    long number;

    if (str == NULL || *str == '\0') {
        return -1;
    }

    errno = 0;
    endptr = NULL;

    number = strtol(str, &endptr, 10);

    if (errno == ERANGE ||
        endptr == str ||
        *endptr != '\0' ||
        number < 1 ||
        number > INT_MAX) {
        return -1;
    }

    *value = (int)number;

    return 0;
}

static int parse_parity(const char *str, parity_t *parity)
{
    if (strcmp(str, "none") == 0) {
        *parity = PARITY_NONE;
        return 0;
    }

    if (strcmp(str, "even") == 0) {
        *parity = PARITY_EVEN;
        return 0;
    }

    if (strcmp(str, "odd") == 0) {
        *parity = PARITY_ODD;
        return 0;
    }

    return -1;
}

static int parse_device_type(
    const char *str,
    device_type_t *device_type
)
{
    if (strcmp(str, "all") == 0) {
        *device_type = DEVICE_TYPE_ALL;
        return 0;
    }

    if (strcmp(str, "ttyACM") == 0) {
        *device_type = DEVICE_TYPE_TTYACM;
        return 0;
    }

    if (strcmp(str, "ttyUSB") == 0) {
        *device_type = DEVICE_TYPE_TTYUSB;
        return 0;
    }

    return -1;
}

void print_usage(const char *program_name)
{
    printf(
        "Usage:\n"
        "  %s DEVICE [OPTIONS]\n"
        "  %s --scan [--type TYPE]\n"
        "\n"
        "Read mode options:\n"
        "  --baud RATE       Baud rate (default: 9600)\n"
        "  --data BITS       Data bits: 5, 6, 7 or 8 (default: 8)\n"
        "  --parity TYPE     Parity: none, even or odd (default: none)\n"
        "  --stop BITS       Stop bits: 1 or 2 (default: 1)\n"
        "\n"
        "Scan mode options:\n"
        "  --scan            Scan available serial devices\n"
        "  --type TYPE       Device type: all, ttyACM or ttyUSB\n"
        "\n"
        "Other options:\n"
        "  -h, --help        Show this help\n"
        "\n"
        "Examples:\n"
        "  %s /dev/ttyACM0 --baud 9600 --data 8 --parity none --stop 1\n"
        "  %s --scan\n"
        "  %s --scan --type ttyACM\n"
        "  %s --scan --type ttyUSB\n",
        program_name,
        program_name,
        program_name,
        program_name,
        program_name,
        program_name
    );
}

int parse_args(int argc, char *argv[], serial_config_t *config)
{
    static const struct option long_options[] = {
        { "baud",   required_argument, NULL, 'b' },
        { "data",   required_argument, NULL, 'd' },
        { "parity", required_argument, NULL, 'p' },
        { "stop",   required_argument, NULL, 's' },
        { "scan",   no_argument,       NULL, 'c' },
        { "type",   required_argument, NULL, 't' },
        { "help",   no_argument,       NULL, 'h' },
        { NULL,     0,                 NULL,  0  }
    };

    int option;
    int option_index = 0;

    if (config == NULL) {
        fprintf(stderr, "Internal error: config is NULL\n");
        return -1;
    }

    /*
     * Defaults corresponding to Arduino:
     *
     * Serial.begin(9600);
     *
     * 9600 8N1
     */
    config->mode = MODE_READ;
    config->device_type = DEVICE_TYPE_ALL;

    config->device = NULL;
    config->baudrate = 9600;
    config->data_bits = 8;
    config->parity = PARITY_NONE;
    config->stop_bits = 1;

    /*
     * ':' at the beginning makes getopt_long() distinguish
     * between a missing argument and an unknown option.
     */
    opterr = 0;

    while ((option = getopt_long(
                argc,
                argv,
                ":b:d:p:s:ct:h",
                long_options,
                &option_index)) != -1) {

        switch (option) {

        case 'b':
            if (parse_positive_int(optarg, &config->baudrate) != 0) {
                fprintf(
                    stderr,
                    "Invalid baud rate: '%s'\n",
                    optarg
                );
                return -1;
            }
            break;

        case 'd':
            if (parse_positive_int(optarg, &config->data_bits) != 0 ||
                config->data_bits < 5 ||
                config->data_bits > 8) {

                fprintf(
                    stderr,
                    "Invalid data bits: '%s' "
                    "(expected 5, 6, 7 or 8)\n",
                    optarg
                );
                return -1;
            }
            break;

        case 'p':
            if (parse_parity(optarg, &config->parity) != 0) {
                fprintf(
                    stderr,
                    "Invalid parity: '%s' "
                    "(expected none, even or odd)\n",
                    optarg
                );
                return -1;
            }
            break;

        case 's':
            if (parse_positive_int(optarg, &config->stop_bits) != 0 ||
                (config->stop_bits != 1 &&
                 config->stop_bits != 2)) {

                fprintf(
                    stderr,
                    "Invalid stop bits: '%s' "
                    "(expected 1 or 2)\n",
                    optarg
                );
                return -1;
            }
            break;

        case 'c':
            config->mode = MODE_SCAN;
            break;

        case 't':
            if (parse_device_type(
                    optarg,
                    &config->device_type) != 0) {

                fprintf(
                    stderr,
                    "Invalid device type: '%s' "
                    "(expected all, ttyACM or ttyUSB)\n",
                    optarg
                );
                return -1;
            }
            break;

        case 'h':
            print_usage(argv[0]);
            return 1;

        case ':':
            fprintf(
                stderr,
                "Option '%s' requires an argument\n",
                argv[optind - 1]
            );
            return -1;

        case '?':
            if (optopt != 0) {
                fprintf(
                    stderr,
                    "Unknown option: '-%c'\n",
                    optopt
                );
            } else {
                fprintf(
                    stderr,
                    "Unknown option: '%s'\n",
                    argv[optind - 1]
                );
            }
            return -1;

        default:
            return -1;
        }
    }

    /*
     * SCAN mode:
     *
     *     --scan
     *     --scan --type ttyACM
     *     --scan --type ttyUSB
     *     --scan --type all
     *
     * No serial device argument is allowed.
     */
    if (config->mode == MODE_SCAN) {

        if (optind < argc) {
            fprintf(
                stderr,
                "Unexpected argument in scan mode: '%s'\n",
                argv[optind]
            );
            return -1;
        }

        return 0;
    }

    /*
     * READ mode:
     *
     * We require exactly one positional argument:
     *
     *     /dev/ttyACM0
     */
    if (optind >= argc) {
        fprintf(stderr, "Missing serial device\n");
        return -1;
    }

    if (argc - optind > 1) {
        fprintf(
            stderr,
            "Unexpected argument: '%s'\n",
            argv[optind + 1]
        );
        return -1;
    }

    /*
     * --type has no meaning in READ mode.
     */
    if (config->device_type != DEVICE_TYPE_ALL) {
        fprintf(
            stderr,
            "Option '--type' can only be used with '--scan'\n"
        );
        return -1;
    }

    config->device = argv[optind];

    return 0;
}