/*
   Angular-velocity-based PID controller for roll, pitch, yaw

   Copyright (c) 2021 Juan Gallostra and Simon D. Levy

   MIT License
 */

#pragma once

#include <RFT_filters.hpp>
#include <RFT_state.hpp>
#include <rft_closedloops/pidcontroller.hpp>

#include "demands.hpp"
#include "receiver.hpp"
#include "pidcontrollers/angvel.hpp"

namespace hf {

    class RatePid : public rft::PidController {

        private: 

            // Aribtrary constants
            static constexpr float BIG_YAW_DEMAND = 0.1f;

            // Rate mode uses a rate controller for roll, pitch
            _AngularVelocityPid _rollPid;
            _AngularVelocityPid _pitchPid;
            _AngularVelocityPid _yawPid;

        public:

            RatePid(const float Kp, const float Ki, const float Kd, const float Kp_yaw, const float Ki_yaw) 
            {
                _rollPid.begin(Kp, Ki, Kd);
                _pitchPid.begin(Kp, Ki, Kd);
                _yawPid.begin(Kp_yaw, Ki_yaw, 0);
            }

            void modifyDemands(rft::State * state, float * demands)
            {
                float * x = ((State *)state)->x;

                demands[DEMANDS_ROLL]  = _rollPid.compute(demands[DEMANDS_ROLL], x[State::STATE_DPHI]);

                demands[DEMANDS_PITCH] = _pitchPid.compute(demands[DEMANDS_PITCH], x[State::STATE_DTHETA]);
                demands[DEMANDS_YAW]   = _yawPid.compute(demands[DEMANDS_YAW], x[State::STATE_DPSI]);

                // Prevent "yaw jump" during correction
                demands[DEMANDS_YAW] =
                    rft::Filter::constrainAbs(demands[DEMANDS_YAW], 0.1 + fabs(demands[DEMANDS_YAW]));

                // Reset yaw integral on large yaw command
                if (fabs(demands[DEMANDS_YAW]) > BIG_YAW_DEMAND) {
                    _yawPid.reset();
                }
            }

            virtual void resetOnInactivity(bool inactive) override
            {
                // Check throttle-down for integral reset
                _rollPid.resetOnInactivity(inactive);
                _pitchPid.resetOnInactivity(inactive);
                _yawPid.resetOnInactivity(inactive);
            }

    };  // class RatePid

} // namespace hf
