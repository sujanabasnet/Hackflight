/*
   Hackflight sketch for Teensy4.0 Flight Controller with Spektrum DSMX receiver

   Additional libraries needed:

       https://github.com/simondlevy/EM7180
       https://github.com/simondlevy/CrossPlatformDataBus
       https://github.com/simondlevy/SpektrumDSM 

   Copyright (c) 2019 Simon D. Levy

   This file is part of Hackflight.

   Hackflight is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Hackflight is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   You should have received a copy of the GNU General Public License
   along with Hackflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "hackflight.hpp"
#include "boards/arduino/teensy40.hpp"
#include "receivers/arduino/dsmx_serial1.hpp"
#include "mixers/quadxcf.hpp"
#include "pidcontrollers/level.hpp"
#include "pidcontrollers/rate.hpp"

static constexpr uint8_t CHANNEL_MAP[6] = {0, 1, 2, 3, 6, 4};
static constexpr float DEMAND_SCALE = 8.58f;

hf::Hackflight h;

hf::DSMX_Receiver_Serial1 rc = hf::DSMX_Receiver_Serial1(CHANNEL_MAP, DEMAND_SCALE);

hf::MixerQuadXCF mixer;

hf::RatePid ratePid = hf::RatePid(0.01f, 0.00f, 0.00f, 0.10f, 0.01f);

hf::LevelPid levelPid = hf::LevelPid(0.40f);

void setup(void)
{
    // Initialize Hackflight firmware
    h.init(new hf::Teensy40(), &rc, &mixer);

    // Add PID controllers
    h.addPidController(&levelPid);
    h.addPidController(&levelPid);

}

void loop(void)
{
    h.update();
}
