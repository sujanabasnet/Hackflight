/*
   Angular-velocity-based PID controller for roll, pitch, yaw

   Copyright (c) 2018 Juan Gallostra and Simon D. Levy

   MIT License
 */

#pragma once

#include "receiver.hpp"
#include "state.hpp"
#include "demands.hpp"

#include "pidcontrollers/angvel.hpp"

#include <RFT_Debugger.hpp>

namespace hf {

    class RatePid : public rft::PidController {

        private: 

            // Rate mode uses a rate controller for roll, pitch
            AngularVelocityPid _rollPid;
            AngularVelocityPid _pitchPid;

        public:

            RatePid(const float Kp, const float Ki, const float Kd) 
            {
                _rollPid.begin(Kp, Ki, Kd);
                _pitchPid.begin(Kp, Ki, Kd);
            }

            virtual void modifyDemands(rft::State * state, float * demands) override
            {
                State * hfstate = (State *)state;

                // Roll angle and roll demand are both positive for starboard right down
                demands[DEMANDS_ROLL]  = _rollPid.compute(demands[DEMANDS_ROLL],  hfstate->x[State::DPHI]);

                // Pitch demand is postive for stick forward, but pitch angle is positive for nose up.
                // So we negate pitch angle to compute demand
                demands[DEMANDS_PITCH] = _pitchPid.compute(demands[DEMANDS_PITCH], -hfstate->x[State::DTHETA]);
            }

            /* XXX should be replaced by resetOnInactivity()
            virtual void updateReceiver(bool throttleIsDown) override
            {
                // Check throttle-down for integral reset
                _rollPid.updateReceiver(throttleIsDown);
                _pitchPid.updateReceiver(throttleIsDown);
            }*/

    };  // class RatePid

} // namespace hf
