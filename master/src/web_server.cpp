#include "web_server.h"
#include <TimeLib.h>

ESP8266WebServer _server(80);

t_browser_time _browser_time = {0, 0, 0, 0, 0, 0};
bool _time_changed_browser = false;
bool _restart_requested = false;
unsigned long _restart_after_ms = 0;

void server_start()
{
  // Setup web server connection
  _server.enableCORS(true);
  _server.begin();
  _server.on("/", HTTP_GET, handle_get);
  _server.on("/config", HTTP_GET, handle_get_config);
  _server.on("/now", HTTP_GET, handle_get_now);
  _server.on("/time", HTTP_POST, handle_post_time);
  _server.on("/adjust", HTTP_POST, handle_post_adjust);
  _server.on("/mode", HTTP_POST, handle_post_mode);
  _server.on("/sleep", HTTP_POST, handle_post_sleep);
  _server.on("/silent", HTTP_POST, handle_post_silent);
  _server.on("/connection", HTTP_POST, handle_post_connection);
  Serial.println("WebServer setup done");
}

void handle_webclient()
{
  _server.handleClient();

  if (_restart_requested && millis() >= _restart_after_ms)
  {
    Serial.println("Rebooting to apply wireless configuration");
    delay(50);
    ESP.restart();
  }
}

void server_stop()
{
  _server.close();
}

void handle_get()
{
  Serial.println("Handle GET /");
  _server.send(200, "text/html", WEB_PAGE);
}

void handle_get_now()
{
  Serial.println("Handle GET /now");
  char payload[256];
  snprintf(payload, sizeof(payload),
    "{\"h\":%d,\"m\":%d,\"s\":%d,\"day\":%d}",
    hour(), minute(), second(), ((weekday() + 5) % 7));
  _server.send(200, "application/json", payload);
}

void handle_get_config()
{
  Serial.println("Handle GET /config");\
  char payload[1024];
  {
    char s_time[512] = "[";
    for (int i = 0; i < 7; i++)
    {
      strncat(s_time, "[", 2);
      for (int j = 0; j < 24; j++)
      {
        strncat(s_time, get_sleep_time(i, j) ? "1" : "0", 2);
        if(j < 23)
          strncat(s_time,",", 2);
      }
      strncat(s_time, "]", 2);
      if(i < 6)
        strncat(s_time,",", 2);
    }
    strncat(s_time, "]", 2);
    snprintf(payload, sizeof(payload),
      "{\"clock_mode\":%d,"
      "\"silent_hour\":%d,"
      "\"wireless_mode\":%d,"
      "\"ssid\":\"%s\","
      "\"password\":\"%s\","
      "\"sleep_time\":%s,"
      "\"timezone_id\":\"%s\","
      "\"timezone_configured\":%s,"
      "\"time_authority\":\"%s\"}",
      get_clock_mode(),
      get_silent_hour(),
      get_connection_mode(),
      get_ssid(),
      get_password(),
      s_time,
      get_timezone_id(),
      get_timezone_configured() ? "true" : "false",
      get_time_authority());
  }
  _server.send(200, "application/json", payload);
}

void handle_post_time()
{
  Serial.println("Handle POST /time");
  // With external connection enabled, NTP is authoritative and browser/manual
  // timestamp must not override time unless fallback mode is active.
  if (get_connection_mode() == EXT_CONN && _server.hasArg("h"))
  {
    _server.send(409, "text/plain", "network_ntp_authoritative");
    Serial.println("Ignored browser/manual time update: network authority active");
    return;
  }

  if (_server.hasArg("timezone_id"))
  {
    String tz_id = _server.arg("timezone_id");
    set_timezone_id(tz_id.c_str());
    set_timezone_configured(tz_id.length() > 0);
    Serial.printf("Timezone ID set: %s\n", tz_id.c_str());
  }

  if (_server.hasArg("h"))
    _browser_time.hour = _server.arg("h").toInt();
  if (_server.hasArg("m"))
    _browser_time.minute = _server.arg("m").toInt();
  if (_server.hasArg("s"))
    _browser_time.second = _server.arg("s").toInt();
  if (_server.hasArg("D"))
    _browser_time.day = _server.arg("D").toInt();
  if (_server.hasArg("M"))
    _browser_time.month = _server.arg("M").toInt();
  if (_server.hasArg("Y"))
    _browser_time.year = _server.arg("Y").toInt();
  if (_server.hasArg("timezone"))
  {
    int _browser_timezone = _server.arg("timezone").toInt();
    set_timezone(_browser_timezone);
  }
  set_time_authority("browser_manual_fallback");
  _time_changed_browser = true;
  _server.send(200, "text/plain", "");
  Serial.printf("Time received: %d:%d:%d\n", 
    _browser_time.hour, _browser_time.minute, _browser_time.second);
}

void handle_post_adjust()
{
  Serial.println("Handle POST /adjust");
  int clock_index = 0;
  int m_amount = 0;
  int h_amount = 0;
  if (_server.hasArg("index"))
    clock_index = _server.arg("index").toInt();
  if (_server.hasArg("m_amount"))
    m_amount = _server.arg("m_amount").toInt();
  if (_server.hasArg("h_amount"))
    h_amount = _server.arg("h_amount").toInt();

  _server.send(200, "text/plain", "");

  Serial.printf("Adjust received, clock: %d, m_amount: %d, h_amount: %d\n", 
    clock_index, m_amount, h_amount);
  adjust_hands(clock_index, m_amount, h_amount);
}

void handle_post_mode()
{
  Serial.println("Handle POST /mode");
  if (_server.hasArg("mode"))
    set_clock_mode(_server.arg("mode").toInt());
  _server.send(200, "text/plain", "");
}

extern int last_minute;

void handle_post_silent()
{
  Serial.println("Handle POST /silent");
  if (_server.hasArg("silent_hour"))
    set_silent_hour(_server.arg("silent_hour").toInt());
  last_minute = -1; // force immediate re-evaluation in set_time()
  _server.send(200, "text/plain", "");
}

void handle_post_sleep()
{
  Serial.println("Handle POST /sleep");
  if (_server.hasArg("day"))
  {
    int sleep_day = _server.arg("day").toInt();
    for(int i = 0; i < 24; i++)
    {
      char arg[8];
      snprintf(arg, sizeof(arg), "h%d", i);
      if (_server.hasArg(arg))
        set_sleep_time(sleep_day, i, _server.arg(arg).toInt() == 0 ? false : true);
    }
    save_sleep_time();
  }
  _server.send(200, "text/html", "");
}

void handle_post_connection()
{
  Serial.println("Handle POST /connection");

  int new_mode = get_connection_mode();
  String new_ssid = get_ssid();
  String new_password = get_password();

  if (_server.hasArg("mode"))
    new_mode = _server.arg("mode").toInt();
  if (_server.hasArg("ssid"))
    new_ssid = _server.arg("ssid");
  if (_server.hasArg("password"))
    new_password = _server.arg("password");

  bool changed = (new_mode != get_connection_mode()) ||
    (new_ssid != String(get_ssid())) ||
    (new_password != String(get_password()));

  if (changed)
  {
    set_connection_mode(new_mode);
    set_ssid(new_ssid.c_str());
    set_password(new_password.c_str());
  }

  _server.send(200, "text/plain", changed ? "restarting" : "no_changes");

  if (changed && !_restart_requested)
  {
    end_config();
    _restart_requested = true;
    _restart_after_ms = millis() + 500;
    Serial.println("Wireless configuration changed, restart scheduled");
  }
  else if (!changed)
  {
    Serial.println("Wireless configuration unchanged, no restart");
  }
}

bool is_time_changed_browser()
{
  bool tmp = _time_changed_browser;
  _time_changed_browser = false;
  return tmp;
}

t_browser_time get_browser_time()
{
  return _browser_time;
}