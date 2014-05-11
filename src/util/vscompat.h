/*
 * Copyright 2012-2014 Moritz Hilscher, Thor Erik Lie <thor@thorerik.com>
 * 
 * Visual Studio Compatability based on http://ofekshilon.com/2009/04/10/coding-binary-as-binary/
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

//namespace mapcrafter {
//namespace util {

template <unsigned long N>
struct binary
{
   static unsigned const value
     = binary<N/10>::value << 1   // prepend higher bits
       | N%10;                    // to lowest bit
};
template <>                           // specialization
struct binary<0>                      // terminates recursion
{
static unsigned const value = 0;
};

//}
//}