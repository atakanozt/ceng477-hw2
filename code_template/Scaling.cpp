#include <iomanip>
#include "Scaling.h"

Scaling::Scaling() {
    this->scalingId = -1;
    this->sx = 0;
    this->sy = 0;
    this->sz = 0;
}

Scaling::Scaling(int scalingId, double sx, double sy, double sz)
{
    this->scalingId = scalingId;
    this->sx = sx;
    this->sy = sy;
    this->sz = sz;
}

Matrix4 Scaling::doScaling(Matrix4 matrix) {
    double matrix_values[4][4] = {
        {this->sx, 0, 0, 0},
        {0, this->sy, 0, 0},
        {0, 0, this->sz, 0},
        {0, 0, 0, 1}
    };
    Matrix4 scaling_matrix = Matrix4(matrix_values);
    return multiplyMatrixWithMatrix(scaling_matrix, matrix);
}

std::ostream &operator<<(std::ostream &os, const Scaling &s)
{
    os << std::fixed << std::setprecision(3) << "Scaling " << s.scalingId << " => [" << s.sx << ", " << s.sy << ", " << s.sz << "]";

    return os;
}
