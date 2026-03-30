#ifndef TIMEZONE_PROFILES_H
#define TIMEZONE_PROFILES_H

/**
 * Timezone Profile Definitions for DST Support
 * 
 * This header defines timezone identifiers and their corresponding DST transition rules.
 * Profiles are used by ntp_timezone.cpp to determine local time offset and DST rules.
 * 
 * Related Tasks:
 *  - T011: Add DST profile constants and non-DST handling table
 *  - T008: Timezone-rule based offset resolution
 */

#include <time.h>

/**
 * Timezone Profile Structure
 * Defines a timezone with its IANA identifier and DST transition rules
 */
struct TimezoneProfile {
  const char *iana_id;          // e.g., "Europe/Berlin", "UTC", "America/New_York"
  bool has_dst;                 // true if this timezone observes DST
  int standard_offset_seconds;  // UTC offset during standard time (winter)
  int daylight_offset_seconds;  // UTC offset during DST (summer); only used if has_dst=true
};

/**
 * DST Transition Rules Database
 * Each timezone profile defines when DST transitions occur
 */

// DST-OBSERVING TIMEZONES (Europe)

extern const TimezoneProfile PROFILE_EUROPE_BERLIN;
extern const TimezoneProfile PROFILE_EUROPE_LONDON;
extern const TimezoneProfile PROFILE_EUROPE_PARIS;
extern const TimezoneProfile PROFILE_EUROPE_AMSTERDAM;
extern const TimezoneProfile PROFILE_EUROPE_BRUSSELS;
extern const TimezoneProfile PROFILE_EUROPE_VIENNA;
extern const TimezoneProfile PROFILE_EUROPE_PRAGUE;
extern const TimezoneProfile PROFILE_EUROPE_WARSAW;
extern const TimezoneProfile PROFILE_EUROPE_MOSCOW;      // No DST since 2014
extern const TimezoneProfile PROFILE_EUROPE_ISTANBUL;

// DST-OBSERVING TIMEZONES (North America)

extern const TimezoneProfile PROFILE_AMERICA_NEW_YORK;
extern const TimezoneProfile PROFILE_AMERICA_CHICAGO;
extern const TimezoneProfile PROFILE_AMERICA_DENVER;
extern const TimezoneProfile PROFILE_AMERICA_LOS_ANGELES;
extern const TimezoneProfile PROFILE_AMERICA_TORONTO;
extern const TimezoneProfile PROFILE_AMERICA_MEXICO_CITY;

// DST-OBSERVING TIMEZONES (Asia-Pacific)

extern const TimezoneProfile PROFILE_AUSTRALIA_SYDNEY;
extern const TimezoneProfile PROFILE_AUSTRALIA_MELBOURNE;
extern const TimezoneProfile PROFILE_ASIA_TOKYO;
extern const TimezoneProfile PROFILE_ASIA_SHANGHAI;
extern const TimezoneProfile PROFILE_ASIA_HONG_KONG;
extern const TimezoneProfile PROFILE_ASIA_SINGAPORE;
extern const TimezoneProfile PROFILE_ASIA_BANGKOK;
extern const TimezoneProfile PROFILE_ASIA_KOLKATA;
extern const TimezoneProfile PROFILE_ASIA_DUBAI;

// NON-DST TIMEZONES (No seasonal adjustment)

extern const TimezoneProfile PROFILE_UTC;
extern const TimezoneProfile PROFILE_UTC_PLUS_1;
extern const TimezoneProfile PROFILE_UTC_MINUS_5;
extern const TimezoneProfile PROFILE_AFRICA_LAGOS;
extern const TimezoneProfile PROFILE_AFRICA_NAIROBI;

/**
 * Lookup a timezone profile by IANA identifier
 * @param iana_id   IANA timezone identifier (e.g., "Europe/Berlin")
 * @returns pointer to TimezoneProfile, or NULL if not found
 */
const TimezoneProfile *get_timezone_profile(const char *iana_id);

/**
 * Get DST transition information for a given timezone
 * @param profile       The timezone profile
 * @param epoch_utc     Current UTC epoch
 * @param is_dst_now    Output: true if currently in DST, false if in standard time
 * @param next_transition_epoch Output: UTC epoch of next transition (or 0 if none known)
 * @returns offset in seconds from UTC for current time
 */
int get_timezone_offset_and_transition(
  const TimezoneProfile *profile,
  time_t epoch_utc,
  bool *is_dst_now,
  time_t *next_transition_epoch
);

/**
 * List all supported timezone identifiers
 * Used for web UI timezone selector population
 * @returns comma-separated list of IANA identifiers
 */
const char *get_supported_timezones();

#endif
