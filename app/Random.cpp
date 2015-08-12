// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

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
