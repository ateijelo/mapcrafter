/*
 * Copyright 2012-2016 Moritz Hilscher
 *
 * This file is part of Mapcrafter.
 *
 * Mapcrafter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mapcrafter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Mapcrafter.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef COMPAT_NULLPTR_H_
#define COMPAT_NULLPTR_H_

#include "../config.h"

#ifndef HAVE_NULLPTR
// official workaround for nullptr
// see also http://stackoverflow.com/questions/2419800/can-nullptr-be-emulated-in-gcc
const // this is a const object...
    class {
  public:
    template <class T>   // convertible to any type
    operator T *() const // of null non-member
    {
        return 0;
    } // pointer...
    template <class C, class T> // or any type of null
    operator T C::*() const     // member pointer...
    {
        return 0;
    }

  private:
    void operator&() const; // whose address can't be taken
} nullptr = {};             // and whose name is nullptr
#endif

#endif /* COMPAT_NULLPTR_H_ */
