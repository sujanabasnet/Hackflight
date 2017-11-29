/*
   debug.hpp : Cross-platform serial debugging support for Hackflight

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

#pragma once

#include <stdarg.h>
#include "board.hpp"

namespace hf {

    class Debug {

        public:

            static void printf(const char * fmt, ...)
            {
                va_list ap;
                va_start(ap, fmt);
                char buf[200];
                vsnprintf(buf, 200, fmt, ap); 
                Board::outbuf(buf);
                va_end(ap);
            }

    }; // class Debug

} // namespace hf


