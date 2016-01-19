// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

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
