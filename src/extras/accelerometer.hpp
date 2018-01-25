/* 
   accelerometer.hpp: Altitude estimation via accelerometer Z-axis integration

   Adapted from

   https://github.com/multiwii/baseflight/blob/master/src/imu.c

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
   along with EM7180.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "debug.hpp"
#include "datatypes.hpp"

namespace hf {

    class Accelerometer {

        private: 

            // These constants probably don't need to be changed
            const float lpfCutoff  = 5.00f;
            const float lpfFactor  = 0.25f;
            const float deadband   = 0.02f;

            float    accZ;
            float    fc;
            float    lpf[3];
            uint32_t previousTimeUsec;
            float    accelGsSmoothed[3];
            float    verticalVelocity;
            float    zOffset;

            static float rotate(float ned[3], stateval_t angles[3])
            {
                float cosx = cosf(-angles[0].value);
                float sinx = sinf(-angles[0].value);
                float cosy = cosf(-angles[1].value);
                float siny = sinf(-angles[1].value);

                return ned[0] * siny + ned[1] * (-sinx * cosy) + ned[2] * (cosy * cosx);
            }

        public:

            void init(void)
            {
                // Calculate RC time constant used in the low-pass filter
                fc = (float)(0.5f / (M_PI * lpfCutoff)); 

                for (int k=0; k<3; ++k) {
                    accelGsSmoothed[k] = 0;
                    lpf[k] = 0;
                }

                verticalVelocity = 0;
                previousTimeUsec = 0;
                accZ = 0;
                zOffset = 0;
            }

            void update(stateval_t angles[3], float accelGs[3], uint32_t currentTimeUsec, bool armed)
            {
                // Track delta time
                uint32_t dT_usec = currentTimeUsec - previousTimeUsec;
                previousTimeUsec = currentTimeUsec;

                // Smoothe the raw values if indicated
                for (uint8_t k=0; k<3; k++) {
                    if (lpfFactor > 0) {
                        lpf[k] = Filter::complementary(accelGs[k], lpf[k], lpfFactor);
                        accelGsSmoothed[k] = lpf[k];
                    } else {
                        accelGsSmoothed[k] = accelGs[k];
                    }
                }

                // Rotate accel values into the earth frame
                float rotatedZ = rotate(accelGsSmoothed, angles);

                // Get vertical acceleration offset at rest
                if (!armed) {
                    zOffset = rotatedZ;
                }

                // Subtract rest acceleration to get zero acceleration
                rotatedZ -= zOffset;

                // Compute smoothed vertical acceleration
                float dT_sec = dT_usec * 1e-6f;
                accZ += ((dT_sec / (fc + dT_sec)) * (rotatedZ - accZ));

            } // update


            float getVerticalVelocity(uint32_t dTimeMicros)
            {
                // Integrate vertical acceleration to compute IMU velocity in meters per second
                verticalVelocity += Filter::deadband(accZ, deadband) * 9.80665f  * dTimeMicros/1e6;

                return verticalVelocity;
            }

    }; // class Accelerometer

} // namespace hf
