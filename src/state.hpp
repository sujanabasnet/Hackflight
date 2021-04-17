/*
   Hackflight state class

   Copyright (c) 2018 Simon D. Levy

   MIT License
   */

#pragma once

#include <RFT_filters.hpp>

namespace hf {

    class State {

        private:

            static constexpr float MAX_ARMING_ANGLE_DEGREES = 25.0f;

        public:

            // See Bouabdallah et al. (2004)
            enum {X, DX, Y, DY, Z, DZ, PHI, DPHI, THETA, DTHETA, PSI, DPSI, SIZE};

            float x[SIZE];

            bool armed = false;
            bool failsafe = false;

            bool safeAngle(uint8_t axis)
            {
                return fabs(x[axis]) < rft::Filter::deg2rad(MAX_ARMING_ANGLE_DEGREES);
            }

            bool safeToArm(void)
            {
                return safeAngle(PHI) && safeAngle(THETA);
            }

    }; // class State

} // namespace hf
