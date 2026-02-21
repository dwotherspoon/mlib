#ifndef _MTIME_H
#define _MTIME_H

#include <stdint.h>

enum mtime_month {
    MTIME_MONTH_JANUARY = 1,
    MTIME_MONTH_FEBRUARY,
    MTIME_MONTH_MARCH,
    MTIME_MONTH_APRIL,
    MTIME_MONTH_MAY,
    MTIME_MONTH_JUNE,
    MTIME_MONTH_JULY,
    MTIME_MONTH_AUGUST,
    MTIME_MONTH_SEPTEMBER,
    MTIME_MONTH_OCTOBER,
    MTIME_MONTH_NOVEMBER,
    MTIME_MONTH_DECEMBER
};

enum mtime_week_day {
    MTIME_WEEK_DAY_MONDAY = 0,
    MTIME_WEEK_DAY_TUESDAY,
    MTIME_WEEK_DAY_WEDNESDAY,
    MTIME_WEEK_DAY_THURSDAY,
    MTIME_WEEK_DAY_FRIDAY,
    MTIME_WEEK_DAY_SATURDAY,
    MTIME_WEEK_DAY_SUNDAY
};

enum mtime_zone_id {
    MTIME_ZONE_UTC_M12_00_ID,
    MTIME_ZONE_UTC_M11_00_ID,
    MTIME_ZONE_UTC_M10_00_ID,
    MTIME_ZONE_UTC_M09_30_ID,
    MTIME_ZONE_UTC_M09_00_ID,
    MTIME_ZONE_UTC_M08_00_ID,
    MTIME_ZONE_UTC_M07_00_ID,
    MTIME_ZONE_UTC_M06_00_ID,
    MTIME_ZONE_UTC_M05_00_ID,
    MTIME_ZONE_UTC_M04_00_ID,
    MTIME_ZONE_UTC_M03_30_ID,
    MTIME_ZONE_UTC_M03_00_ID,
    MTIME_ZONE_UTC_M02_00_ID,
    MTIME_ZONE_UTC_M01_00_ID,
    MTIME_ZONE_UTC_00_00_ID,
    MTIME_ZONE_UTC_P01_00_ID,
    MTIME_ZONE_UTC_P02_00_ID,
    MTIME_ZONE_UTC_P03_00_ID,
    MTIME_ZONE_UTC_P03_30_ID,
    MTIME_ZONE_UTC_P04_00_ID,
    MTIME_ZONE_UTC_P04_30_ID,
    MTIME_ZONE_UTC_P05_00_ID,
    MTIME_ZONE_UTC_P05_30_ID,
    MTIME_ZONE_UTC_P05_45_ID,
    MTIME_ZONE_UTC_P06_00_ID,
    MTIME_ZONE_UTC_P06_30_ID,
    MTIME_ZONE_UTC_P07_00_ID,
    MTIME_ZONE_UTC_P08_00_ID,
    MTIME_ZONE_UTC_P08_45_ID,
    MTIME_ZONE_UTC_P09_00_ID,
    MTIME_ZONE_UTC_P09_30_ID,
    MTIME_ZONE_UTC_P10_00_ID,
    MTIME_ZONE_UTC_P10_30_ID,
    MTIME_ZONE_UTC_P11_00_ID,
    MTIME_ZONE_UTC_P12_00_ID,
    MTIME_ZONE_UTC_P12_45_ID,
    MTIME_ZONE_UTC_P13_00_ID,
    MTIME_ZONE_UTC_P14_00_ID,
    MTIME_ZONE_COUNT
};

struct mtime_zone_info {
    char *name;
    uint8_t neg : 1;
    /* Minutes 0-59 */
    uint8_t minutes : 6;
    /* Hours 0-24 */
    uint8_t hours : 5;
};

#define MTIME_ZONE_DEF(TZ, NAME, NEG, HRS, MINS)      [MTIME_ZONE_##TZ##_ID] = {  \
                                                        .name = NAME,               \
                                                        .neg = NEG,                 \
                                                        .minutes = MINS,            \
                                                        .hours = HRS                \
}

#define MTIME_CHECK(T)           (((T)->milliseconds >= 0 && (T)->milliseconds <= 999)   \
                                    && ((T)->seconds >= 0 && (T)->seconds <= 59)        \
                                    && ((T)->minutes >= 0 && (T)->minutes <= 59)        \
                                    && ((T)->hours >= 0 && (T)->hours <= 23)            \
                                    && ((T)->day >= 1 && (T)->day <= 31)                \
                                    && ((T)->month >= MTIME_MONTH_JANUARY                \
                                        && (T)->month <= MTIME_MONTH_DECEMBER))

/* Align to cache line size? Currently 6 bytes. */
struct __attribute__((packed)) mtime {
    /* Milliseconds 0-999*/
    uint16_t milliseconds : 10;
    /* Seconds 0-59 */
    uint8_t seconds : 6;
    /* Minutes 0-59 */
    uint8_t minutes : 6;
    /* Hours 0-23 */
    uint8_t hours : 5;
    /* Day of the month 1-31 */
    uint8_t day : 5;
    /* Month, 1-12 */
    enum mtime_month month : 4;
    /* Year */
    uint16_t year : 12;
};

/* https://cs.uwaterloo.ca/~alopez-o/math-faq/node73.html */
enum mtime_week_day mtime_get_week_day(struct mtime *time);
double mtime_difftime(struct mtime *start, struct mtime *end);

#endif
