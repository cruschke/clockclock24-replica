/**
 * Timezone-Rule Based Local Time Computation
 * 
 * Converts UTC time to local civil time for a given timezone identifier,
 * handling DST transitions automatically for supported zones.
 * 
 * Related Tasks:
 *  - T008: Implement timezone-rule based offset resolution and next-transition handling
 *  - T009: Wire timezone-rule runtime support into NTP provider path
 *  - T014: Replace fixed offset arithmetic with timezone-rule local time conversion
 */

#include <Arduino.h>
#include "timezone_profiles.h"
#include <time.h>
#include <string.h>

// Generic DST rule families (city-agnostic):
// - Europe: last Sunday March 01:00 UTC to last Sunday October 01:00 UTC
// - North America: second Sunday March 02:00 local to first Sunday November 02:00 local
// - Australia (Sydney/Melbourne): first Sunday October 02:00 local to first Sunday April 03:00 local

enum DstRuleFamily {
  DST_RULE_NONE,
  DST_RULE_EUROPE,
  DST_RULE_NORTH_AMERICA,
  DST_RULE_AUSTRALIA
};

static int days_from_civil(int y, unsigned m, unsigned d) {
  y -= m <= 2;
  const int era = (y >= 0 ? y : y - 399) / 400;
  const unsigned yoe = (unsigned)(y - era * 400);
  const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
  const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return era * 146097 + (int)doe - 719468;
}

static time_t make_utc_epoch(int y, int m, int d, int hh, int mm, int ss) {
  long days = (long)days_from_civil(y, (unsigned)m, (unsigned)d);
  return (time_t)(days * 86400L + hh * 3600L + mm * 60L + ss);
}

// weekday: 0=Sunday ... 6=Saturday
static int weekday_ymd(int y, int m, int d) {
  int days = days_from_civil(y, (unsigned)m, (unsigned)d);
  int w = (days + 4) % 7; // 1970-01-01 = Thursday (4)
  return w < 0 ? w + 7 : w;
}

static bool is_leap_year(int y) {
  // Gregorian leap year rule:
  // - divisible by 4 -> leap
  // - divisible by 100 -> not leap
  // - divisible by 400 -> leap
  // Examples: 2024 -> leap, 2100 -> not leap, 2000 -> leap.
  return ((y % 4 == 0) && (y % 100 != 0)) || (y % 400 == 0);
}

static int days_in_month(int y, int m) {
  static const int dim[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (m < 1 || m > 12) return 30;
  if (m != 2) return dim[m - 1];
  return is_leap_year(y) ? 29 : 28;
}

static int nth_weekday_of_month(int y, int m, int weekday, int n) {
  int w_first = weekday_ymd(y, m, 1);
  int delta = (weekday - w_first + 7) % 7;
  return 1 + delta + (n - 1) * 7;
}

static int last_weekday_of_month(int y, int m, int weekday) {
  int last_day = days_in_month(y, m);
  int w_last = weekday_ymd(y, m, last_day);
  int delta = (w_last - weekday + 7) % 7;
  return last_day - delta;
}

static DstRuleFamily get_dst_rule_family(const char *iana_id) {
  if (!iana_id) return DST_RULE_NONE;
  if (strncmp(iana_id, "Europe/", 7) == 0) return DST_RULE_EUROPE;
  if (strncmp(iana_id, "America/", 8) == 0) return DST_RULE_NORTH_AMERICA;
  if (strncmp(iana_id, "Australia/", 10) == 0) return DST_RULE_AUSTRALIA;
  return DST_RULE_NONE;
}

static void compute_dst_window_utc(
  const TimezoneProfile *profile,
  int year,
  DstRuleFamily family,
  time_t *start_utc,
  time_t *end_utc
) {
  if (!profile || !start_utc || !end_utc) {
    return;
  }

  if (family == DST_RULE_EUROPE) {
    int spring_day = last_weekday_of_month(year, 3, 0);
    int fall_day = last_weekday_of_month(year, 10, 0);
    *start_utc = make_utc_epoch(year, 3, spring_day, 1, 0, 0);
    *end_utc = make_utc_epoch(year, 10, fall_day, 1, 0, 0);
    return;
  }

  if (family == DST_RULE_NORTH_AMERICA) {
    int spring_day = nth_weekday_of_month(year, 3, 0, 2);
    int fall_day = nth_weekday_of_month(year, 11, 0, 1);
    time_t spring_local_std = make_utc_epoch(year, 3, spring_day, 2, 0, 0);
    time_t fall_local_dst = make_utc_epoch(year, 11, fall_day, 2, 0, 0);
    *start_utc = spring_local_std - profile->standard_offset_seconds;
    *end_utc = fall_local_dst - profile->daylight_offset_seconds;
    return;
  }

  if (family == DST_RULE_AUSTRALIA) {
    // Australian DST season: starts first Sunday Oct (year), ends first Sunday Apr (year+1).
    // start_utc and end_utc therefore span two calendar years — end is April of year+1.
    int spring_day = nth_weekday_of_month(year, 10, 0, 1);
    int fall_day   = nth_weekday_of_month(year + 1, 4, 0, 1);  // April of NEXT year
    time_t spring_local_std = make_utc_epoch(year,     10, spring_day, 2, 0, 0);
    time_t fall_local_dst   = make_utc_epoch(year + 1,  4, fall_day,   3, 0, 0);
    *start_utc = spring_local_std - profile->standard_offset_seconds;
    *end_utc   = fall_local_dst   - profile->daylight_offset_seconds;
    return;
  }

  *start_utc = 0;
  *end_utc = 0;
}

/**
 * Stub timezone profile definitions
 * In production, these would be defined in timezone_profiles.cpp
 */

const TimezoneProfile PROFILE_EUROPE_BERLIN = {
  .iana_id = "Europe/Berlin",
  .has_dst = true,
  .standard_offset_seconds = 3600,      // CET = UTC+1
  .daylight_offset_seconds = 7200       // CEST = UTC+2
};

const TimezoneProfile PROFILE_UTC = {
  .iana_id = "UTC",
  .has_dst = false,
  .standard_offset_seconds = 0,
  .daylight_offset_seconds = 0
};

const TimezoneProfile PROFILE_AMERICA_NEW_YORK = {
  .iana_id = "America/New_York",
  .has_dst = true,
  .standard_offset_seconds = -18000,    // EST = UTC-5
  .daylight_offset_seconds = -14400     // EDT = UTC-4
};

const TimezoneProfile PROFILE_AFRICA_LAGOS = {
  .iana_id = "Africa/Lagos",
  .has_dst = false,
  .standard_offset_seconds = 3600,      // WAT = UTC+1
  .daylight_offset_seconds = 3600
};

// Stub implementations for other profiles
const TimezoneProfile PROFILE_EUROPE_LONDON = {"Europe/London", true, 0, 3600};
const TimezoneProfile PROFILE_EUROPE_PARIS = {"Europe/Paris", true, 3600, 7200};
const TimezoneProfile PROFILE_EUROPE_AMSTERDAM = {"Europe/Amsterdam", true, 3600, 7200};
const TimezoneProfile PROFILE_EUROPE_BRUSSELS = {"Europe/Brussels", true, 3600, 7200};
const TimezoneProfile PROFILE_EUROPE_VIENNA = {"Europe/Vienna", true, 3600, 7200};
const TimezoneProfile PROFILE_EUROPE_PRAGUE = {"Europe/Prague", true, 3600, 7200};
const TimezoneProfile PROFILE_EUROPE_WARSAW = {"Europe/Warsaw", true, 3600, 7200};
const TimezoneProfile PROFILE_EUROPE_MOSCOW = {"Europe/Moscow", false, 10800, 10800};
const TimezoneProfile PROFILE_EUROPE_ISTANBUL = {"Europe/Istanbul", true, 10800, 14400};

const TimezoneProfile PROFILE_AMERICA_CHICAGO = {"America/Chicago", true, -21600, -18000};
const TimezoneProfile PROFILE_AMERICA_DENVER = {"America/Denver", true, -25200, -21600};
const TimezoneProfile PROFILE_AMERICA_LOS_ANGELES = {"America/Los_Angeles", true, -28800, -25200};
const TimezoneProfile PROFILE_AMERICA_TORONTO = {"America/Toronto", true, -18000, -14400};
const TimezoneProfile PROFILE_AMERICA_MEXICO_CITY = {"America/Mexico_City", true, -21600, -18000};

const TimezoneProfile PROFILE_AUSTRALIA_SYDNEY = {"Australia/Sydney", true, 36000, 39600};
const TimezoneProfile PROFILE_AUSTRALIA_MELBOURNE = {"Australia/Melbourne", true, 36000, 39600};
const TimezoneProfile PROFILE_ASIA_TOKYO = {"Asia/Tokyo", false, 32400, 32400};
const TimezoneProfile PROFILE_ASIA_SHANGHAI = {"Asia/Shanghai", false, 28800, 28800};
const TimezoneProfile PROFILE_ASIA_HONG_KONG = {"Asia/Hong_Kong", false, 28800, 28800};
const TimezoneProfile PROFILE_ASIA_SINGAPORE = {"Asia/Singapore", false, 28800, 28800};
const TimezoneProfile PROFILE_ASIA_BANGKOK = {"Asia/Bangkok", false, 25200, 25200};
const TimezoneProfile PROFILE_ASIA_KOLKATA = {"Asia/Kolkata", false, 19800, 19800};
const TimezoneProfile PROFILE_ASIA_DUBAI = {"Asia/Dubai", false, 14400, 14400};

const TimezoneProfile PROFILE_UTC_PLUS_1 = {"UTC+1", false, 3600, 3600};
const TimezoneProfile PROFILE_UTC_MINUS_5 = {"UTC-5", false, -18000, -18000};
const TimezoneProfile PROFILE_AFRICA_NAIROBI = {"Africa/Nairobi", false, 10800, 10800};

/**
 * Timezone profile lookup table
 */
typedef struct {
  const char *iana_id;
  const TimezoneProfile *profile;
} ProfileMapping;

static const ProfileMapping TIMEZONE_PROFILES[] = {
  {"Europe/Berlin", &PROFILE_EUROPE_BERLIN},
  {"Europe/London", &PROFILE_EUROPE_LONDON},
  {"Europe/Paris", &PROFILE_EUROPE_PARIS},
  {"Europe/Amsterdam", &PROFILE_EUROPE_AMSTERDAM},
  {"Europe/Brussels", &PROFILE_EUROPE_BRUSSELS},
  {"Europe/Vienna", &PROFILE_EUROPE_VIENNA},
  {"Europe/Prague", &PROFILE_EUROPE_PRAGUE},
  {"Europe/Warsaw", &PROFILE_EUROPE_WARSAW},
  {"Europe/Moscow", &PROFILE_EUROPE_MOSCOW},
  {"Europe/Istanbul", &PROFILE_EUROPE_ISTANBUL},
  {"America/New_York", &PROFILE_AMERICA_NEW_YORK},
  {"America/Chicago", &PROFILE_AMERICA_CHICAGO},
  {"America/Denver", &PROFILE_AMERICA_DENVER},
  {"America/Los_Angeles", &PROFILE_AMERICA_LOS_ANGELES},
  {"America/Toronto", &PROFILE_AMERICA_TORONTO},
  {"America/Mexico_City", &PROFILE_AMERICA_MEXICO_CITY},
  {"Australia/Sydney", &PROFILE_AUSTRALIA_SYDNEY},
  {"Australia/Melbourne", &PROFILE_AUSTRALIA_MELBOURNE},
  {"Asia/Tokyo", &PROFILE_ASIA_TOKYO},
  {"Asia/Shanghai", &PROFILE_ASIA_SHANGHAI},
  {"Asia/Hong_Kong", &PROFILE_ASIA_HONG_KONG},
  {"Asia/Singapore", &PROFILE_ASIA_SINGAPORE},
  {"Asia/Bangkok", &PROFILE_ASIA_BANGKOK},
  {"Asia/Kolkata", &PROFILE_ASIA_KOLKATA},
  {"Asia/Dubai", &PROFILE_ASIA_DUBAI},
  {"UTC", &PROFILE_UTC},
  {"UTC+1", &PROFILE_UTC_PLUS_1},
  {"UTC-5", &PROFILE_UTC_MINUS_5},
  {"Africa/Lagos", &PROFILE_AFRICA_LAGOS},
  {"Africa/Nairobi", &PROFILE_AFRICA_NAIROBI},
  {NULL, NULL}  // Sentinel
};

/**
 * Implementation: Get timezone profile by IANA identifier
 */
const TimezoneProfile *get_timezone_profile(const char *iana_id) {
  if (!iana_id) return NULL;
  
  for (int i = 0; TIMEZONE_PROFILES[i].iana_id != NULL; i++) {
    if (strcmp(TIMEZONE_PROFILES[i].iana_id, iana_id) == 0) {
      return TIMEZONE_PROFILES[i].profile;
    }
  }
  return NULL;  // Not found
}

/**
 * Implementation: Get DST transition information and current offset
 */
int get_timezone_offset_and_transition(
  const TimezoneProfile *profile,
  time_t epoch_utc,
  bool *is_dst_now,
  time_t *next_transition_epoch
) {
  if (!profile) {
    if (is_dst_now) *is_dst_now = false;
    if (next_transition_epoch) *next_transition_epoch = 0;
    return 0;  // UTC
  }
  
  int current_offset = profile->standard_offset_seconds;
  bool is_dst = false;
  time_t next_transition = 0;
  
  // Apply DST logic if this timezone observes DST
  if (profile->has_dst) {
    const struct tm *tm_info = gmtime(&epoch_utc);
    if (tm_info) {
      int year = tm_info->tm_year + 1900;
      DstRuleFamily family = get_dst_rule_family(profile->iana_id);

      if (family == DST_RULE_EUROPE || family == DST_RULE_NORTH_AMERICA) {
        time_t start_utc = 0;
        time_t end_utc = 0;
        compute_dst_window_utc(profile, year, family, &start_utc, &end_utc);

        if (start_utc > 0 && end_utc > 0) {
          is_dst = (epoch_utc >= start_utc && epoch_utc < end_utc);
          if (is_dst) {
            current_offset = profile->daylight_offset_seconds;
            next_transition = end_utc;
          } else {
            current_offset = profile->standard_offset_seconds;
            next_transition = (epoch_utc < start_utc) ? start_utc : 0;
            if (next_transition == 0) {
              time_t next_year_start = 0;
              time_t next_year_end = 0;
              compute_dst_window_utc(profile, year + 1, family, &next_year_start, &next_year_end);
              next_transition = next_year_start;
            }
          }
        }
      } else if (family == DST_RULE_AUSTRALIA) {
        // Season is Oct(year-1) -> Apr(year). compute_dst_window_utc(year-1) gives:
        //   start = Oct(year-1),  end = Apr(year)  — both correct.
        time_t start_utc = 0;
        time_t end_utc = 0;
        compute_dst_window_utc(profile, year - 1, family, &start_utc, &end_utc);

        is_dst = (start_utc > 0 && end_utc > 0 &&
                  epoch_utc >= start_utc && epoch_utc < end_utc);
        current_offset = is_dst ? profile->daylight_offset_seconds
                                : profile->standard_offset_seconds;

        if (is_dst) {
          next_transition = end_utc;  // next change: fall back in April
        } else {
          // Next change: spring forward in October of current year
          time_t next_start = 0, next_end = 0;
          compute_dst_window_utc(profile, year, family, &next_start, &next_end);
          next_transition = next_start;
        }
      }
    }
  }
  
  if (is_dst_now) *is_dst_now = is_dst;
  if (next_transition_epoch) *next_transition_epoch = next_transition;
  
  return current_offset;
}

/**
 * Implementation: List supported timezone identifiers
 */
const char *get_supported_timezones() {
  // Return comma-separated list for web UI
  // In production, this would be dynamically built or stored in PROGMEM
  return 
    "Europe/Berlin,"
    "Europe/London,"
    "Europe/Paris,"
    "America/New_York,"
    "America/Los_Angeles,"
    "Asia/Tokyo,"
    "Australia/Sydney,"
    "UTC,"
    "Africa/Lagos";
}
