#include "main.h"
#include "lemlib/api.hpp"

pros::Controller master(pros::E_CONTROLLER_MASTER);
pros::MotorGroup left_mg({-5, -7}, pros::MotorGears::blue); // blue = 600 rpm
pros::MotorGroup right_mg({9, 3}, pros::MotorGears::blue);

pros::Motor lift(-1, pros::MotorGears::red);
pros::Motor lift_right(2, pros::MotorGears::red);
pros::MotorGroup lift_mg({2, -1}, pros::MotorGears::red);

pros::Motor intake(4, pros::MotorGears::blue);   // use -4 if it spins the wrong way
pros::Motor banshee(6, pros::MotorGears::green); // use -4 if it spins the wrong way

pros::adi::DigitalOut clamp('B');
pros::adi::DigitalOut claw('D');
// drivetrain settings
lemlib::Drivetrain drivetrain(&left_mg,                   // left motor group
                              &right_mg,                  // right motor group
                              11,                         // 10 inch track width
                              lemlib::Omniwheel::NEW_325, // using new 4" omnis
                                                          // TODO: update track width, diameter, and rpm.
                              600,                        // drivetrain rpm is 600
                              2                           // horizontal drift is 2 (for now)
);

// imu
pros::Imu imu(17);

// rotation sensor
pros::Rotation rotation_sensor(10); // rotation sensor on port 10, not reversee

// vertical tracking wheel
lemlib::TrackingWheel vertical_tracking_wheel(&rotation_sensor, lemlib::Omniwheel::NEW_2, 1);

// odometry settings
lemlib::OdomSensors sensors(&vertical_tracking_wheel, // vertical tracking wheel 1
                            nullptr,                  // vertical tracking wheel 2
                            nullptr,                  // horizontal tracking wheel 1
                            nullptr,                  // horizontal tracking wheel 2
                            &imu                      // inertial sensor
);

// lateral PID controller
lemlib::ControllerSettings lateral_controller(10,   // proportional gain (kP)
                                              0,    // integral gain (kI)
                                              23.5, // derivative gain (kD)
                                              0,    // anti windup
                                              0,    // small error range, in inches
                                              0,    // small error range timeout, in milliseconds
                                              0,    // large error range, in inches
                                              0,    // large error range timeout, in milliseconds
                                              2000  // maximum acceleration (slew)
);

// angular PID controller
lemlib::ControllerSettings angular_controller(0, // proportional gain (kP)
                                              0, // integral gain (kI)
                                              0, // derivative gain (kD)
                                              0, // anti windup
                                              0, // small error range, in degrees
                                              0, // small error range timeout, in milliseconds
                                              0, // large error range, in degrees
                                              0, // large error range timeout, in milliseconds
                                              0  // maximum acceleration (slew)
);

// drive curves
lemlib::ExpoDriveCurve throttle_curve(3,    // deadband
                                      10,   // min output to move
                                      1.019 // curve gain
);
lemlib::ExpoDriveCurve steer_curve(3, 10, 1.019);

// create the chassis
lemlib::Chassis chassis(drivetrain,         // drivetrain settings
                        lateral_controller, // lateral PID settings
                        angular_controller, // angular PID settings
                        sensors,            // odometry sensors
                        &throttle_curve,    // NEW
                        &steer_curve        // NEW
);

void autonomous()
{
    chassis.setPose(0, 0, 0);
    chassis.moveToPoint(0, 30, 9999);
}

// initialize function. Runs on program startup
void initialize()
{
    pros::lcd::initialize(); // initialize brain screen

    clamp.set_value(false);
    claw.set_value(false);

    // TODO: RE-ENABLE CHASSIS CALIBRATION FOR PID
    // chassis.calibrate(); // calibrate sensors
    // print position to brain screen
    // Initialize the lift motor
    lift.set_encoder_units(pros::MotorEncoderUnits::degrees);
    lift.set_brake_mode(pros::MotorBrake::hold);
    lift.tare_position();
    lift_right.set_brake_mode(pros::MotorBrake::hold);
    lift_right.tare_position();
    lift_mg.set_brake_mode(pros::MotorBrake::hold);
    lift_mg.tare_position();

    intake.set_brake_mode(pros::MotorBrake::brake);

    left_mg.set_brake_mode(pros::MotorBrake::coast);
    right_mg.set_brake_mode(pros::MotorBrake::coast);

    banshee.set_encoder_units(pros::MotorEncoderUnits::degrees);
    banshee.set_brake_mode(pros::MotorBrake::hold);
    banshee.tare_position(); // tare the position of the banshee motor

    pros::Task screen_task([&]()
                           {
        while (true) {
            // print robot location to the brain screen
            pros::lcd::print(0, "X: %f", chassis.getPose().x); // x
            pros::lcd::print(1, "Y: %f", chassis.getPose().y); // y
            pros::lcd::print(2, "Theta: %f", chassis.getPose().theta); // heading
            pros::lcd::print(3, "Motors Heat: %f", left_mg.get_temperature()); // left motors heat
            pros::lcd::print(4, "Motors Heat: %f", right_mg.get_temperature()); //right motors heat
        
            pros::delay(20);    // delay to save resources
        } });
}
// <3 Jinwoong