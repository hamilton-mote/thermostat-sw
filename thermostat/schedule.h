#ifndef _SCHEDULE_H
#define _SCHEDULE_H

typedef struct
{
    char    label[8];   // name of the modality
    uint8_t hsp;        // heating setpoint
    uint8_t csp;        // cooling setpoint
    bool    occupied;   // does this modality imply occupancy?
} modality_t;

typedef struct
{
    // midnight to midnight in 1-hour segments
    uint8_t modalities[24];
} daysched_t;

typedef struct
{
    // 0 is sunday, 6 is saturday
    daysched_t days[7];
} weeksched_t;

typedef struct
{
    modality_t modalities[8];
    daysched_t days[7];
} schedule_cfg_t; // 256-bytes

#endif // _SCHEDULE_H
