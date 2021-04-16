/*
   Hackflight core algorithm

   Copyright (c) 2020 Simon D. Levy

   This file is part of Hackflight.

   Hackflight is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Hackflight is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MEReceiverHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   You should have received a copy of the GNU General Public License
   along with Hackflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <RFT_board.hpp>

#include "mspparser.hpp"
#include "receiver.hpp"
#include "sensor.hpp"
#include "state.hpp"
#include "pidcontroller.hpp"
#include "mixer.hpp"
#include "timertasks/pidtask.hpp"
#include "timertasks/serialtask.hpp"

#include <RFT_filters.hpp>

namespace hf {

    class Hackflight {

        private:

            static constexpr float MAX_ARMING_ANGLE_DEGREES = 25.0f;

            // Sensors 
            Sensor * _sensors[256] = {NULL};
            uint8_t _sensor_count = 0;

            // Safety
            bool _safeToArm = false;

            // Timer task for PID controllers
            PidTask _pidTask;

            // Passed to Hackflight::begin() for a particular build
            Mixer * _mixer    = NULL;

            // Serial timer task for GCS
            SerialTask _serialTask;

            bool safeAngle(uint8_t axis)
            {
                return fabs(_state.x[axis]) < rft::Filter::deg2rad(MAX_ARMING_ANGLE_DEGREES);
            }

            void checkSensors(void)
            {
                // Some gyrometers may need to know the current time
                float time = _board->getTime();

                for (uint8_t k=0; k<_sensor_count; ++k) {
                    Sensor * sensor = _sensors[k];
                    float time = _board->getTime();
                    if (sensor->ready(time)) {
                        sensor->modifyState(_state, time);
                    }
                }
             }

            void startSensors(void)
            {
                for (uint8_t k=0; k<_sensor_count; ++k) {
                    _sensors[k]->begin();
                }
             }

            rft::Board    * _board    = NULL;
            Receiver * _receiver = NULL;

            // Vehicle state
            state_t _state;

            void checkReceiver(void)
            {
                // Sync failsafe to receiver
                if (_receiver->lostSignal() && _state.armed) {
                    _mixer->cut();
                    _state.armed = false;
                    _state.failsafe = true;
                    _board->showArmedStatus(false);
                    return;
                }

                // Check whether receiver data is available
                if (!_receiver->getDemands()) {
                    return;
                }

                // Disarm
                if (_state.armed && !_receiver->getAux1State()) {
                    _state.armed = false;
                } 

                // Avoid arming if aux1 switch down on startup
                if (!_safeToArm) {
                    _safeToArm = !_receiver->getAux1State();
                }

                // Arm (after lots of safety checks!)
                if (_safeToArm && !_state.armed && _receiver->throttleIsDown() && _receiver->getAux1State() && 
                        !_state.failsafe && safeAngle(STATE_PHI) && safeAngle(STATE_THETA)) {
                    _state.armed = true;
                }

                // Cut motors on throttle-down
                if (_state.armed && _receiver->throttleIsDown()) {
                    _mixer->cut();
                }

                // Set LED based on arming status
                _board->showArmedStatus(_state.armed);

            } // checkReceiver

        public:

            Hackflight(rft::Board * board, Receiver * receiver, Mixer * mixer)
            {
                // Store the essentials
                _board = board;
                _receiver = receiver;
                _mixer = mixer;

                // Support adding new sensors and PID controllers
                _sensor_count = 0;
            }

            void begin(bool armed=false)
            {  
                // Initialize state
                memset(&_state, 0, sizeof(state_t));

                // Start the sensors
                startSensors();

                // Start the board
                _board->begin();

                // Start the receiver
                _receiver->begin();

                // Setup failsafe
                _state.failsafe = false;

                // Initialize timer task for PID controllers
                _pidTask.begin(_board, _receiver, _mixer, &_state);
 
                // Initialize serial timer task
                _serialTask.begin(_board, &_state, _receiver, _mixer);

                // Support safety override by simulator
                _state.armed = armed;

                // Start the mixer
                _mixer->begin();

            } // init

            void addSensor(Sensor * sensor) 
            {
                _sensors[_sensor_count++] = sensor;
            }

            void addPidController(PidController * pidController, uint8_t auxState=0) 
            {
                _pidTask.addPidController(pidController, auxState);
            }

            void update(void)
            {
                // Grab control signal if available
                checkReceiver();

                // Update PID controllers task
                _pidTask.update();

                // Check sensors
                checkSensors();

                // Update serial comms task
                _serialTask.update();

            }

    }; // class Hackflight

} // namespace
