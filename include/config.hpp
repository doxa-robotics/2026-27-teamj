#pragma once
#include "main.h"
#include "lemlib/api.hpp"

extern pros::Controller master;
extern pros::Motor lift;
extern pros::Motor lift_right;
extern pros::MotorGroup lift_mg;
extern pros::MotorGroup left_mg;
extern pros::MotorGroup right_mg;
extern pros::Controller master;
extern lemlib::Drivetrain drivetrain;
extern pros::Imu imu;
extern pros::Rotation horizontal_encoder;
extern lemlib::Chassis chassis;
extern lemlib::PID headingPID;
extern lemlib::PID liftPID;
extern pros::Motor intake;
extern pros::Motor banshee;
extern pros::Motor outtake;
