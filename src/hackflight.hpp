/*
   Hackflight core algorithm

   Copyright (c) 2020 Simon D. Levy

   MIT License
 */

#pragma once

#include <RFT_board.hpp>

#include "mspparser.hpp"
#include "receiver.hpp"
#include "state.hpp"
#include "mixer.hpp"
#include "serialtask.hpp"

#include <RFT_sensor.hpp>
#include <RFT_filters.hpp>

#include <RoboFirmwareToolkit.hpp>

namespace hf {

    class Hackflight : public rft::RFT {

        private:

            // Passed to Hackflight::begin() for a particular build
            Mixer * _mixer    = NULL;

            // Serial timer task for GCS
            SerialTask _serialTask;

            Receiver * _receiver = NULL;

            // Vehicle state
            State _state;

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
                if (!_receiver->ready()) {
                    return;
                }

                // Disarm
                if (_state.armed && !_receiver->inArmedState()) {
                    _state.armed = false;
                } 

                // Avoid arming if armed on startup
                if (!_safeToArm) {
                    _safeToArm = !_receiver->inArmedState();
                }

                // Arm (after lots of safety checks!)
                if (
                        _safeToArm && 
                        !_state.armed && 
                        _receiver->inactive() && 
                        _receiver->inArmedState() && 
                        !_state.failsafe && 
                        _state.safeToArm()) {

                    _state.armed = true;
                }

                // Cut motors on inactivity (e.g., throttle-down)
                if (_state.armed && _receiver->inactive()) {
                    _mixer->cut();
                }

                // Set LED based on arming status
                _board->showArmedStatus(_state.armed);

            } // checkReceiver

        protected:

            virtual State * getState(void) override
            {
                return &_state;
            }

        public:

            Hackflight(rft::Board * board, Receiver * receiver, Mixer * mixer)
                : rft::RFT(board, receiver, mixer)
            {
                // Store the essentials
                _receiver = receiver;
                _mixer = mixer;
            }

            void begin(bool armed=false)
            {  
                RFT::begin(armed);

                // Initialize state
                memset(&_state, 0, sizeof(State));

                // Initialize serial timer task
                _serialTask.begin(_board, &_state, _receiver, _mixer);

            } // init

            void update(void)
            {
                // Grab control signal if available
                checkReceiver();

                RFT::update();

                // Update serial comms task
                _serialTask.update();

            }

    }; // class Hackflight

} // namespace
