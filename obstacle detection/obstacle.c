#include "obstacle.h"
#include "vl53l0x.h"

void I2C_init(void);

#define OBSTACLE_THRESHOLD_MM 120

static uint16_t last_distance_mm = 0;
static int sensor_ready = 0;

void Obstacle_Init(void)
{
    unsigned char success;

    I2C_init();
    success = vl53l0x_init();

    if(success)
    {
        sensor_ready = 1;
    }
    else
    {
        sensor_ready = 0;
    }
}

uint16_t Obstacle_GetDistanceMM(void)
{
    unsigned char success;
    unsigned short range = 0;

    if(!sensor_ready)
    {
        return 0;
    }

    success = vl53l0x_read_range_single(&range);

    if(success)
    {
        last_distance_mm = (uint16_t)range;
    }

    return last_distance_mm;
}

int Obstacle_IsBlocked(void)
{
    uint16_t distance;

    distance = Obstacle_GetDistanceMM();

    if(distance > 0 && distance < OBSTACLE_THRESHOLD_MM)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int Obstacle_IsReady(void)
{
    return sensor_ready;
}