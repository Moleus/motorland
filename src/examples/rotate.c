#include <gpiod.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>

// Define the GPIO pins used to control the stepper motor
// GPIO7 line pins 58 59 60 61
#define PIN_1 2
#define PIN_2 3
#define PIN_3 4
#define PIN_4 5

#define CHIP0_DEV "/dev/gpiochip7"
#define STEPS_PER_REVOLUTION 2038
#define SPEED_REV_PER_MINUTE 5

// Define the delay between steps in microseconds
const uint32_t STEP_DELAY = 60 * 1000 * 1000 / STEPS_PER_REVOLUTION / SPEED_REV_PER_MINUTE;

int SEQUENCE[8][4] = {
        { 1, 0, 0, 1 },
        { 1, 0, 1, 1 },
        { 1, 0, 1, 0 },
        { 1, 1, 1, 0 },
        { 0, 1, 1, 0 },
        { 0, 1, 1, 1 },
        { 0, 1, 0, 1 },
        { 1, 1, 0, 1 },
};


struct gpiod_line_request* config_lines(struct gpiod_chip* chip,  const unsigned int *pin_offsets, uint16_t num_pins);

void rotate(const unsigned int* pin_offsets, struct gpiod_line_request* line_request);

int main()
{
    // Initialize the GPIO chip.
    struct gpiod_chip *chip = gpiod_chip_open(CHIP0_DEV);
    if (!chip) {
        perror("Failed to open GPIO chip");
        return EXIT_FAILURE;
    }

    // Define the pin offsets and values.
    const unsigned int pin_offsets[] = { PIN_1, PIN_2, PIN_3, PIN_4 };
    struct gpiod_line_request* request = config_lines(chip, pin_offsets, 4);
    if (!request) {
        perror("Failed to request GPIO lines");
        gpiod_chip_close(chip);
        return EXIT_FAILURE;
    }

    // Rotate the stepper motor by setting the pins high in pairs.
    rotate(pin_offsets, request);

    // Release the GPIO lines and close the GPIO chip.
    gpiod_line_request_release(request);
    gpiod_chip_close(chip);

    return EXIT_SUCCESS;
}

struct gpiod_line_request* config_lines(struct gpiod_chip* chip, const unsigned int *pin_offsets, uint16_t num_pins) {
    const enum gpiod_line_value disabled_values[] = {GPIOD_LINE_VALUE_INACTIVE, GPIOD_LINE_VALUE_INACTIVE,
                                                     GPIOD_LINE_VALUE_INACTIVE, GPIOD_LINE_VALUE_INACTIVE };

    struct gpiod_line_config* line_config = gpiod_line_config_new();
    if (line_config == NULL) {
        perror("Failed get new line config");
        exit(EXIT_FAILURE);
    }

    struct gpiod_line_settings* settings = gpiod_line_settings_new();

    int out_res = gpiod_line_settings_set_output_value(settings, GPIOD_LINE_VALUE_INACTIVE);
    if (out_res == -1) {
        perror("Failed to set output value");
        exit(EXIT_FAILURE);
    }

    int dir_res = gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);
    if (dir_res == -1) {
        perror("Failed to set direction");
        exit(EXIT_FAILURE);
    }
    int add_res = gpiod_line_config_add_line_settings(line_config, pin_offsets, num_pins, settings);
    if (add_res == -1) {
        perror("Failed to add line");
        exit(EXIT_FAILURE);
    }

    int set_res = gpiod_line_config_set_output_values(line_config, disabled_values, num_pins);
    if (set_res == -1) {
        perror("Failed to set output values line");
        exit(EXIT_FAILURE);
    }
    struct gpiod_line_request* request = gpiod_chip_request_lines(chip, NULL, line_config);
    return request;
}


void rotate(const unsigned int* pin_offsets, struct gpiod_line_request* line_request) {
    for (int i = 0; i < 8; i++) {
        int res = gpiod_line_request_set_values_subset(line_request, 4, pin_offsets, SEQUENCE[i]);
        if (res == -1) {
            perror("Failed to rotate line");
            return;
        }
        usleep(STEP_DELAY);
    }
}


void die_error(const char* fmt, ...) {
    const char* prog_name = "gpio-rotate-example";
    va_list va;

    va_start(va, fmt);
    fprintf(stderr, "%s: ", prog_name);
    vfprintf(stderr, fmt, va);
    fprintf(stderr, "\n");
    va_end(va);

    exit(EXIT_FAILURE);
}

