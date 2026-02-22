#include <Arduino.h>
#include <Wire.h>
#include <TimeLib.h>

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
  // Bug #3 fix: limit I2C clock stretching so a crashed/glitching slave
  // cannot hold SDA low and lock up Wire.endTransmission() indefinitely.
  Wire.setClockStretchLimit(1500);
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
    // Bug #4 fix: begin_NTP() resolves the NTP IP once (blocking, but only
    // at startup). The actual sync is now done non-blocking via tick_NTP()
    // called from loop(), replacing the old blocking setSyncProvider approach.
    begin_NTP();
    set_ntp_timezone(get_timezone());
  }

  // Starts web server
  server_start();
}

void loop() {

  if(get_connection_mode() == HOTSPOT && is_time_changed_browser())
  {
    t_browser_time browser_time = get_browser_time();
    setTime(browser_time.hour, 
      browser_time.minute, 
      browser_time.second, 
      browser_time.day, 
      browser_time.month,  
      browser_time.year);
  }

  if(get_connection_mode() == EXT_CONN && get_timezone() != get_ntp_timezone())
  {
    // Timezone changed: update the offset and trigger an immediate re-sync.
    // Previously this called setSyncProvider() again (blocking); now we just
    // request a new sync on the next tick_NTP() call.
    set_ntp_timezone(get_timezone());
    request_ntp_sync();
  }

  // Non-blocking NTP tick: sends packet and reads reply across separate loop()
  // iterations so handle_webclient() is never starved.
  if(get_connection_mode() == EXT_CONN)
    tick_NTP();

  get_clock_mode() != OFF ? set_time() : stop();

  update_MDNS();
  handle_webclient();
}

void set_time()
{
  int day_week = (weekday() + 5) % 7;
  if(get_sleep_time(day_week, hour()))
    stop();
  else if(hour() != last_hour || minute() != last_minute)
  {
    is_stopped = false;
    last_hour = hour();
    last_minute = minute();
    switch(get_clock_mode())
    {
      case LAZY:
        set_lazy();
        break;
      case FUN:
        set_fun();
        break;
      case WAVES:
        set_waves();
        break;
    }
  }
}

void set_lazy()
{
  set_speed(200);
  set_acceleration(100);
  set_direction(MIN_DISTANCE);
  set_clock_time(last_hour, last_minute);
}

void set_fun()
{
  set_speed(400);
  set_acceleration(150);
  set_direction(CLOCKWISE2);
  set_clock_time(last_hour, last_minute);
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
  t_full_clock clock = get_clock_state_from_time(last_hour, last_minute);
  for (int i = 0; i <8; i++)
  {
    set_half_digit(i, clock.digit[i/2].halfs[i%2]);
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
  for (int i = 0; i < value/100; i++)
  {
    ESP.wdtFeed(); // prevent watchdog reboot during long animations
    update_MDNS();
    handle_webclient();
    delay(value/100);
  }
}