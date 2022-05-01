#include "main.h"
pros::Motor tilter(1, MOTOR_GEARSET_36, false, MOTOR_ENCODER_DEGREES);
pros::Motor conveyor(1, MOTOR_GEARSET_6, false MOTOR_ENCODER_DEGREES);
pros::Motor lift(1, MOTOR_GEARSET_36, false MOTOR_ENCODER_DEGREES);
pros::Motor clamp(1, MOTOR_GEARSET_6, false MOTOR_ENCODER_DEGREES);
pros::ADIDigitalIn tilter_button(DIGITAL_SENSOR_PORT);
const int tilter_in_pos;    // in postion, when you do not have a mogo in
const int tilter_out_pos;   // mogo postion, when you need to drop rings on the a-mogo (added to in postion)
const int tilter_down_pos;  // out postion, for intaking and dropping mogos (added to in postion)
bool is_tilter_up;          // is the tilter up?
bool is_tilter_down;        // is the tilter down?
int tilter_lock = 0;        // do you want to lock the tilter in a certain state?
int controller_tilter_timer = 0;
void zero_tilter() { tilter.tare_position(); }                      // reset the tilter's position
int get_tilter_position() { return tilter.get_position(); }         // get the tilter's current postion
int get_tilter_velocity() { return tilter.get_actual_velocity(); }  // get the tilter's current velocity
void set_tilter(int input) { tilter = input; }
bool timeout(int target, int vel, int current) {
  static int vel_timeout;
  static int small_timeout;
  if (abs(target - current) < 20) {
    small_timeout += DELAY_TIME;
    if (small_timeout > 50) {
      small_timeout = 0;
      vel_timeout = 0;
      return true;
    }
  } else {
    small_timeout = 0;
  }
  if (vel == 0) {
    vel_timeout += DELAY_TIME;
    if (vel_timeout > 250) {
      vel_timeout = 0;
      small_timeout = 0;
      return true;
    }
  } else {
    vel_timeout = 0;
  }
  return false;
}
void set_tilter_position(int target_pos, int speed, bool hold) {
  bool is_there = false;
  // run the built in PID to brinf the tilter to a sepcified target point
  tilter.move_absolute(target_pos, speed);
  is_there = timeout(target_pos, speed, !is_there);
  // if using during autonmous
  if (hold) {
    // loop the code if the target has not been reached yet
    pros::delay(DELAY_TIME);
    set_tilter_position(target_pos, speed, !is_there);
  }
}
void home_tilter() {
  // once autonomous runs, bring the tilter out  and back in
  set_tilter_position(get_tilter_position + 20, 100, true);
  // sense if the touch sensor is depressed
  while (!tilter_button) {
    tilter.move(-100);
  }
  // once tilter is pressed, stop the motor
  tilter.move(0);
  // set the in postion as the current postion +5 (for innacuracies and so it doesnt ram into itself all the time lol)
  tilter_in_pos = get_tilter_position() + 5;
}
void set_tilter_in(bool hold) {
  // run built in PID to bring tilter into in position
  set_tilter_position(tilter_in_pos, 100, hold);
  // set states so the mogo will be in the last position in driver it was in during auto
  if (hold) {
    is_tilter_up = true;
    is_tilter_down = false;
  }
}
void set_tilter_down(bool hold) {
  // run built in PID to bring tilter into down position
  set_tilter_position(tilter_down_pos, 100, hold);
  // set states so the mogo will be in the last position in driver it was in during auto
  if (hold) {
    is_tilter_down = true;
  }
}
void set_tilter_out(bool hold) {
  // run built in PID to bring tilter into out position
  set_tilter_position(tilter_out_pos, 100, hold);
  // set states so the mogo will be in the last position in driver it was in during auto
  if (hold) {
    is_tilter_down = false;
    is_tilter_up = false;
  }
}