// Copyright (C) 2012-2019 The VPaint Developers
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

#include "Random.h"

// Removed for Windows compatibility
// #include <ctime>

#include <cstdlib>

void Random::setSeed(int seed)
{
    srand(seed);
}

void Random::setRandomSeed()
{
    setSeed(0); // setSeed(time(0));
}

double Random::random()
{
    return rand()/(double(RAND_MAX)+1);
}

double Random::random(double min, double max)
{
    if (max>min)
            return random()*(max-min)+min;
    else
            return random()*(min-max)+max;
}

int Random::randomInt()
{
    return rand();
}

int Random::randomInt(int min, int max)
{
    if (max>min)
            return min + rand() % (max-min+1);
    else
            return max + rand() % (min-max+1);
}
