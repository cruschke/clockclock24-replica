#ifndef NTP_H
#define NTP_H

#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <TimeLib.h>

// NTP Servers:
static const char ntp_server_name[] = "pool.ntp.org";

int _time_zone = 1; // Central European Time

WiFiUDP Udp;
unsigned int local_port = 8888; // local port to listen for UDP packets

const int NTP_PACKET_SIZE = 48;     // NTP time is in the first 48 bytes of message
byte packet_buffer[NTP_PACKET_SIZE]; // buffer to hold incoming & outgoing packets

// --- Non-blocking NTP state machine ---
// Previously, get_NTP_time() called WiFi.hostByName() (blocking DNS, no timeout)
// and then spin-waited up to 1500ms for a UDP reply, both inside loop().
// This froze handle_webclient() for seconds every 30 minutes.
// The new approach: send the UDP packet and return immediately; check for a
// reply on subsequent loop() calls via tick_NTP().

#define NTP_SYNC_INTERVAL_MS  (30UL * 60UL * 1000UL) // 30 minutes
#define NTP_RESPONSE_TIMEOUT_MS 2000                  // give up after 2s

static IPAddress _ntp_server_ip;
static uint8_t   _ntp_state        = 0;  // 0 = IDLE, 1 = WAITING_RESPONSE
static uint32_t  _ntp_send_time    = 0;
static uint32_t  _ntp_last_sync_ms = 0;

void begin_NTP();
void tick_NTP();          // call from loop() instead of setSyncProvider
void send_NTP_packet(IPAddress &address);
void set_ntp_timezone(int value);
int  get_ntp_timezone();
void request_ntp_sync(); // force an immediate re-sync (e.g. on timezone change)

// --- Legacy stub kept so existing callers compile ---
// No longer registered with setSyncProvider.
inline time_t get_NTP_time() { return 0; }

void begin_NTP()
{
  Udp.begin(local_port);
  // Resolve NTP IP once at startup (blocking is fine inside setup()).
  // On every subsequent sync tick_NTP() re-resolves — pool.ntp.org is a
  // round-robin pool and expects re-resolution per sync to spread load.
  Serial.println("Resolving NTP server...");
  if (WiFi.hostByName(ntp_server_name, _ntp_server_ip))
  {
    Serial.print("NTP server IP: ");
    Serial.println(_ntp_server_ip);
  }
  else
  {
    Serial.println("NTP DNS lookup failed at startup.");
  }
}

void tick_NTP()
{
  uint32_t now_ms = millis();

  if (_ntp_state == 0) // IDLE
  {
    bool sync_due = (_ntp_last_sync_ms == 0) ||
                    (now_ms - _ntp_last_sync_ms >= NTP_SYNC_INTERVAL_MS);
    if (!sync_due) return;

    // Re-resolve DNS on every sync. pool.ntp.org is a round-robin pool;
    // re-resolving distributes each request across different servers.
    // WiFi.hostByName() on a healthy LAN is typically < 100ms — acceptable.
    // (The old 1500ms spin-wait for the UDP reply was the real blocker.)
    if (!WiFi.hostByName(ntp_server_name, _ntp_server_ip))
    {
      Serial.println("NTP DNS lookup failed, skipping sync.");
      _ntp_last_sync_ms = now_ms; // back off for a full interval
      return;
    }

    // Discard any stale packets
    while (Udp.parsePacket() > 0);

    send_NTP_packet(_ntp_server_ip);
    _ntp_send_time = now_ms;
    _ntp_state = 1; // WAITING_RESPONSE
    Serial.println("NTP request sent (non-blocking).");
  }
  else // WAITING_RESPONSE
  {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE)
    {
      Udp.read(packet_buffer, NTP_PACKET_SIZE);
      unsigned long secs_since_1900;
      secs_since_1900  = (unsigned long)packet_buffer[40] << 24;
      secs_since_1900 |= (unsigned long)packet_buffer[41] << 16;
      secs_since_1900 |= (unsigned long)packet_buffer[42] << 8;
      secs_since_1900 |= (unsigned long)packet_buffer[43];
      time_t unix_time = secs_since_1900 - 2208988800UL + _time_zone * SECS_PER_HOUR;
      setTime(unix_time);
      _ntp_last_sync_ms = now_ms;
      _ntp_state = 0; // back to IDLE
      Serial.println("NTP sync successful.");
    }
    else if (now_ms - _ntp_send_time > NTP_RESPONSE_TIMEOUT_MS)
    {
      Serial.println("NTP response timeout.");
      _ntp_last_sync_ms = now_ms;
      _ntp_state = 0; // back to IDLE — will re-resolve DNS on next attempt
    }
  }
}

void request_ntp_sync()
{
  // Force next tick_NTP() call to send a new request
  _ntp_last_sync_ms = 0;
  _ntp_state = 0;
}

// send an NTP request to the time server at the given address
void send_NTP_packet(IPAddress &address)
{
  memset(packet_buffer, 0, NTP_PACKET_SIZE);
  packet_buffer[0]  = 0b11100011; // LI, Version, Mode
  packet_buffer[1]  = 0;          // Stratum
  packet_buffer[2]  = 6;          // Polling Interval
  packet_buffer[3]  = 0xEC;       // Peer Clock Precision
  packet_buffer[12] = 49;
  packet_buffer[13] = 0x4E;
  packet_buffer[14] = 49;
  packet_buffer[15] = 52;
  Udp.beginPacket(address, 123);
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

#endif