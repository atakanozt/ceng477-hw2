#include <iomanip>
#include "Rotation.h"

Rotation::Rotation() {
    this->rotationId = -1;
    this->angle = 0;
    this->ux = 0;
    this->uy = 0;
    this->uz = 0;
}

Rotation::Rotation(int rotationId, double angle, double x, double y, double z)
{
    this->rotationId = rotationId;
    this->angle = angle;
    this->ux = x;
    this->uy = y;
    this->uz = z;
}

Matrix4 Rotation::doRotation(Matrix4 matrix) {
    Vec3 u = Vec3(this->ux, this->uy, this->uz);
    Vec3 v, w;

    if(ux <= uy && ux <= uz) {
        v = Vec3(0, -uz, uy);
    }
    else if(uy <= ux && uy <= uz) {
        v = Vec3(uz, 0, -ux);
    }
    else {
        v = Vec3(-uy, ux, 0);
    }

    v = normalizeVec3(v);
    w = crossProductVec3(u, v);
    w = normalizeVec3(w);

    double matrix_values[4][4] = {
        {u.x, u.y, u.z, 0},
        {v.x, v.y, v.z, 0},
        {w.x, w.y, w.z, 0},
        {0, 0, 0, 1}
    };

    double matrix_values_transpose[4][4] = {
        {u.x, v.x, w.x, 0},
        {u.y, v.y, w.y, 0},
        {u.z, v.z, w.z, 0},
        {0, 0, 0, 1}
    };

    double radian = this->angle * M_PI / 180.0;
    double cosTheta = cos(radian);
    double sinTheta = sin(radian);

    double matrix_values_with_theta[4][4] = {
        {1, 0, 0, 0},
        {0, cosTheta, -sinTheta, 0},
        {0, sinTheta, cosTheta, 0},
        {0, 0, 0, 1}
    };

    Matrix4 M = Matrix4(matrix_values);
    Matrix4 MT = Matrix4(matrix_values_transpose);
    Matrix4 Rx = Matrix4(matrix_values_with_theta);

    Matrix4 transformation_matrix = multiplyMatrixWithMatrix(MT, multiplyMatrixWithMatrix(Rx, M));
    return multiplyMatrixWithMatrix(transformation_matrix, matrix);
}

std::ostream &operator<<(std::ostream &os, const Rotation &r)
{
    os << std::fixed << std::setprecision(3) << "Rotation " << r.rotationId << " => [angle=" << r.angle << ", " << r.ux << ", " << r.uy << ", " << r.uz << "]";
    return os;
}