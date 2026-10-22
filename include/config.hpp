#pragma once
#include "pros/adi.hpp"
#include "pros/motors.hpp"
# include "pros/imu.hpp"
#include "pros/rotation.hpp"
#include "pros/motor_group.hpp"
#include "pros/imu.hpp"
#include "pros/adi.hpp"



extern pros::Motor lift;
extern pros::Motor lift_right;
extern pros::MotorGroup lift_mg;
extern pros::MotorGroup left_mg;
extern pros::MotorGroup right_mg;
extern pros::Controller master;
extern lemlib::Drivetrain drivetrain;
extern pros::Imu imu;
extern pros::Rotation horizontal_encoder;

