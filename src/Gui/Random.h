// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef RANDOM_H
#define RANDOM_H

/*
 * Random.h
 *
 * A convenient class to generate random numbers
 *
 */

class Random
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
