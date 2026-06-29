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

void set_time();
void set_lazy();
void set_fun();
void set_waves();
void set_propeller();
void set_arrow();
void set_ripple();
void set_bubble();
void set_gear();
void set_scatter();
void set_diagonal();
void set_cascade();
void set_cycle();
void dispatch_animation(int mode);
void stop();
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
    
    // Timing instrumentation for noise reduction validation
    unsigned long anim_start_ms = millis();
    const char* mode_name = "UNKNOWN";
    switch(get_clock_mode())
    {
      case LAZY:       mode_name = "LAZY"; break;
      case FUN:        mode_name = "FUN"; break;
      case WAVES:      mode_name = "WAVES"; break;
      case PROPELLER:  mode_name = "PROPELLER"; break;
      case ARROW:      mode_name = "ARROW"; break;
      case RIPPLE:     mode_name = "RIPPLE"; break;
      case BUBBLE:     mode_name = "BUBBLE"; break;
      case GEAR:       mode_name = "GEAR"; break;
      case SCATTER:    mode_name = "SCATTER"; break;
      case DIAGONAL:   mode_name = "DIAGONAL"; break;
      case CASCADE:    mode_name = "CASCADE"; break;
      case CYCLE:      mode_name = "CYCLE"; break;
    }
    int sh = get_silent_hour();
    if (sh >= 0 && hour() >= sh)
      set_lazy();
    else
      dispatch_animation(get_clock_mode());

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
  _delay(5000);
  set_speed(400);
  set_acceleration(100);
  set_direction(CLOCKWISE2);
  t_full_clock clock_state = get_clock_state_from_time(last_hour, last_minute);
  for (int i = 0; i <8; i++)
  {
    set_half_digit_staggered(i, clock_state.digit[i/2].halfs[i%2]);
    delay(100);
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
  for (int i = 0; i < value / 100; i++)
  {
    update_MDNS();
    handle_webclient();
    delay(100);
  }
}

void dispatch_animation(int mode)
{
  switch(mode)
  {
    case LAZY:      set_lazy();      break;
    case FUN:       set_fun();       break;
    case WAVES:     set_waves();     break;
    case PROPELLER: set_propeller(); break;
    case ARROW:     set_arrow();     break;
    case RIPPLE:    set_ripple();    break;
    case BUBBLE:    set_bubble();    break;
    case GEAR:      set_gear();      break;
    case SCATTER:   /* disabled — requires slave reflash for COUNTERCLOCKWISE5 */
    case DIAGONAL:  set_diagonal();  break;
    case CASCADE:   set_cascade();   break;
    case CYCLE:     set_cycle();     break;
  }
}

void set_propeller()
{
  set_speed(800);
  set_acceleration(200);
  set_direction(CLOCKWISE);
  t_full_clock clock = get_clock_state_from_time(last_hour, last_minute);
  for (int i = 0; i < 8; i++)
  {
    t_half_digit hd = get_full_half_digit(clock.digit[i/2].halfs[i%2]);
    for (int j = 0; j < 3; j++)
    {
      hd.clocks[j].mode_h = CLOCKWISE;
      hd.clocks[j].mode_m = COUNTERCLOCKWISE;
    }
    set_half_digit_full(i, hd);
  }
}

void set_arrow()
{
  set_speed(800);
  set_acceleration(200);
  set_direction(MIN_DISTANCE);
  set_clock(d_joint);
  _delay(5000);

  set_speed(800);
  set_acceleration(200);
  set_direction(CLOCKWISE);

  t_full_clock target = get_clock_state_from_time(last_hour, last_minute);

  const int MIN_PROJ = -14;
  const int MAX_PROJ = 14;
  for (int proj = MIN_PROJ; proj <= MAX_PROJ; proj++)
  {
    bool group_has_clock = false;
    for (int hd = 0; hd < 8; hd++)
    {
      t_half_digitl lite = target.digit[hd / 2].halfs[hd % 2];
      for (int p = 0; p < 3; p++)
      {
        int col = hd;
        int row = p;
        if (2 * col - 7 * row == proj)
        {
          set_single_clock_full(hd, p, lite, CLOCKWISE, COUNTERCLOCKWISE);
          group_has_clock = true;
        }
      }
    }
    if (group_has_clock && proj < MAX_PROJ)
      delay(25);
  }
}

void set_ripple()
{
  set_speed(800);
  set_acceleration(200);
  set_direction(MIN_DISTANCE);
  set_clock(d_WAVE);
  _delay(5000);
  set_speed(800);
  set_acceleration(200);
  set_direction(CLOCKWISE);

  t_full_clock target = get_clock_state_from_time(last_hour, last_minute);

  const int MAX_DIST = 4;
  for (int d = 0; d <= MAX_DIST; d++)
  {
    for (int hd = 0; hd < 8; hd++)
    {
      int mode_h = (hd < 4) ? CLOCKWISE : COUNTERCLOCKWISE;
      int mode_m = (hd < 4) ? COUNTERCLOCKWISE : CLOCKWISE;
      t_half_digitl lite = target.digit[hd / 2].halfs[hd % 2];
      for (int p = 0; p < 3; p++)
      {
        float col_dist = fabsf((float)hd - 3.5f);
        float row_dist = fabsf((float)p - 1.0f);
        int dist = (int)(col_dist + row_dist);
        if (dist == d)
          set_single_clock_full(hd, p, lite, mode_h, mode_m);
      }
    }
    if (d < MAX_DIST)
      delay(250);
  }
}

void set_bubble()
{
  set_speed(800);
  set_acceleration(200);
  set_direction(MIN_DISTANCE);
  set_clock(d_bubble);
  _delay(5000);
  set_speed(800);
  set_acceleration(200);
  set_direction(CLOCKWISE);
  t_full_clock clock = get_clock_state_from_time(last_hour, last_minute);
  for (int i = 0; i < 8; i++)
  {
    t_half_digit hd = get_full_half_digit(clock.digit[i/2].halfs[i%2]);
    for (int j = 0; j < 3; j++)
    {
      bool hour_cw = ((i + j) % 2 == 0);
      if (i % 2 == 0)
        hour_cw = !hour_cw;
      if (hour_cw)
      {
        hd.clocks[j].mode_h = CLOCKWISE;
        hd.clocks[j].mode_m = COUNTERCLOCKWISE;
      }
      else
      {
        hd.clocks[j].mode_h = COUNTERCLOCKWISE;
        hd.clocks[j].mode_m = CLOCKWISE;
      }
    }
    set_half_digit_full(i, hd);
  }
}

void set_gear()
{
  set_speed(800);
  set_acceleration(200);
  set_direction(MIN_DISTANCE);
  set_clock(d_CENT);
  _delay(5000);

  set_speed(800);
  set_acceleration(200);
  set_direction(CLOCKWISE);

  t_full_clock target = get_clock_state_from_time(last_hour, last_minute);

  const int MAX_GROUP = 3;
  for (int g = 0; g <= MAX_GROUP; g++)
  {
    for (int hd = 0; hd < 8; hd++)
    {
      t_half_digitl lite = target.digit[hd / 2].halfs[hd % 2];
      for (int p = 0; p < 3; p++)
      {
        int col_dist = min(abs(hd - 3), abs(hd - 4));
        int row_dist = abs(p - 1);
        int group = max(col_dist, row_dist);
        if (group == g)
          set_single_clock_full(hd, p, lite, CLOCKWISE, CLOCKWISE);
      }
    }
    if (g < MAX_GROUP)
      delay(300);
  }
}

void set_scatter()
{
  // Known limitation: COUNTERCLOCKWISE5 (value 13) is not recognised by
  // unflashed slave firmware. Minute hand rotation will be incorrect but
  // the final time angle is still reached correctly.
  set_speed(400);
  set_acceleration(150);
  set_direction(COUNTERCLOCKWISE3);

  t_full_clock clock = get_clock_state_from_time(last_hour, last_minute);

  for (int i = 0; i < 8; i++)
  {
    t_half_digit hd = get_full_half_digit(clock.digit[i/2].halfs[i%2]);
    for (int j = 0; j < 3; j++)
    {
      hd.clocks[j].mode_h = COUNTERCLOCKWISE3;
      hd.clocks[j].mode_m = COUNTERCLOCKWISE5;
      hd.clocks[j].speed_m = 800;
    }
    set_half_digit_full(i, hd);
    if (i < 7)
      delay(100);
  }
}

void set_diagonal()
{
  set_speed(800);
  set_acceleration(200);
  set_direction(MIN_DISTANCE);
  set_clock(d_diagonal);
  _delay(5000);
  set_speed(800);
  set_acceleration(200);
  set_direction(CLOCKWISE);
  t_full_clock clock = get_clock_state_from_time(last_hour, last_minute);
  for (int i = 0; i < 8; i++)
  {
    set_half_digit(i, clock.digit[i/2].halfs[i%2]);
    delay(100);
  }
}

void set_cascade()
{
  set_speed(800);
  set_acceleration(200);
  set_direction(MIN_DISTANCE);
  set_clock(d_stop);
  _delay(5000);

  set_speed(800);
  set_acceleration(200);
  set_direction(COUNTERCLOCKWISE);

  t_full_clock clock = get_clock_state_from_time(last_hour, last_minute);

  for (int i = 0; i < 8; i++)
  {
    t_half_digit hd = get_full_half_digit(clock.digit[i/2].halfs[i%2]);
    for (int j = 0; j < 3; j++)
    {
      hd.clocks[j].mode_h = COUNTERCLOCKWISE;
      hd.clocks[j].mode_m = COUNTERCLOCKWISE;
    }
    set_half_digit_full(i, hd);
    if (i < 7)
      delay(100);
  }
}

void set_cycle()
{
  static const int cycle_order[] = {
    FUN, WAVES, ARROW, RIPPLE, BUBBLE, PROPELLER, DIAGONAL, GEAR, CASCADE
    // SCATTER disabled — requires slave reflash for COUNTERCLOCKWISE5
  };
  static const int cycle_count = sizeof(cycle_order) / sizeof(cycle_order[0]);
  int minutes_today = last_hour * 60 + last_minute;
  int mode = cycle_order[minutes_today % cycle_count];
  dispatch_animation(mode);
}