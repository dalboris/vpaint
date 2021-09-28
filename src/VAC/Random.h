// Copyright (C) 2012-2019 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef RANDOM_H
#define RANDOM_H

#include "vpaint_global.h"
/*
 * Random.h
 *
 * A convenient class to generate random numbers
 *
 */

class Q_VPAINT_EXPORT Random
{
public:
    // Initiliaze with a seed. The second one use the current time as seed.
    static void setSeed(int seed);
    static void setRandomSeed();

    // Get a random double, between [0,1[ or [min,max[ 
    static double random();
    static double random(double min, double max);

    // Get a random int, between [0,RAND_MAX] or [min,max]    
    static int randomInt(); 
    static int randomInt(int min, int max);
};

#endif
