#include "main.h"
#include "lemlib/api.hpp"

pros::Controller master(pros::E_CONTROLLER_MASTER);
pros::MotorGroup left_mg({-5, -7}, pros::MotorGears::blue); // blue = 600 rpm
pros::MotorGroup right_mg({8, 3}, pros::MotorGears::blue);

pros::Motor lift(-19, pros::MotorGears::red);
pros::Motor lift_right(11, pros::MotorGears::red);
pros::MotorGroup lift_mg({-19, 11}, pros::MotorGears::red);

lemlib::PID headingPID(2, 0, 10); // for drive-straight
lemlib::PID liftPID(0.5, 0, 0.3); // only if you use lift PID

pros::adi::DigitalOut clamp('B');

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
// horizontal tracking wheel encoder
pros::Rotation horizontal_encoder(20);
// vertical tracking wheel encoder
pros::adi::Encoder vertical_encoder('C', 'D', true);
// horizontal tracking wheel
lemlib::TrackingWheel horizontal_tracking_wheel(&horizontal_encoder, lemlib::Omniwheel::NEW_275, -5.75);
// vertical tracking wheel
lemlib::TrackingWheel vertical_tracking_wheel(&vertical_encoder, lemlib::Omniwheel::NEW_275, -2.5);

// odometry settings
lemlib::OdomSensors sensors(nullptr, // vertical tracking wheel 1
                            nullptr, // vertical tracking wheel 2
                            nullptr, // horizontal tracking wheel 1
                            nullptr, // horizontal tracking wheel 2
                            &imu     // inertial sensor
);

// lateral PID controller
lemlib::ControllerSettings lateral_controller(10,  // proportional gain (kP)
                                              0,   // integral gain (kI)
                                              3,   // derivative gain (kD)
                                              3,   // anti windup
                                              1,   // small error range, in inches
                                              100, // small error range timeout, in milliseconds
                                              3,   // large error range, in inches
                                              500, // large error range timeout, in milliseconds
                                              20   // maximum acceleration (slew)
);

// angular PID controller
lemlib::ControllerSettings angular_controller(2,   // proportional gain (kP)
                                              0,   // integral gain (kI)
                                              10,  // derivative gain (kD)
                                              3,   // anti windup
                                              1,   // small error range, in degrees
                                              100, // small error range timeout, in milliseconds
                                              3,   // large error range, in degrees
                                              500, // large error range timeout, in milliseconds
                                              0    // maximum acceleration (slew)
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

// initialize function. Runs on program startup
void initialize()
{
    pros::lcd::initialize(); // initialize brain screen

    clamp.set_value(false);

    // TODO: RE-ENABLE CHASSIS CALIBRATION WHEN WE PUT SENSOR
    chassis.calibrate(); // calibrate sensors
    // print position to brain screen
    // Initialize the lift motor
    lift.set_encoder_units(pros::MotorEncoderUnits::degrees);
    lift.set_brake_mode(pros::MotorBrake::hold);
    lift.tare_position();
    lift_right.set_brake_mode(pros::MotorBrake::hold);
    lift_right.tare_position();
    lift_mg.set_brake_mode(pros::MotorBrake::hold);
    lift_mg.tare_position();

    pros::Task screen_task([&]()
                           {
        while (true) {
            // print robot location to the brain screen
            pros::lcd::print(0, "X: %f", chassis.getPose().x); // x
            pros::lcd::print(1, "Y: %f", chassis.getPose().y); // y
            pros::lcd::print(2, "Theta: %f", chassis.getPose().theta); // heading
            // delay to save resources
            pros::delay(20);
        } });
}
