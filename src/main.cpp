#include "main.h"
#include "api.h"
#include "lemlib/api.hpp"
#include "config.hpp"
#include <cmath>

/* !!!!
 */

constexpr double LIFT_MIN = 5;
constexpr double LIFT_MAX = 1500;

/*working on displaying image on vex v5

----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void display_img_from_c_array() {

	LV_IMAGE_DECLARE(shibuya_sky_bg);

	lv_obj_t* img = lv_image_create(lv_screen_active());

	lv_image_set_src(img, &shibuya_sky_bg);
}

void display_image_from_file(const void * src) {

}

}

*/

/**
 *
 * A callback function for LLEMU's center button.
 *
 * When this callback is fired, it will toggle line 2 of the LCD text between
 * "I was pressed!" and nothing.
 */
void on_center_button()
{
	static bool pressed = false;
	pressed = !pressed;
	if (pressed)
	{
		pros::lcd::set_text(2, "I was pressed!");
	}
	else
	{
		pros::lcd::clear_line(2);
	}
}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competiti  on Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {}

/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */

/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 *
 */

// Moves `current` toward `target`, but by at most one step per call.
// Uses a gentler step when speeding up and a different one when slowing down or reversing.
double slew(double target, double current, double accel_step, double decel_step)
{
	bool speeding_up = (target * current >= 0) && (std::abs(target) > std::abs(current));
	double max_step = speeding_up ? accel_step : decel_step;

	double delta = target - current;
	if (delta > max_step)
		delta = max_step;
	else if (delta < -max_step)
		delta = -max_step;

	return current + delta;
}

// Tuning values: max change per 20 ms loop, on the -127..127 scale
constexpr double THROTTLE_ACCEL = 5; // lower = slower, smoother start
constexpr double THROTTLE_DECEL = 7; // lower = softer braking
constexpr double TURN_ACCEL = 10;
constexpr double TURN_DECEL = 14;

void opcontrol()
{
	static bool clamp_state = false;
	static bool claw_state = false;

	static double throttle_out = 0;
	static double turn_out = 0;

	while (true)
	{

		pros::lcd::print(0, "%d %d %d", (pros::lcd::read_buttons() & LCD_BTN_LEFT) >> 2,
						 (pros::lcd::read_buttons() & LCD_BTN_CENTER) >> 1,
						 (pros::lcd::read_buttons() & LCD_BTN_RIGHT) >> 0); // Prints status of the emulated screen LCDs

		// --- Drive control (arcade) with slew limiting ---
		int throttle_in = master.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
		int turn_in = master.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X);

		throttle_out = slew(throttle_in, throttle_out, THROTTLE_ACCEL, THROTTLE_DECEL);
		turn_out = slew(turn_in, turn_out, TURN_ACCEL, TURN_DECEL);

		pros::lcd::print(5, "in:%d out:%d conn:%d", throttle_in, (int)throttle_out, master.is_connected());

		chassis.arcade(throttle_out, turn_out);

		double pos = lift.get_position();
		bool up_pressed = master.get_digital(pros::E_CONTROLLER_DIGITAL_L1);
		bool down_pressed = master.get_digital(pros::E_CONTROLLER_DIGITAL_R1);

		if (up_pressed && pos < LIFT_MAX)
		{
			lift_mg.move(100);
		}
		else if (down_pressed && pos > LIFT_MIN)
		{
			lift_mg.move(-100);
		}
		else
		{
			lift_mg.move(0);
			lift_mg.brake();
		}

		if (master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_X))
		{
			claw_state = !claw_state;
			claw.set_value(claw_state);
		}

		if (master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_A))
		{
			clamp_state = !clamp_state;
			clamp.set_value(clamp_state);
		}
		pros::delay(20); // Delay to prevent wasted resources
	}
}
