#ifndef __SCALING_H__
#define __SCALING_H__
#include "Matrix4.h"
#include "Helpers.h"

class Scaling
{
public:
    int scalingId;
    double sx, sy, sz;

    Scaling();
    Scaling(int scalingId, double sx, double sy, double sz);
    Matrix4 doScaling(Matrix4 matrix);
    friend std::ostream &operator<<(std::ostream &os, const Scaling &s);
};

#endif