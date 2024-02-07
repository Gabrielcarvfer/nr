// Copyright (c) 2022 Leonid Moroz (moroz_lv@lp.edu.ua)
//
// SPDX-License-Identifier: MIT

#ifndef FAST_EXP_H
#define FAST_EXP_H

#include <iomanip>

/*
These functions return an approximation of exp(x) with a relative error <0.173%.
They are several times faster than std::exp().

The code assumes that values of type float and double are stored in the IEEE754
single and double precision floating point formats.

References:

Leonid Moroz, Volodymyr Samotyy, Zbigniew Kokosiński, Paweł Gepner.
“Simple multiple precision algorithms for exponential functions”,
IEEE Signal Processing Magazine 39(4):130 - 137 (2022).
(available at https://ieeexplore.ieee.org/document/9810030)
*/

inline float
exp21f(float x)
{
    if (x < -87.0)
    {
        return 0;
    }
    if (x > 88.0)
    {
        return std::numeric_limits<float>::infinity();
    }
    int z = x * 0x00b8aa3b + 0x3f800000;

    union {
        int i;
        float f;
    } zii;

    zii.i = z & 0x7f800000;
    int zif = z & 0x007fffff;
    float d1 = 0.40196114e-7f;
    float d2 = 0xf94ee7 + zif;
    float d3 = 0x560e + zif;
    d2 = d1 * d2;
    zif = d2 * d3;
    zii.i |= zif;
    float y = zii.f;
    return y;
}

[[maybe_unused]] void
testExp21f()
{
    bool pass = true;
    std::cout << "x"
              << "\t\tref"
              << "\t\tfast"
              << "\t\tabsErr"
              << "\t\trelErr" << std::setprecision(3) << std::endl;

    for (int i = -256; i < 256; i++)
    {
        float ref = std::exp(i);
        float fast = exp21f(i);
        float absError = std::abs(fast - ref);
        float relError = absError / ref;
        if ((ref < 0 || ref > 0) && absError > 10e-4 && relError > 0.017)
        {
            pass = false;
        }
        std::cout << std::setw(4) << i << "\t" << std::setw(8) << ref << "\t" << std::setw(8)
                  << fast << "\t" << std::setw(8) << absError << "\t" << std::setw(8) << relError
                  << std::endl;
    }
    std::cout << (pass ? "PASS" : "FAIL") << std::endl;
    exit(!pass);
}

#endif
