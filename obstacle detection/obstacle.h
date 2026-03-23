#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <stdint.h>

void Obstacle_Init(void);
uint16_t Obstacle_GetDistanceMM(void);
int Obstacle_IsBlocked(void);
int Obstacle_IsReady(void);

#endif

