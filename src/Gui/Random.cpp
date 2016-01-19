// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

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
