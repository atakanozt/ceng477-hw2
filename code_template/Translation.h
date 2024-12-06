#ifndef __TRANSLATION_H__
#define __TRANSLATION_H__
#include "Matrix4.h"
#include "Helpers.h"

class Translation
{
public:
    int translationId;
    double tx, ty, tz;

    Translation();
    Translation(int translationId, double tx, double ty, double tz);
    Matrix4 doTranslation(Matrix4 matrix);
    friend std::ostream &operator<<(std::ostream &os, const Translation &t);
};

#endif