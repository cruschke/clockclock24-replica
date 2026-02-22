#include "clock_manager.h"

int _speed = 200;
int _acceleration = 100;
int _direction = MIN_DISTANCE;

// Changes when the clock state changes.
// IMPORTANT: must never be 0 — slaves initialize change_counter to 0,
// so if master sends 0, slaves ignore all future commands permanently.
uint32_t _counter = 1;

static void increment_counter()
{
  _counter++;
  if (_counter == 0) _counter = 1; // skip 0 on overflow
}

const t_digit _digits[10] = {digit_0, digit_1, digit_2, digit_3, digit_4, digit_5, digit_6, digit_7, digit_8, digit_9};
// Last sended clock state
half_digit _last_state[8] = {0};

int get_speed()
{
  return _speed;
}

void set_speed(int value)
{
  _speed = value;
}

int get_acceleration()
{
  return _acceleration;
}

void set_acceleration(int value)
{
  _acceleration = value;
}

int get_direction()
{
  return _direction;
}

void set_direction(int value)
{
  _direction = value;
}


void send_half_digit(int index, t_half_digit half_digit)
{
  uint8_t address = index + 1;
  Wire.beginTransmission(address);
  I2C_writeAnything(half_digit);
  uint8_t error = Wire.endTransmission();
  if (error != 0)
  {
    const char* roles[] = {
      "Hours Tens - Left", "Hours Tens - Right",
      "Hours Units - Left", "Hours Units - Right",
      "Minutes Tens - Left", "Minutes Tens - Right",
      "Minutes Units - Left", "Minutes Units - Right"
    };
    const char* role = (index >= 0 && index <= 7) ? roles[index] : "Unknown position";
    
    const char* err_str = "Unknown error";
    switch (error) {
      case 1: err_str = "Data too long"; break;
      case 2: err_str = "Board missing/unresponsive (NACK on address)"; break;
      case 3: err_str = "Data rejection (NACK on data)"; break;
      case 4: err_str = "Other hardware error"; break;
      case 5: err_str = "Timeout (I2C bus locked up)"; break;
    }
    
    Serial.printf("\n[!] I2C ERROR -> Board %d (%s) failed: %s (code %d)\n", address, role, err_str, error);
  }
}

// 0 <= index < 4
void send_digit(int index, t_digit digit)
{
    t_half_digit l = get_full_half_digit(digit.halfs[0]);
    t_half_digit r = get_full_half_digit(digit.halfs[1]);
    send_half_digit(index*2, l);
    send_half_digit(index*2 + 1, r);
    _last_state[index*2] = l;
    _last_state[index*2 + 1] = r;
}

void send_clock(t_full_clock full_clock)
{
  for (int i = 0; i < 4; i++)
    send_digit(i, full_clock.digit[i]);
}

t_half_digit get_full_half_digit(t_half_digitl lite_digit)
{
    t_half_digit tmp = {0};
    for (int i = 0; i < 3; i++)
    {
        tmp.change_counter[i] = _counter;
        tmp.clocks[i].angle_h = lite_digit.clocks[i].angle_h;
        tmp.clocks[i].angle_m = lite_digit.clocks[i].angle_m;
        tmp.clocks[i].speed_h = _speed;
        tmp.clocks[i].speed_m = _speed;
        tmp.clocks[i].accel_h = _acceleration;
        tmp.clocks[i].accel_m = _acceleration;
        tmp.clocks[i].mode_h = _direction;
        tmp.clocks[i].mode_m = _direction;
        tmp.clocks[i].adjust_h = 0;
        tmp.clocks[i].adjust_m = 0;
    }
  return tmp;
}

void set_clock(t_full_clock clock_state)
{
  send_clock(clock_state);
  increment_counter();
}

// 0 <= index < 4
void set_digit(int index, t_digit digit)
{
  send_digit(index, digit);
  increment_counter();
}

// 0 <= index < 8
void set_half_digit(int index, t_half_digitl half)
{
    t_half_digit hd = get_full_half_digit(half);
    send_half_digit(index, hd);
    _last_state[index] = hd;
    increment_counter();
}

void set_clock_time(int h, int m)
{
  if(h < 0 || h > 99 || m < 0 || m > 99 )
    return;
  set_clock(get_clock_state_from_time(h, m));
}

t_full_clock get_clock_state_from_time(int h, int m)
{
  int d0 = h / 10;
  int d1 = h - d0 * 10;  
  int d2 = m / 10;
  int d3 = m - d2 * 10;
  Serial.printf("Set time: %d:%d\n", h, m);
  t_full_clock clock_state = {_digits[d0], _digits[d1], _digits[d2], _digits[d3]};
  return clock_state;
}

void adjust_hands(int clock_index, int h_amount, int m_amount)
{
  t_half_digit tmp = _last_state[clock_index/3];
  tmp.clocks[clock_index % 3].adjust_h = ((signed char) h_amount);
  tmp.clocks[clock_index % 3].adjust_m = ((signed char) m_amount);
  tmp.clocks[clock_index % 3].mode_h = ADJUST_HAND;
  tmp.clocks[clock_index % 3].mode_m = ADJUST_HAND;
  tmp.clocks[clock_index % 3].speed_h = 5000;
  tmp.clocks[clock_index % 3].speed_m = 5000;
  tmp.clocks[clock_index % 3].accel_h = 5000;
  tmp.clocks[clock_index % 3].accel_m = 5000;
  tmp.change_counter[clock_index % 3] = _counter;
  send_half_digit(clock_index/3, tmp);
  increment_counter();
}