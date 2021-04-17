/*
   Abstract class for sensors

   Copyright (c) 2018 Simon D. Levy

   MIT License
 */

#pragma once

#include "state.hpp"

namespace hf {

    class Sensor {

        friend class Hackflight;

        protected:

            virtual void begin(void) { }

            virtual void modifyState(state_t & state, float time) = 0;

            virtual bool ready(float time) = 0;

    };  // class Sensor

} // namespace hf
