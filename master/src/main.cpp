#include <Arduino.h>
#include <Wire.h>
#include <TimeLib.h>
#include <string.h>

#include "i2c.h"
#include "clock_state.h"
#include "clock_manager.h"
#include "digit.h"
#include "wifi_utils.h"
#include "web_server.h"
#include "clock_config.h"
#include "ntp.h"

int last_hour = -1;
int last_minute = -1;
bool is_stopped = false;
bool last_ntp_synced = false;
int last_registered_timezone = -999;  // Track last timezone we registered for NTP
bool last_in_dst_window = false;      // Track if we were in DST transition window last loop

static time_t ntp_sync_provider_with_timezone()
{
  const char *tz = get_timezone_id();
  return get_NTP_time_with_timezone((tz && strlen(tz) > 0) ? tz : "UTC");
}

/**
 * Sets clock to the current time
*/
void set_time();

/**
 * Sets clock time using lazy animation
*/
void set_lazy();

/**
 * Sets clock time using fun animation
*/
void set_fun();

/**
 * Sets clock time using waves animation
*/
void set_waves();

/**
 * Sets clock to stop state
*/
void stop();

/**
 * Custom delay to update web clients
 * @param value   time in milliseconds
*/
void _delay(int value);

void setup() {
  Serial.begin(115200);
  Serial.println("\nclockclock24 replica by Vallasc master v1.0");
  delay(3000);
  // Load configuration from EEPROM
  begin_config();

  Wire.begin();
  pinMode(LED_BUILTIN, OUTPUT);

  if(get_connection_mode() == HOTSPOT)
    wifi_create_AP("ClockClock 24", "clockclock24");
  else if( !wifi_connect(get_ssid(), get_password(), "clockclock24") )
  {
    set_connection_mode(HOTSPOT);
    wifi_create_AP("ClockClock 24", "clockclock24");
  }

  if(get_connection_mode() == EXT_CONN)
  {
    // Initialize NTP
    begin_NTP();
    setSyncProvider(ntp_sync_provider_with_timezone);
    // Sync every 30 minutes
    setSyncInterval(60 * 30);
  }

  // Starts web server
  server_start();
}

void loop() {

  if (is_time_changed_browser())
  {
    if (get_connection_mode() == HOTSPOT || timeStatus() != timeSet)
    {
      t_browser_time browser_time = get_browser_time();
      setTime(browser_time.hour,
        browser_time.minute,
        browser_time.second,
        browser_time.day,
        browser_time.month,
        browser_time.year);
      set_time_authority("browser_manual_fallback");
      Serial.println("Applied browser/manual time (fallback mode)");
    }
    else if (get_connection_mode() == EXT_CONN)
    {
      // Timezone or time payload changed while NTP is authoritative:
      // apply immediately by forcing a one-shot NTP resync with current timezone_id.
      time_t synced = ntp_sync_provider_with_timezone();
      if (synced > 0)
      {
        setTime(synced);
        set_time_authority("network_ntp");
        Serial.println("Applied timezone change via immediate NTP resync");
      }
      else
      {
        Serial.println("Immediate NTP resync failed after timezone change");
      }
    }
  }

  if(get_connection_mode() == EXT_CONN && get_timezone() != get_ntp_timezone())
  {
    int current_tz = get_timezone();
    set_ntp_timezone(current_tz);
    // Only re-register if timezone changed from last time we registered
    if (current_tz != last_registered_timezone)
    {
      last_registered_timezone = current_tz;
      setSyncProvider(ntp_sync_provider_with_timezone);
      Serial.printf("Registered NTP provider for timezone offset\n");
    }
  }

  if (get_connection_mode() == EXT_CONN)
  {
    bool ntp_synced = (timeStatus() == timeSet);
    if (ntp_synced)
    {
      set_time_authority("network_ntp");
      // Only re-register provider on first successful sync (transition from not synced -> synced)
      if (!last_ntp_synced)
      {
        last_registered_timezone = get_timezone();
        setSyncProvider(ntp_sync_provider_with_timezone);
        Serial.println("NTP synced, registered timezone-aware provider");
      }
    }
    last_ntp_synced = ntp_synced;
  }

  // Wi-Fi reconnection watchdog: if the router dropped us, attempt to rejoin every 30 s
  if (get_connection_mode() == EXT_CONN && !is_connected())
  {
    static unsigned long _last_reconnect_ms = 0;
    if (millis() - _last_reconnect_ms > 30000UL)
    {
      _last_reconnect_ms = millis();
      Serial.println("Wi-Fi lost, reconnecting...");
      WiFi.begin(get_ssid(), get_password());
    }
  }

  get_clock_mode() != OFF ? set_time() : stop();

  update_MDNS();
  handle_webclient();
}

void set_time()
{
  time_t next_transition = get_next_dst_transition();
  bool in_dst_window = false;
  
  if (next_transition > 0)
  {
    long seconds_to_transition = (long)(next_transition - now());
    in_dst_window = (seconds_to_transition >= 0 && seconds_to_transition <= 60);
    
    // Only log and resync when entering transition window
    if (in_dst_window && !last_in_dst_window)
    {
      setSyncProvider(ntp_sync_provider_with_timezone);
      Serial.printf("DST transition window active, forcing sync (in %ld sec)\n", seconds_to_transition);
    }
  }
  last_in_dst_window = in_dst_window;

  int day_week = (weekday() + 5) % 7;
  if(get_sleep_time(day_week, hour()))
    stop();
  else if(hour() != last_hour || minute() != last_minute)
  {
    is_stopped = false;
    last_hour = hour();
    last_minute = minute();
    
    // T013: Timing instrumentation for noise reduction validation
    unsigned long anim_start_ms = millis();
    const char* mode_name = "UNKNOWN";
    
    switch(get_clock_mode())
    {
      case LAZY:
        mode_name = "LAZY";
        set_lazy();
        break;
      case FUN:
        mode_name = "FUN";
        set_fun();
        break;
      case WAVES:
        mode_name = "WAVES";
        set_waves();
        break;
    }
    
    unsigned long anim_end_ms = millis();
    unsigned long anim_duration_ms = anim_end_ms - anim_start_ms;
    Serial.printf("Anim[%s] duration: %lu ms\n", mode_name, anim_duration_ms);
  }
}

void set_lazy()
{
  set_speed(200);
  set_acceleration(100);
  set_direction(MIN_DISTANCE);
  set_clock_time_staggered(last_hour, last_minute);
}

void set_fun()
{
  set_speed(400);
  set_acceleration(150);
  set_direction(CLOCKWISE2);
  set_clock_time_staggered(last_hour, last_minute);
}

void set_waves()
{
  set_speed(800);
  set_acceleration(150);
  set_direction(MIN_DISTANCE);
  set_clock(d_IIII);
  _delay(9000);
  set_speed(400);
  set_acceleration(100);
  set_direction(CLOCKWISE2);
  t_full_clock clock_state = get_clock_state_from_time(last_hour, last_minute);
  for (int i = 0; i <8; i++)
  {
    set_half_digit_staggered(i, clock_state.digit[i/2].halfs[i%2]);
    delay(400);
  }
}

void stop()
{
  if(!is_stopped)
  {
    is_stopped = true;
    last_hour = -1;
    last_minute = -1;
    set_direction(MIN_DISTANCE);
    set_speed(200);
    set_acceleration(100);
    set_clock(d_stop);
  }
}

void _delay(int value)
{
  for (int i = 0; i <value/100; i++)
  {
    update_MDNS();
    handle_webclient();
    delay(value/100);
  }
}