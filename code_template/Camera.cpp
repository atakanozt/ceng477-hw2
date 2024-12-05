#include <iostream>
#include <iomanip>
#include "Camera.h"

Camera::Camera() {}

Camera::Camera(int cameraId,
               int projectionType,
               Vec3 position, Vec3 gaze,
               Vec3 u, Vec3 v, Vec3 w,
               double left, double right, double bottom, double top,
               double near, double far,
               int horRes, int verRes,
               std::string outputFilename)
{

    this->cameraId = cameraId;
    this->projectionType = projectionType;
    this->position = position;
    this->gaze = gaze;
    this->u = u;
    this->v = v;
    this->w = w;
    this->left = left;
    this->right = right;
    this->bottom = bottom;
    this->top = top;
    this->near = near;
    this->far = far;
    this->horRes = horRes;
    this->verRes = verRes;
    this->outputFilename = outputFilename;
}

Camera::Camera(const Camera &other)
{
    this->cameraId = other.cameraId;
    this->projectionType = other.projectionType;
    this->position = other.position;
    this->gaze = other.gaze;
    this->u = other.u;
    this->v = other.v;
    this->w = other.w;
    this->left = other.left;
    this->right = other.right;
    this->bottom = other.bottom;
    this->top = other.top;
    this->near = other.near;
    this->far = other.far;
    this->horRes = other.horRes;
    this->verRes = other.verRes;
    this->outputFilename = other.outputFilename;
}

std::ostream &operator<<(std::ostream &os, const Camera &c)
{
    const char *camType = c.projectionType ? "perspective" : "orthographic";

    os << std::fixed << std::setprecision(6) << "Camera " << c.cameraId << " (" << camType << ") => pos: " << c.position << " gaze: " << c.gaze << std::endl
       << "\tu: " << c.u << " v: " << c.v << " w: " << c.w << std::endl
       << std::fixed << std::setprecision(3) << "\tleft: " << c.left << " right: " << c.right << " bottom: " << c.bottom << " top: " << c.top << std::endl
       << "\tnear: " << c.near << " far: " << c.far << " resolutions: " << c.horRes << "x" << c.verRes << " fileName: " << c.outputFilename;

    return os;
}

Matrix4 Camera::getCameraTransformationMatrix() {
    Vec3 e = position;
    double matrix_values[4][4] = {
        {u.x, u.y, u.z, -(u.x * e.x + u.y * e.y + u.z * e.z)},
        {v.x, v.y, v.z, -(v.x * e.x + v.y * e.y + v.z * e.z)},
        {w.x, w.y, w.z, -(w.x * e.x + w.y * e.y + w.z * e.z)},
        {0, 0, 0, 1}
    };

    return Matrix4(matrix_values);
}

Matrix4 Camera::getProjectionTransformationMatrix() {
    double l = left;
    double r = right;
    double t = top;
    double b = bottom;
    double n = near;
    double f = far;

    double orthographic_matrix_values[4][4] = {
        {2/(r-l), 0, 0, -(r+l)/(r-l)},
        {0, 2/(t-b), 0, -(t+b)/(t-b)},
        {0, 0, -2/(f-n), -(f+n)/(f-n)},
        {0, 0, 0, 1}
    };
    Matrix4 orthographic_matrix = Matrix4(orthographic_matrix_values);

    if (projectionType == 0) {
        return orthographic_matrix;
    }

    double perspective_matrix_values[4][4] = {
        {2*n/(r-l), 0, (r+l)/(r-l), 0},
        {0, 2*n/(t-b), (t+b)/(t-b), 0},
        {0, 0, -(f+n)/(f-n), -2*(f*n)/(f-n)},
        {0, 0, -1, 0}
    };
    Matrix4 perspective_matrix = Matrix4(perspective_matrix_values);

    return perspective_matrix;
}

Matrix4 Camera::getViewportTransformationMatrix() {
    double matrix_values[4][4] = {
        {horRes*0.5, 0, 0, (horRes-1)*0.5},
        {0, verRes*0.5, 0, (verRes-1)*0.5},
        {0, 0, 0.5, 0.5},
        {0, 0, 0, 1.0}
    };
    return Matrix4(matrix_values);
}