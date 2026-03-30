#include "clock_config.h"

// Non volatile preferences
Preferences prefs;

// Internal config state
int _clock_mode;
bool _sleep_time[7 * 24];
int _clock_timezone;
char _timezone_id[64];          // IANA timezone identifier (e.g., "Europe/Berlin")
bool _timezone_configured;      // true if timezone_id has been explicitly set
char _time_authority[32];       // "network_ntp" or "browser_manual_fallback"

int _wireless_mode;
char _ssid[64];
char _password[64];

void begin_config()
{
  prefs.begin("clockclock24");
  _clock_mode = prefs.getInt("clock_mode", LAZY);
  _wireless_mode = prefs.getInt("wireless_mode", HOTSPOT);
  _clock_timezone = prefs.getInt("clock_timezone", 0);
  
  // Load timezone identifier (IANA format)
  // On clean flash (no EEPROM), defaults to unconfigured state
  strncpy(_timezone_id, prefs.getString("timezone_id", "").c_str(), sizeof(_timezone_id));
  
  // Load timezone configured flag
  // Clean flash: defaults to false (no timezone set yet)
  _timezone_configured = prefs.getBool("timezone_configured", false);
  
  // Load time authority state
  // Clean flash: defaults to "browser_manual_fallback" (will use browser time until NTP sync)
  strncpy(_time_authority, prefs.getString("time_authority", "browser_manual_fallback").c_str(), sizeof(_time_authority));
  
  strncpy(_ssid, prefs.getString("ssid", "").c_str(), sizeof(_ssid));
  strncpy(_password, prefs.getString("password", "").c_str(), sizeof(_password));
  if(prefs.isKey("sleep_time"))
    prefs.getBytes("sleep_time", _sleep_time, sizeof(_sleep_time));
  else
    memset(_sleep_time, 0, sizeof(_sleep_time));
}

void end_config()
{
  prefs.end();
}

void clear_config()
{
  prefs.clear();
  _clock_mode = LAZY;
  _wireless_mode = HOTSPOT;
  strncpy(_ssid, "", sizeof(_ssid));
  strncpy(_password, "", sizeof(_password));
  strncpy(_timezone_id, "", sizeof(_timezone_id));
  _timezone_configured = false;
  strncpy(_time_authority, "browser_manual_fallback", sizeof(_time_authority));
  memset(_sleep_time, 0, sizeof(_sleep_time));
}

int get_clock_mode()
{
  return _clock_mode;
}

bool get_sleep_time(int day, int hour)
{
  return _sleep_time[(day * 24) + (hour % 24)];
}

int get_connection_mode()
{
  return _wireless_mode;
}

int get_timezone()
{
  return _clock_timezone;
}

char *get_ssid()
{
  return _ssid;
}

char *get_password()
{
  return _password;
}

void set_clock_mode(int value)
{
  _clock_mode = value;
  prefs.putInt("clock_mode", value);
}

void set_sleep_time(int day, int hour, bool value)
{
  _sleep_time[(day * 24) + (hour % 24)] = value;
}

void save_sleep_time()
{
  prefs.putBytes("sleep_time", _sleep_time, sizeof(_sleep_time));
}

void set_connection_mode(int value)
{
  _wireless_mode = value;
  prefs.putInt("wireless_mode", value);
}

void set_timezone(int value)
{
  _clock_timezone = value;
  prefs.putInt("clock_timezone", value);
}

void set_ssid(const char *value)
{
  strncpy(_ssid, value, sizeof(_ssid));
  prefs.putString("ssid", value);
}

void set_password(const char *value)
{
  strncpy(_password, value, sizeof(_password));
  prefs.putString("password", value);
}

/* ========== New DST Support Functions ========== */

const char *get_timezone_id()
{
  return _timezone_id;
}

bool get_timezone_configured()
{
  return _timezone_configured;
}

const char *get_time_authority()
{
  return _time_authority;
}

void set_timezone_id(const char *value)
{
  strncpy(_timezone_id, value, sizeof(_timezone_id));
  _timezone_configured = (value && strlen(value) > 0);  // Auto-set configured flag
  prefs.putString("timezone_id", value);
  prefs.putBool("timezone_configured", _timezone_configured);
}

void set_timezone_configured(bool configured)
{
  _timezone_configured = configured;
  prefs.putBool("timezone_configured", configured);
}

void set_time_authority(const char *authority)
{
  strncpy(_time_authority, authority, sizeof(_time_authority));
  prefs.putString("time_authority", authority);
}