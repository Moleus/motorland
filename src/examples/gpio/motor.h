#pragma once

#include <gpiod.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>

struct GpioInfo;

struct Motor;

struct Motor* setup_motor(uint32_t speed);

void motor_rotate(struct Motor* motor, int steps_to_move);

void motor_deactivate(struct Motor* motor);
