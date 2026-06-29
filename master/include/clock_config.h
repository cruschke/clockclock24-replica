#ifndef CLOCK_CONFIG_H
#define CLOCK_CONFIG_H

#include <Preferences.h>

/** 
 * Clock connection's modes
 */
enum wireless_modes
{
  HOTSPOT,
  EXT_CONN
};

/**
 * Clock animation's modes
 */
enum clock_modes
{
  LAZY,
  FUN,
  WAVES,
  PROPELLER,
  ARROW,
  RIPPLE,
  BUBBLE,
  GEAR,
  SCATTER,
  DIAGONAL,
  CASCADE,
  CYCLE
};

// Virtual mode — not part of clock_modes enum; stored as 255 in EEPROM
#define OFF 255

/**
 * Load configuration from the EEPROM
 */
void begin_config();

/**
 * Clear EEPROM configuration
 */
void clear_config();

/**
 * Closes the preferencies object
 */
void end_config();

/**
 * Get current clock mode
 */
int get_clock_mode();

/**
 * Gets current sleep time at a given day and hour
 * @param day   day of the week
 * @param hour  hour of the day
 */
bool get_sleep_time(int day, int hour);

/**
 * Gets current connection mode
 */
int get_connection_mode();

/**
 * Gets current time zone based on UTC offset (legacy, deprecated)
 */
int get_timezone();

/**
 * Gets current timezone identifier (IANA format, e.g., "Europe/Berlin")
 * Returns pointer to string in EEPROM preferences; valid until next call
 */
const char *get_timezone_id();

/**
 * Gets whether timezone has been explicitly configured
 * true = timezone_id is set and valid; false = unconfigured (clean-flash state)
 */
bool get_timezone_configured();

/**
 * Gets current time authority source
 * Returns "network_ntp" or "browser_manual_fallback"
 */
const char *get_time_authority();

/**
 * Gets current SSID
 */
char *get_ssid();

/**
 * Gets current password
 */
char *get_password();

/**
 * Sets clock mode
 * @param value   mode value of type clock_modes
 */
void set_clock_mode(int value);

/**
 *  Sets current sleep time at a given day and hour
 * @param day   day of the week
 * @param hour  hour of the day
 * @param value true if clock is  disabled, false otherwise
 */
void set_sleep_time(int day, int hour, bool value);

/**
 *  Saves sleep time array on EEPROM
 */
void save_sleep_time();

/**
 *  Sets connection mode
 * @param value   mode value of type wireless_modes
 */
void set_connection_mode(int value);

/**
 *  Sets the time zone (legacy, deprecated)
 * @param value   time zone based on UTC offset
 */
void set_timezone(int value);

/**
 *  Sets the timezone identifier (IANA format, e.g., "Europe/Berlin")
 * Automatically sets timezone_configured=true on successful set
 * @param value   IANA timezone identifier string
 */
void set_timezone_id(const char *value);

/**
 *  Sets timezone configuration state
 * @param configured   true if user has explicitly set timezone_id; false for unconfigured state
 */
void set_timezone_configured(bool configured);

/**
 *  Sets the time authority source
 * @param authority   "network_ntp" or "browser_manual_fallback"
 */
void set_time_authority(const char *authority);

/**
 *  Sets SSID value
 * @param value   SSID string
 */
void set_ssid(const char *value);

/**
 *  Sets password value
 * @param value   password string
 */
void set_password(const char *value);

#endif