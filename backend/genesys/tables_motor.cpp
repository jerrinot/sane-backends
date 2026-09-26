/*  sane - Scanner Access Now Easy.

    Copyright (C) 2019 Povilas Kanapickas <povilas@radix.lt>

    This file is part of the SANE package.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#define DEBUG_DECLARE_ONLY

#include "low.h"

namespace genesys {

StaticInit<std::vector<Genesys_Motor>> s_motors;

void genesys_init_motor_tables()
{
    s_motors.init();

    MotorProfile profile;

    Genesys_Motor motor;
    motor.id = MotorId::UMAX;
    motor.base_ydpi = 2400;
    motor.profiles.push_back({MotorSlope::create_from_steps(11000, 3000, 128), StepType::FULL, 0});
    motor.profiles.push_back({MotorSlope::create_from_steps(11000, 3000, 128), StepType::HALF, 0});
    s_motors->push_back(std::move(motor));


    motor = Genesys_Motor();
    motor.id = MotorId::MD_5345; // MD5345/6228/6471
    motor.base_ydpi = 2400;
    motor.profiles.push_back({MotorSlope::create_from_steps(2000, 1375, 128), StepType::FULL, 0});
    motor.profiles.push_back({MotorSlope::create_from_steps(2000, 1375, 128), StepType::HALF, 0});
    s_motors->push_back(std::move(motor));


    motor = Genesys_Motor();
    motor.id = MotorId::ST24;
    motor.base_ydpi = 2400;
    motor.profiles.push_back({MotorSlope::create_from_steps(2289, 2100, 128), StepType::FULL, 0});
    motor.profiles.push_back({MotorSlope::create_from_steps(2289, 2100, 128), StepType::HALF, 0});
    s_motors->push_back(std::move(motor));


    motor = Genesys_Motor();
    motor.id = MotorId::HP3670;
    motor.base_ydpi = 1200;
    motor.profiles.push_back({MotorSlope::create_from_steps(11000, 3000, 128), StepType::FULL, 0});
    motor.profiles.push_back({MotorSlope::create_from_steps(11000, 3000, 128), StepType::HALF, 0});
    s_motors->push_back(std::move(motor));


    motor = Genesys_Motor();
    motor.id = MotorId::HP2400;
    motor.base_ydpi = 1200;
    motor.profiles.push_back({MotorSlope::create_from_steps(11000, 3000, 128), StepType::FULL, 0});
    motor.profiles.push_back({MotorSlope::create_from_steps(11000, 3000, 128), StepType::HALF, 0});
    s_motors->push_back(std::move(motor));


    motor = Genesys_Motor();
    motor.id = MotorId::HP2300;
    motor.base_ydpi = 1200;
    motor.profiles.push_back({MotorSlope::create_from_steps(3200, 1200, 128), StepType::FULL, 0});
    motor.profiles.push_back({MotorSlope::create_from_steps(3200, 1200, 128), StepType::HALF, 0});
    s_motors->push_back(std::move(motor));


    motor = Genesys_Motor();
    motor.id = MotorId::CANON_LIDE_35;
    motor.base_ydpi = 1200;

    profile = MotorProfile{MotorSlope::create_from_steps(3500, 1300, 150), StepType::HALF, 0};
    profile.resolutions = { 75, 150, 200, 300, 600 };
    motor.profiles.push_back(profile);

    profile = MotorProfile{MotorSlope::create_from_steps(3500, 1300, 150), StepType::QUARTER, 0};
    profile.resolutions = { 1200, 2400 };
    motor.profiles.push_back(profile);

    profile = MotorProfile{MotorSlope::create_from_steps(3500, 1400, 150), StepType::FULL, 0};
    profile.resolutions = { 75, 150, 200, 300 };
    motor.fast_profiles.push_back(profile);

    profile = MotorProfile{MotorSlope::create_from_steps(6000, 3000, 100), StepType::FULL, 0};
    profile.resolutions = { 600, 1200, 2400 };
    motor.fast_profiles.push_back(profile);

    s_motors->push_back(std::move(motor));


    motor = Genesys_Motor();
    motor.id = MotorId::CANON_LIDE_60;
    motor.base_ydpi = 1200;

    profile = MotorProfile{MotorSlope::create_from_steps(3500, 1400, 150), StepType::HALF, 0};
    motor.profiles.push_back(profile);

    profile = MotorProfile{MotorSlope::create_from_steps(3500, 1400, 150), StepType::FULL, 0};
    profile.resolutions = { 75, 150, 300 };
    motor.fast_profiles.push_back(profile);

    profile = MotorProfile{MotorSlope::create_from_steps(6000, 3000, 100), StepType::FULL, 0};
    profile.resolutions = { 600, 1200, 2400 };
    motor.fast_profiles.push_back(profile);

    s_motors->push_back(std::move(motor));


    motor = Genesys_Motor();
    motor.id = MotorId::CANON_LIDE_90;
    motor.base_ydpi = 1200;
    profile = {MotorSlope::create_from_steps(8000, 3000, 200), StepType::FULL, 0};
    profile.resolutions = { 150, 300 };
    motor.profiles.push_back(profile);

    profile = {MotorSlope::create_from_steps(7000, 3000, 200), StepType::HALF, 0};
    profile.resolutions = { 600, 1200 };
    motor.profiles.push_back(profile);

    profile = {MotorSlope::create_from_steps(7000, 3000, 200), StepType::QUARTER, 0};
    profile.resolutions = { 2400 };
    motor.profiles.push_back(profile);
    s_motors->push_back(std::move(motor));


    motor = Genesys_Motor();
    motor.id = MotorId::XP200;
    motor.base_ydpi = 600;
    motor.profiles.push_back({MotorSlope::create_from_steps(3500, 1300, 60), StepType::FULL, 0});
    motor.profiles.push_back({MotorSlope::create_from_steps(3500, 1300, 60), StepType::HALF, 0});
    motor.fast_profiles.push_back({MotorSlope::create_from_steps(3500, 1300, 60), StepType::FULL, 0});
    s_motors->push_back(std::move(motor));


    motor = Genesys_Motor();
    motor.id = MotorId::XP300;
    motor.base_ydpi = 300;
    // works best with GPIO10, GPIO14 off
    profile = MotorProfile{MotorSlope::create_from_steps(3700, 3700, 2), StepType::FULL, 0};
    profile.resolutions = {}; // used during fast moves
    motor.profiles.push_back(profile);

    // FIXME: this motor profile is useless
    profile = MotorProfile{MotorSlope::create_from_steps(11000, 11000, 2), StepType::HALF, 0};
    profile.resolutions = {75, 150, 300, 600};
    motor.profiles.push_back(profile);

    profile = MotorProfile{MotorSlope::create_from_steps(3700, 3700, 2), StepType::FULL, 0};
    motor.fast_profiles.push_back(profile);
    s_motors->push_back(std::move(motor));


    motor = Genesys_Motor();
    motor.id = MotorId::DP665;
    motor.base_ydpi = 750;

    profile = MotorProfile{MotorSlope::create_from_steps(3000, 2500, 10), StepType::FULL, 0};
    profile.resolutions = {75, 150};
    motor.profiles.push_back(profile);

    // FIXME: this motor profile is useless
    profile = MotorProfile{MotorSlope::create_from_steps(11000, 11000, 2), StepType::HALF, 0};
    profile.resolutions = {300, 600, 1200};
    motor.profiles.push_back(profile);

    profile = MotorProfile{MotorSlope::create_from_steps(3000, 2500, 10), StepType::FULL, 0};
    motor.fast_profiles.push_back(profile);
    s_motors->push_back(std::move(motor));


    motor = Genesys_Motor();
    motor.id = MotorId::ROADWARRIOR;
    motor.base_ydpi = 750;

    profile = MotorProfile{MotorSlope::create_from_steps(3000, 2600, 10), StepType::FULL, 0};
    profile.resolutions = {75, 150};
    motor.profiles.push_back(profile);

    // FIXME: this motor profile is useless
    profile = MotorProfile{MotorSlope::create_from_steps(11000, 11000, 2), StepType::HALF, 0};
    profile.resolutions = {300, 600, 1200};
    motor.profiles.push_back(profile);

    profile = MotorProfile{MotorSlope::create_from_steps(3000, 2600, 10), StepType::FULL, 0};
    motor.fast_profiles.push_back(profile);
    s_motors->push_back(std::move(motor));


    motor = Genesys_Motor();
    motor.id = MotorId::DSMOBILE_600;
    motor.base_ydpi = 750;

    profile = MotorProfile{MotorSlope::create_from_steps(6666, 3700, 8), StepType::FULL, 0};
    profile.resolutions = {75, 150};
    motor.profiles.push_back(profile);

    profile = MotorProfile{MotorSlope::create_from_steps(6666, 3700, 8), StepType::HALF, 0};
    profile.resolutions = {300, 600, 1200};
    motor.profiles.push_back(profile);

    profile = MotorProfile{MotorSlope::create_from_steps(6666, 3700, 8), StepType::FULL, 0};
    motor.fast_profiles.push_back(profile);
    s_motors->push_back(std::move(motor));


    motor = Genesys_Motor();
    motor.id = MotorId::CANON_LIDE_100;
    motor.base_ydpi = 1200;
    motor.profiles.push_back({MotorSlope::create_from_steps(46876, 864, 255),
                              StepType::HALF, 1432});
    motor.profiles.push_back({MotorSlope::create_from_steps(46876, 864, 279),
                              StepType::QUARTER, 2712});
    motor.profiles.push_back({MotorSlope::create_from_steps(31680, 864, 247),
                              StepType::EIGHTH, 5280});
    s_motors->push_back(std::move(motor));


    motor = Genesys_Motor();
    motor.id = MotorId::CANON_LIDE_200;
    motor.base_ydpi = 1200;
    motor.profiles.push_back({MotorSlope::create_from_steps(46876, 864, 255),
                              StepType::HALF, 1432});
    motor.profiles.push_back({MotorSlope::create_from_steps(46876, 864, 279),
                              StepType::QUARTER, 2712});
    motor.profiles.push_back({MotorSlope::create_from_steps(31680, 864, 247),
                              StepType::EIGHTH, 5280});
    motor.profiles.push_back({MotorSlope::create_from_steps(31680, 864, 247),
                              StepType::EIGHTH, 10416});
    s_motors->push_back(std::move(motor));


    motor = Genesys_Motor();
    motor.id = MotorId::CANON_LIDE_700;
    motor.base_ydpi = 1200;
    motor.profiles.push_back({MotorSlope::create_from_steps(46876, 534, 255),
                              StepType::HALF, 1424});
    motor.profiles.push_back({MotorSlope::create_from_steps(46876, 534, 255),
                              StepType::HALF, 1504});
    motor.profiles.push_back({MotorSlope::create_from_steps(46876, 2022, 127),
                              StepType::HALF, 2696});
    motor.profiles.push_back({MotorSlope::create_from_steps(46876, 534, 255),
                              StepType::HALF, 2848});
    motor.profiles.push_back({MotorSlope::create_from_steps(46876, 15864, 2),
                              StepType::EIGHTH, 10576});
    s_motors->push_back(std::move(motor));


    motor = Genesys_Motor();
    motor.id = MotorId::KVSS080;
    motor.base_ydpi = 1200;
    motor.profiles.push_back({MotorSlope::create_from_steps(44444, 500, 489),
                              StepType::HALF, 8000});
    s_motors->push_back(std::move(motor));


    motor = Genesys_Motor();
    motor.id = MotorId::G4050;
    motor.base_ydpi = 2400;
    motor.profiles.push_back({MotorSlope::create_from_steps(7842, 320, 602),
                              StepType::HALF, 8016});
    motor.profiles.push_back({MotorSlope::create_from_steps(9422, 254, 1004),
                              StepType::HALF, 15624});
    motor.profiles.push_back({MotorSlope::create_from_steps(28032, 2238, 604),
                              StepType::HALF, 56064});
    motor.profiles.push_back({MotorSlope::create_from_steps(42752, 1706, 610),
                              StepType::QUARTER, 42752});
    s_motors->push_back(std::move(motor));


    motor = Genesys_Motor();
    motor.id = MotorId::CANON_4400F;
    motor.base_ydpi = 2400;

    profile = MotorProfile();
    profile.slope = MotorSlope::create_from_steps(28597 * 2, 727 * 2, 200);
    profile.step_type = StepType::QUARTER;
    profile.motor_vref = 1;
    profile.resolutions = { 300, 600 };
    motor.profiles.push_back(std::move(profile));

    profile = MotorProfile();
    profile.slope = MotorSlope::create_from_steps(28597 * 2, 727 * 2, 200);
    profile.step_type = StepType::QUARTER;
    profile.motor_vref = 0;
    profile.resolutions = { 1200, 2400, 4800, 9600 };
    motor.profiles.push_back(std::move(profile));

    profile = MotorProfile();
    profile.slope = MotorSlope::create_from_steps(28597 * 2, 279 * 2, 1000);
    profile.step_type = StepType::QUARTER;
    profile.motor_vref = 0;
    motor.fast_profiles.push_back(std::move(profile));

    s_motors->push_back(std::move(motor));


    motor = Genesys_Motor();
    motor.id = MotorId::CANON_5600F;
    motor.base_ydpi = 2400;

    // FIXME: real limit is 134, but for some reason the motor can't acquire that speed.
    profile = MotorProfile();
    profile.slope = MotorSlope::create_from_steps(2500 * 2, 134 * 2, 1000);
    profile.step_type = StepType::HALF;
    profile.motor_vref = 0;
    profile.resolutions = { 75, 150 };
    motor.profiles.push_back(std::move(profile));

    profile = MotorProfile();
    profile.slope = MotorSlope::create_from_steps(2500 * 2, 200 * 2, 1000);
    profile.step_type = StepType::QUARTER;
    profile.motor_vref = 0;
    profile.resolutions = { 300, 600, 1200, 2400, 4800 };
    motor.profiles.push_back(std::move(profile));

    profile = MotorProfile();
    profile.slope = MotorSlope::create_from_steps(2500 * 2, 200 * 2, 1000);
    profile.step_type = StepType::QUARTER;
    profile.motor_vref = 0;
    motor.fast_profiles.push_back(std::move(profile));

    s_motors->push_back(std::move(motor));


    motor = Genesys_Motor();
    motor.id = MotorId::CANON_8400F;
    motor.base_ydpi = 1600;

    profile = MotorProfile();
    profile.slope = MotorSlope::create_from_steps(20202 * 4, 333 * 4, 100);
    profile.step_type = StepType::QUARTER;
    profile.motor_vref = 0;
    profile.resolutions = VALUE_FILTER_ANY;
    profile.scan_methods = { ScanMethod::FLATBED };
    motor.profiles.push_back(std::move(profile));

    profile = MotorProfile();
    profile.slope = MotorSlope::create_from_steps(65535 * 4, 333 * 4, 100);
    profile.step_type = StepType::QUARTER;
    profile.motor_vref = 2;
    profile.resolutions = VALUE_FILTER_ANY;
    profile.scan_methods = { ScanMethod::TRANSPARENCY, ScanMethod::TRANSPARENCY_INFRARED };
    motor.profiles.push_back(std::move(profile));

    profile = MotorProfile();
    profile.slope = MotorSlope::create_from_steps(65535 * 4, 333 * 4, 200);
    profile.step_type = StepType::QUARTER;
    profile.motor_vref = 2;
    profile.resolutions = VALUE_FILTER_ANY;
    profile.scan_methods = VALUE_FILTER_ANY;
    motor.fast_profiles.push_back(std::move(profile));

    s_motors->push_back(std::move(motor));


    motor = Genesys_Motor();
    motor.id = MotorId::CANON_8600F;
    motor.base_ydpi = 2400;

    profile = MotorProfile();
    profile.slope = MotorSlope::create_from_steps(54612, 1500, 219);
    profile.step_type = StepType::QUARTER;
    profile.motor_vref = 3;
    profile.resolutions = { 300, 600 };
    profile.scan_methods = { ScanMethod::FLATBED };
    motor.profiles.push_back(std::move(profile));

    profile = MotorProfile();
    profile.slope = MotorSlope::create_from_steps(54612, 1500, 219);
    profile.step_type = StepType::QUARTER;
    profile.motor_vref = 2;
    profile.resolutions = { 1200, 2400 };
    profile.scan_methods = { ScanMethod::FLATBED };
    motor.profiles.push_back(std::move(profile));

    profile = MotorProfile();
    profile.slope = MotorSlope::create_from_steps(54612, 1500, 219);
    profile.step_type = StepType::QUARTER;
    profile.motor_vref = 2;
    profile.resolutions = { 4800 };
    profile.scan_methods = { ScanMethod::FLATBED };
    motor.profiles.push_back(std::move(profile));

    profile = MotorProfile();
    profile.slope = MotorSlope::create_from_steps(54612, 1500, 219);
    profile.step_type = StepType::QUARTER;
    profile.motor_vref = 2;
    profile.resolutions = { 300, 600 };
    profile.scan_methods = { ScanMethod::TRANSPARENCY,
                             ScanMethod::TRANSPARENCY_INFRARED };
    motor.profiles.push_back(std::move(profile));

    profile = MotorProfile();
    profile.slope = MotorSlope::create_from_steps(54612, 1500, 219);
    profile.step_type = StepType::QUARTER;
    profile.motor_vref = 1;
    profile.resolutions = { 1200, 2400 };
    profile.scan_methods = { ScanMethod::TRANSPARENCY,
                             ScanMethod::TRANSPARENCY_INFRARED };
    motor.profiles.push_back(std::move(profile));

    profile = MotorProfile();
    profile.slope = MotorSlope::create_from_steps(54612, 1500, 219);
    profile.step_type = StepType::QUARTER;
    profile.motor_vref = 0;
    profile.resolutions = { 4800 };
    profile.scan_methods = { ScanMethod::TRANSPARENCY,
                             ScanMethod::TRANSPARENCY_INFRARED };
    motor.profiles.push_back(std::move(profile));

    profile = MotorProfile();
    profile.slope = MotorSlope::create_from_steps(59240, 582, 1020);
    profile.step_type = StepType::QUARTER;
    profile.motor_vref = 2;
    motor.fast_profiles.push_back(std::move(profile));

    s_motors->push_back(std::move(motor));


    motor = Genesys_Motor();
    motor.id = MotorId::CANON_LIDE_110;
    motor.base_ydpi = 4800;
    motor.profiles.push_back({MotorSlope::create_from_steps(62496, 335, 255),
                              StepType::FULL, 2768});
    motor.profiles.push_back({MotorSlope::create_from_steps(62496, 335, 469),
                              StepType::HALF, 5360});
    motor.profiles.push_back({MotorSlope::create_from_steps(62496, 2632, 3),
                              StepType::HALF, 10528});
    motor.profiles.push_back({MotorSlope::create_from_steps(62496, 10432, 3),
                              StepType::QUARTER, 20864});
    s_motors->push_back(std::move(motor));


    motor = Genesys_Motor();
    motor.id = MotorId::CANON_LIDE_120;
    motor.base_ydpi = 4800;
    motor.profiles.push_back({MotorSlope::create_from_steps(62496, 864, 127),
                              StepType::FULL, 4608});
    motor.profiles.push_back({MotorSlope::create_from_steps(62496, 1338, 63),
                              StepType::HALF, 5360});
    motor.profiles.push_back({MotorSlope::create_from_steps(62464, 2632, 3),
                              StepType::QUARTER, 10528});
    motor.profiles.push_back({MotorSlope::create_from_steps(62592, 10432, 5),
                              StepType::QUARTER, 20864});
    s_motors->push_back(std::move(motor));


    motor = Genesys_Motor();
    motor.id = MotorId::CANON_LIDE_210;
    motor.base_ydpi = 4800;
    motor.profiles.push_back({MotorSlope::create_from_steps(62496, 335, 255),
                              StepType::FULL, 2768});
    motor.profiles.push_back({MotorSlope::create_from_steps(62496, 335, 469),
                              StepType::HALF, 5360});
    motor.profiles.push_back({MotorSlope::create_from_steps(62496, 2632, 3),
                              StepType::HALF, 10528});
    motor.profiles.push_back({MotorSlope::create_from_steps(62496, 10432, 4),
                              StepType::QUARTER, 20864});
    motor.profiles.push_back({MotorSlope::create_from_steps(62496, 10432, 4),
                              StepType::EIGHTH, 41536});
    s_motors->push_back(std::move(motor));


    motor = Genesys_Motor();
    motor.id = MotorId::PLUSTEK_OPTICPRO_3600;
    motor.base_ydpi = 1200;

    profile = MotorProfile{MotorSlope::create_from_steps(3500, 1300, 60), StepType::FULL, 0};
    profile.resolutions = {75, 100, 150, 200};
    motor.profiles.push_back(profile);

    // FIXME: this motor profile is almost useless
    profile = MotorProfile{MotorSlope::create_from_steps(3500, 3250, 60), StepType::HALF, 0};
    profile.resolutions = {300, 400, 600, 1200};
    motor.profiles.push_back(profile);

    profile = MotorProfile{MotorSlope::create_from_steps(3500, 1300, 60), StepType::FULL, 0};
    motor.fast_profiles.push_back(profile);
    s_motors->push_back(std::move(motor));


    motor = Genesys_Motor();
    motor.id = MotorId::PLUSTEK_OPTICFILM_7200;
    motor.base_ydpi = 3600;

    profile = MotorProfile();
    profile.slope = MotorSlope::create_from_steps(20000 * 2, 600 * 2, 200);
    profile.step_type = StepType::HALF;
    profile.motor_vref = 0;
    motor.profiles.push_back(std::move(profile));

    s_motors->push_back(std::move(motor));


    motor = Genesys_Motor();
    motor.id = MotorId::PLUSTEK_OPTICFILM_7200I;
    motor.base_ydpi = 3600;

    profile = MotorProfile();
    profile.slope = MotorSlope::create_from_steps(34722 * 2, 454 * 2, 40);
    profile.step_type = StepType::HALF;
    profile.motor_vref = 3;
    motor.profiles.push_back(std::move(profile));

    profile = MotorProfile();
    profile.slope = MotorSlope::create_from_steps(34722 * 2, 454 * 2, 40);
    profile.step_type = StepType::HALF;
    profile.motor_vref = 0;
    motor.fast_profiles.push_back(std::move(profile));

    s_motors->push_back(std::move(motor));


    motor = Genesys_Motor();
    motor.id = MotorId::PLUSTEK_OPTICFILM_7300;
    motor.base_ydpi = 3600;

    profile = MotorProfile();
    profile.slope = MotorSlope::create_from_steps(56818 * 4, 454 * 4, 30);
    profile.step_type = StepType::QUARTER;
    profile.motor_vref = 3;
    motor.profiles.push_back(std::move(profile));

    profile = MotorProfile();
    profile.slope = MotorSlope::create_from_steps(56818 * 4, 454 * 4, 30);
    profile.step_type = StepType::QUARTER;
    profile.motor_vref = 0;
    motor.fast_profiles.push_back(std::move(profile));

    s_motors->push_back(std::move(motor));


    motor = Genesys_Motor();
    motor.id = MotorId::PLUSTEK_OPTICFILM_7400;
    motor.base_ydpi = 3600;

    profile = MotorProfile();
    profile.slope = MotorSlope::create_from_steps(64102 * 4, 400 * 4, 30);
    profile.step_type = StepType::QUARTER;
    profile.motor_vref = 3;
    motor.profiles.push_back(profile);
    motor.fast_profiles.push_back(profile);
    s_motors->push_back(std::move(motor));


    motor = Genesys_Motor();
    motor.id = MotorId::PLUSTEK_OPTICFILM_7500I;
    motor.base_ydpi = 3600;

    profile = MotorProfile();
    profile.slope = MotorSlope::create_from_steps(56818 * 4, 454 * 4, 30);
    profile.step_type = StepType::QUARTER;
    profile.motor_vref = 3;
    motor.profiles.push_back(std::move(profile));

    profile = MotorProfile();
    profile.slope = MotorSlope::create_from_steps(56818 * 4, 454 * 4, 30);
    profile.step_type = StepType::QUARTER;
    profile.motor_vref = 0;
    motor.fast_profiles.push_back(std::move(profile));

    s_motors->push_back(std::move(motor));


    motor = Genesys_Motor();
    motor.id = MotorId::PLUSTEK_OPTICFILM_8200I;
    motor.base_ydpi = 3600;

    profile = MotorProfile();
    profile.slope = MotorSlope::create_from_steps(64102 * 4, 400 * 4, 100);
    profile.step_type = StepType::QUARTER;
    profile.motor_vref = 3;
    motor.profiles.push_back(profile);
    motor.fast_profiles.push_back(profile);
    s_motors->push_back(std::move(motor));
    motor = Genesys_Motor();
    motor.id = MotorId::PLUSTEK_OPTICFILM_8200I_SE;
    motor.base_ydpi = 7200;

    // SilverFast's two slope tables, exactly as its driver uploads them (USB captures): they
    // follow no constant-acceleration ramp, so they are given as tables. The fast one is used
    // for the calibration strips and the second positioning feed, the slow one for the first.
    profile = MotorProfile();
    profile.slope = MotorSlope::create_from_table({
        5854, 1755, 1344, 1150, 1030, 945, 881, 830, 788, 754, 724, 698,
        675, 654, 636, 619, 604, 590, 577, 565, 554, 544, 534, 525,
        517, 509, 501, 494, 487, 480, 474, 468, 462, 457, 452, 447,
        442, 437, 433, 428, 424, 420, 416, 413, 409, 405, 402, 399,
        396, 392, 389, 386, 384, 381, 378, 375, 373, 370, 368, 365,
        363, 361, 358, 356, 354, 352, 350, 348, 346, 344, 342, 340,
        338, 337, 335, 333, 331, 330, 328, 326, 325, 323, 322, 320,
        319, 317, 316, 315, 313, 312, 310, 309, 308, 307, 305, 304,
        303, 302, 300, 299, 298, 297, 296, 295, 294, 293, 292, 290,
        289, 288, 287, 286, 285, 284, 283, 282, 282, 281, 280, 279,
        278, 277, 276, 275, 274, 274, 273, 272, 271, 270, 269, 269,
        268, 267, 266, 266, 265, 264, 263, 263, 262, 261, 260, 260,
        259, 258, 258, 257, 256, 256, 255, 254, 254, 253, 252, 252,
        251, 251, 250, 249, 249, 248, 248, 247, 246, 246, 245, 245,
        244, 244, 243, 242, 242, 241, 241, 240, 240, 239, 239, 238,
        238, 237, 237, 236, 236, 235, 235, 234, 234, 233, 233, 232,
        232, 231, 231, 230, 230, 230, 229, 229, 228, 228, 227, 227,
        227, 226, 226, 225, 225, 224, 224, 224, 223, 223, 222, 222,
        222, 221, 221, 220, 220, 220, 219, 219, 218, 218, 218, 217,
        217, 217, 216, 216, 216, 215, 215, 214, 214, 214, 213, 213,
        213, 212, 212, 212, 211, 211, 211, 210, 210, 210, 209, 209,
        209, 208, 208, 208,
    }, StepType::QUARTER);
    profile.step_type = StepType::QUARTER;
    profile.motor_vref = 3;
    motor.fast_profiles.push_back(profile);

    profile = MotorProfile();
    profile.slope = MotorSlope::create_from_table({
        8116, 4863, 4006, 3581, 3306, 3105, 2955, 2828, 2726, 2639, 2561, 2495,
        2436, 2383, 2334, 2289, 2248, 2210, 2175, 2144, 2113, 2085, 2057, 2032,
        2009, 1985, 1965, 1944, 1924, 1905, 1888, 1871, 1854, 1839, 1823, 1808,
        1794, 1781, 1768, 1755, 1743, 1731, 1719, 1708, 1697, 1686, 1676, 1666,
        1656, 1647, 1638, 1629, 1620, 1611, 1603, 1595, 1587, 1580, 1572, 1564,
        1557, 1549, 1543, 1536, 1529, 1523, 1516, 1510, 1504, 1497, 1492, 1486,
        1480, 1474, 1469, 1463, 1458, 1453, 1447, 1442, 1437, 1432, 1427, 1422,
        1418, 1414, 1409, 1404, 1400, 1395, 1391, 1387, 1382, 1378, 1375, 1370,
        1366, 1363, 1358, 1355, 1351, 1347, 1344, 1340, 1336, 1333, 1329, 1326,
        1322, 1319, 1315, 1313, 1309, 1306, 1302, 1300, 1296, 1293, 1290, 1287,
        1284, 1281, 1278, 1275, 1272, 1270, 1267, 1264, 1261, 1258, 1256, 1253,
        1251, 1248, 1245, 1243, 1240, 1238, 1235, 1233, 1230, 1228, 1226, 1223,
        1221, 1218, 1216, 1214, 1211, 1209, 1207, 1205, 1203, 1200, 1198, 1196,
        1194, 1192, 1189, 1188, 1185, 1183, 1182, 1179, 1177, 1175, 1173, 1172,
        1169, 1168, 1166, 1164, 1162, 1160, 1158, 1156, 1154, 1153, 1151, 1149,
        1147, 1145, 1144, 1142, 1140, 1138, 1137, 1135, 1133, 1132, 1130, 1129,
        1127, 1125, 1124, 1122, 1121, 1119, 1117, 1116, 1114, 1113, 1111, 1110,
        1108, 1107, 1105, 1104, 1102, 1101, 1099, 1098, 1096, 1095, 1094, 1092,
        1091, 1089, 1088, 1086, 1085, 1084, 1082, 1081, 1079, 1078, 1077, 1076,
        1074, 1073, 1072, 1071, 1069, 1068, 1067, 1065, 1064, 1063, 1062, 1060,
        1059, 1058, 1057, 1055, 1054, 1053, 1052, 1051, 1049, 1048, 1047, 1046,
        1045, 1044, 1042, 1041,
    }, StepType::QUARTER);
    profile.step_type = StepType::QUARTER;
    profile.motor_vref = 3;
    motor.profiles.push_back(profile);

    s_motors->push_back(std::move(motor));


    motor = Genesys_Motor();
    motor.id = MotorId::IMG101;
    motor.base_ydpi = 600;
    motor.profiles.push_back({MotorSlope::create_from_steps(22000, 1000, 1017),
                              StepType::HALF, 11000});
    s_motors->push_back(std::move(motor));


    motor = Genesys_Motor();
    motor.id = MotorId::PLUSTEK_OPTICBOOK_3800;
    motor.base_ydpi = 600;
    motor.profiles.push_back({MotorSlope::create_from_steps(22000, 1000, 1017),
                              StepType::HALF, 11000});
    s_motors->push_back(std::move(motor));


    motor = Genesys_Motor();
    motor.id = MotorId::CANON_LIDE_80;
    motor.base_ydpi = 2400;
    motor.profiles.push_back({MotorSlope::create_from_steps(9560, 1912, 31), StepType::FULL, 0});
    s_motors->push_back(std::move(motor));
}

} // namespace genesys
