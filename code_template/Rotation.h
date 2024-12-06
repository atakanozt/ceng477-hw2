#ifndef __ROTATION_H__
#define __ROTATION_H__
#include "Matrix4.h"
#include "Helpers.h"
class Rotation
{
public:
    int rotationId;
    double angle, ux, uy, uz;

    Rotation();
    Rotation(int rotationId, double angle, double x, double y, double z);
    Matrix4 doRotation(Matrix4 matrix);
    friend std::ostream &operator<<(std::ostream &os, const Rotation &r);
};

#endif