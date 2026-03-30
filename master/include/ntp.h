#ifndef NTP_H
#define NTP_H

#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <TimeLib.h>
#include <time.h>
#include <string.h>
#include "timezone_profiles.h"

// NTP Servers:
static const char ntp_server_name[] = "pool.ntp.org";
//static const char ntp_server_name[] = "time.nist.gov";

int _time_zone = 1; // Central European Time (legacy, deprecated)
const TimezoneProfile *_current_timezone_profile = NULL;  // Current timezone profile (new DST support)
static char _cached_timezone_id[64] = "";  // Cache last loaded timezone to avoid repeated loads

// const int time_zone = -5;  // Eastern Standard Time (USA)
// const int time_zone = -4;  // Eastern Daylight Time (USA)
// const int time_zone = -8;  // Pacific Standard Time (USA)
// const int time_zone = -7;  // Pacific Daylight Time (USA)

WiFiUDP Udp;
unsigned int local_port = 8888; // local port to listen for UDP packets

const int NTP_PACKET_SIZE = 48;     // NTP time is in the first 48 bytes of message
byte packet_buffer[NTP_PACKET_SIZE]; // buffer to hold incoming & outgoing packets

void begin_NTP();
time_t get_NTP_time();
time_t get_NTP_time_with_timezone(const char *timezone_id);
void send_NTP_packet(IPAddress &address);
void set_ntp_timezone_profile(const char *iana_id);
const TimezoneProfile *get_ntp_timezone_profile();
int get_local_time_offset();
time_t get_next_dst_transition();

void begin_NTP()
{
  Udp.begin(local_port);
}

time_t get_NTP_time()
{
  IPAddress ntp_server_IP; // NTP server's ip address

  while (Udp.parsePacket() > 0)
    ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntp_server_name, ntp_server_IP);
  Serial.print(ntp_server_name);
  Serial.print(": ");
  Serial.println(ntp_server_IP);
  send_NTP_packet(ntp_server_IP);
  uint32_t begin_wait = millis();
  while (millis() - begin_wait < 1500)
  {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE)
    {
      Serial.println("Receive NTP Response");
      Udp.read(packet_buffer, NTP_PACKET_SIZE); // read packet into the buffer
      unsigned long secs_since_1900;
      // convert four bytes starting at location 40 to a long integer
      secs_since_1900 = (unsigned long)packet_buffer[40] << 24;
      secs_since_1900 |= (unsigned long)packet_buffer[41] << 16;
      secs_since_1900 |= (unsigned long)packet_buffer[42] << 8;
      secs_since_1900 |= (unsigned long)packet_buffer[43];
      return secs_since_1900 - 2208988800UL + _time_zone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void send_NTP_packet(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packet_buffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packet_buffer[0] = 0b11100011; // LI, Version, Mode
  packet_buffer[1] = 0;          // Stratum, or type of clock
  packet_buffer[2] = 6;          // Polling Interval
  packet_buffer[3] = 0xEC;       // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packet_buffer[12] = 49;
  packet_buffer[13] = 0x4E;
  packet_buffer[14] = 49;
  packet_buffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); // NTP requests are to port 123
  Udp.write(packet_buffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

void set_ntp_timezone(int value)
{
  _time_zone = value;
}

int get_ntp_timezone()
{
  return _time_zone;
}

/* ========== New DST Support Functions (T007, T009) ========== */

/**
 * Set the NTP timezone profile by IANA identifier
 * This is called during initialization and when user changes timezone configuration
 */
void set_ntp_timezone_profile(const char *iana_id)
{
  if (iana_id && strlen(iana_id) > 0) {
    // Only load profile if timezone changed to avoid repeated Serial.println
    if (strncmp(_cached_timezone_id, iana_id, sizeof(_cached_timezone_id)) != 0) {
      strncpy(_cached_timezone_id, iana_id, sizeof(_cached_timezone_id) - 1);
      _current_timezone_profile = get_timezone_profile(iana_id);
      Serial.print("Set timezone profile: ");
      Serial.println(iana_id);
    }
  } else {
    // Clear cache if timezone_id is empty
    if (strlen(_cached_timezone_id) > 0) {
      memset(_cached_timezone_id, 0, sizeof(_cached_timezone_id));
      _current_timezone_profile = NULL;
      Serial.println("Timezone profile cleared");
    }
  }
}

/**
 * Get the current timezone profile
 */
const TimezoneProfile *get_ntp_timezone_profile()
{
  return _current_timezone_profile;
}

/**
 * Get the current UTC offset in seconds
 * Takes DST rules into account if timezone profile is set
 */
int get_local_time_offset()
{
  if (!_current_timezone_profile) {
    // Fallback to legacy behavior if no timezone profile set
    return _time_zone * SECS_PER_HOUR;
  }
  
  // Use timezone-aware offset calculation
  time_t now_utc = now();  // Get current UTC time from TimeLib
  bool is_dst;
  time_t next_transition;
  return get_timezone_offset_and_transition(_current_timezone_profile, now_utc, &is_dst, &next_transition);
}

/**
 * Get the next DST transition time (in UTC epoch)
 * Returns 0 if no transition is known
 */
time_t get_next_dst_transition()
{
  if (!_current_timezone_profile) {
    return 0;  // No timezone configured
  }
  // Guard against calling now() before time is synced: it would trigger
  // a new NTP request every loop iteration causing a flood.
  if (timeStatus() == timeNotSet) {
    return 0;
  }
  time_t now_utc = now();
  bool is_dst;
  time_t next_transition;
  get_timezone_offset_and_transition(_current_timezone_profile, now_utc, &is_dst, &next_transition);
  return next_transition;
}

/**
 * Get NTP time and apply timezone/DST offset
 * This is the timezone-aware version of get_NTP_time() for use in main.cpp
 */
time_t get_NTP_time_with_timezone(const char *timezone_id)
{
  IPAddress ntp_server_IP;

  while (Udp.parsePacket() > 0)
    ;  // discard any previously received packets
  Serial.println("Transmit NTP Request");
  
  // Get a random server from the pool
  WiFi.hostByName(ntp_server_name, ntp_server_IP);
  Serial.print(ntp_server_name);
  Serial.print(": ");
  Serial.println(ntp_server_IP);
  
  send_NTP_packet(ntp_server_IP);
  
  uint32_t begin_wait = millis();
  while (millis() - begin_wait < 1500)
  {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE)
    {
      Serial.println("Receive NTP Response");
      Udp.read(packet_buffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secs_since_1900;
      
      // Convert four bytes starting at location 40 to a long integer
      secs_since_1900 = (unsigned long)packet_buffer[40] << 24;
      secs_since_1900 |= (unsigned long)packet_buffer[41] << 16;
      secs_since_1900 |= (unsigned long)packet_buffer[42] << 8;
      secs_since_1900 |= (unsigned long)packet_buffer[43];
      
      time_t utc_time = secs_since_1900 - 2208988800UL;  // Convert to Unix epoch
      
      // Load timezone profile if timezone_id provided (T009 wiring)
      if (timezone_id && strlen(timezone_id) > 0) {
        set_ntp_timezone_profile(timezone_id);
      }
      
      // Calculate DST offset using the UTC time from the NTP packet, NOT now().
      // Using now() here would return stale/epoch-0 time before setTime() is called,
      // causing DST to be evaluated for year 1970 instead of the actual year.
      bool is_dst = false;
      time_t next_transition = 0;
      int offset_seconds = get_timezone_offset_and_transition(
        get_ntp_timezone_profile(), utc_time, &is_dst, &next_transition);
      
      // Diagnostic: printed once per NTP sync
      struct tm *tmp = gmtime(&utc_time);
      if (tmp) {
        Serial.printf("NTP UTC: %04d-%02d-%02d %02d:%02d:%02d\n",
          tmp->tm_year + 1900, tmp->tm_mon + 1, tmp->tm_mday,
          tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
      }
      Serial.printf("Offset: %d seconds (%s)\n", offset_seconds, is_dst ? "DST" : "STD");
      
      return utc_time + offset_seconds;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0;  // return 0 if unable to get the time
}

#endif