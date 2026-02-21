#include <mlib/mtime.h>
#include <mlib/mmath.h>


static const struct mtime_zone_info mtime_zone_table[] = {
    MTIME_ZONE_DEF(UTC_M12_00, "UTC-12:00", 1, 12, 0),
    MTIME_ZONE_DEF(UTC_M11_00, "UTC-11:00", 1, 11, 0),
    MTIME_ZONE_DEF(UTC_M10_00, "UTC-10:00", 1, 10, 0),
    MTIME_ZONE_DEF(UTC_M09_30, "UTC-09:30", 1, 9, 30),
    MTIME_ZONE_DEF(UTC_M09_00, "UTC-09:00", 1, 9, 0),
    MTIME_ZONE_DEF(UTC_M08_00, "UTC-08:00", 1, 8, 0),
    MTIME_ZONE_DEF(UTC_M07_00, "UTC-07:00", 1, 7, 0),
    MTIME_ZONE_DEF(UTC_M06_00, "UTC-06:00", 1, 6, 0),
    MTIME_ZONE_DEF(UTC_M05_00, "UTC-05:00", 1, 5, 0),
    MTIME_ZONE_DEF(UTC_M04_00, "UTC-04:00", 1, 4, 0),
    MTIME_ZONE_DEF(UTC_M03_30, "UTC-03:30", 1, 3, 30),
    MTIME_ZONE_DEF(UTC_M03_00, "UTC-03:00", 1, 3, 0),
    MTIME_ZONE_DEF(UTC_M02_00, "UTC-02:00", 1, 2, 0),
    MTIME_ZONE_DEF(UTC_M01_00, "UTC-01:00", 1, 1, 0),
    MTIME_ZONE_DEF(UTC_00_00,  "UTC+00:00", 0, 0, 0),
    MTIME_ZONE_DEF(UTC_P01_00, "UTC+01:00", 0, 1, 0),
    MTIME_ZONE_DEF(UTC_P02_00, "UTC+02:00", 0, 2, 0),
    MTIME_ZONE_DEF(UTC_P03_00, "UTC+03:00", 0, 3, 0),
    MTIME_ZONE_DEF(UTC_P03_30, "UTC+03:30", 0, 3, 30),
    MTIME_ZONE_DEF(UTC_P04_00, "UTC+04:00", 0, 4, 0),
    MTIME_ZONE_DEF(UTC_P04_30, "UTC+04:30", 0, 4, 30),
    MTIME_ZONE_DEF(UTC_P05_00, "UTC+05:00", 0, 5, 0),
    MTIME_ZONE_DEF(UTC_P05_30, "UTC+05:30", 0, 5, 30),
    MTIME_ZONE_DEF(UTC_P05_45, "UTC+05:45", 0, 5, 45),
    MTIME_ZONE_DEF(UTC_P06_00, "UTC+06:00", 0, 6, 0),
    MTIME_ZONE_DEF(UTC_P06_30, "UTC+06:30", 0, 6, 30),
    MTIME_ZONE_DEF(UTC_P07_00, "UTC+07:00", 0, 7, 0),
    MTIME_ZONE_DEF(UTC_P08_00, "UTC+08:00", 0, 8, 0),
    MTIME_ZONE_DEF(UTC_P08_45, "UTC+08:45", 0, 8, 45),
    MTIME_ZONE_DEF(UTC_P09_00, "UTC+09:00", 0, 9, 0),
    MTIME_ZONE_DEF(UTC_P09_30, "UTC+09:30", 0, 9, 30),
    MTIME_ZONE_DEF(UTC_P10_00, "UTC+10:00", 0, 10, 0),
    MTIME_ZONE_DEF(UTC_P10_30, "UTC+10:30", 0, 10, 30),
    MTIME_ZONE_DEF(UTC_P11_00, "UTC+11:00", 0, 11, 0),
    MTIME_ZONE_DEF(UTC_P12_00, "UTC+12:00", 0, 12, 0),
    MTIME_ZONE_DEF(UTC_P12_45, "UTC+12:45", 0, 12, 45),
    MTIME_ZONE_DEF(UTC_P13_00, "UTC+13:00", 0, 13, 0),
    MTIME_ZONE_DEF(UTC_P14_00, "UTC+14:00", 0, 14, 0)
};

/* Remap months for algorithim */
static const uint8_t mtime_get_week_day_month_lut[] = {
    [MTIME_MONTH_JANUARY]    = 11,
    [MTIME_MONTH_FEBRUARY]   = 12,
    [MTIME_MONTH_MARCH]      = 1,
    [MTIME_MONTH_APRIL]      = 2,
    [MTIME_MONTH_MAY]        = 3,
    [MTIME_MONTH_JUNE]       = 4,
    [MTIME_MONTH_JULY]       = 5,
    [MTIME_MONTH_AUGUST]     = 6,
    [MTIME_MONTH_SEPTEMBER]  = 7,
    [MTIME_MONTH_OCTOBER]    = 8,
    [MTIME_MONTH_NOVEMBER]   = 9,
    [MTIME_MONTH_DECEMBER]   = 10
};

/* Remap week day result to enum */
static const enum mtime_week_day mtime_get_week_day_day_lut[] = {
        MTIME_WEEK_DAY_SUNDAY, MTIME_WEEK_DAY_MONDAY, MTIME_WEEK_DAY_TUESDAY,
        MTIME_WEEK_DAY_WEDNESDAY, MTIME_WEEK_DAY_THURSDAY, MTIME_WEEK_DAY_FRIDAY,
        MTIME_WEEK_DAY_SATURDAY
};

static const char *mtime_day_week_str_lut[] = {
    [MTIME_WEEK_DAY_MONDAY]      = "Monday",
    [MTIME_WEEK_DAY_TUESDAY]     = "Tuesday",
    [MTIME_WEEK_DAY_WEDNESDAY]   = "Wednesday",
    [MTIME_WEEK_DAY_THURSDAY]    = "Thursday",
    [MTIME_WEEK_DAY_FRIDAY]      = "Friday",
    [MTIME_WEEK_DAY_SATURDAY]    = "Saturday",
    [MTIME_WEEK_DAY_SUNDAY]      = "Sunday"
};

const char *time_month_str_lut[] = {
    [MTIME_MONTH_JANUARY]    = "January",
    [MTIME_MONTH_FEBRUARY]   = "February",
    [MTIME_MONTH_MARCH]      = "March",
    [MTIME_MONTH_APRIL]      = "April",
    [MTIME_MONTH_MAY]        = "May",
    [MTIME_MONTH_JUNE]       = "June",
    [MTIME_MONTH_JULY]       = "July",
    [MTIME_MONTH_AUGUST]     = "August",
    [MTIME_MONTH_SEPTEMBER]  = "September",
    [MTIME_MONTH_OCTOBER]    = "October",
    [MTIME_MONTH_NOVEMBER]   = "November",
    [MTIME_MONTH_DECEMBER]   = "December"
};


/* https://cs.uwaterloo.ca/~alopez-o/math-faq/node73.html */
enum mtime_week_day mtime_get_week_day(struct mtime *time) {
    uint32_t temp;
    /* ASSERT(TIME_CHECK(time)); */
    uint8_t year;
    uint16_t century;
    uint8_t month;

    /* Year and century */
    century = time->year / 100;
    year = time->year % 100;

    /* Remap month */
    month = mtime_get_week_day_month_lut[time->month];

    /* 400 year correction */
    float correction = (2.6f * month) - 0.2f;

    temp = time->day + mmath_floorf(correction);
    temp = temp - (2 * century) + year + (year >> 2) + (century >> 2);
    temp = temp % 7;

    /* ASSERT(temp >= 0 & temp <= 6); */
    /* Remap result  */
    return mtime_get_week_day_day_lut[temp];
}
double mtime_difftime(struct mtime *start, struct mtime *end) {
    return 0.0;
}
