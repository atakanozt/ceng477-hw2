#ifndef _SCENE_H_
#define _SCENE_H_
#include <vector>
#include "Vec3.h"
#include "Vec4.h"
#include "Color.h"
#include "Rotation.h"
#include "Scaling.h"
#include "Translation.h"
#include "Camera.h"
#include "Mesh.h"
#include "Helpers.h"

class Scene
{
public:
	Color backgroundColor;
	bool cullingEnabled;

	std::vector<std::vector<Color> > image;
	std::vector<Camera *> cameras;
	std::vector<Vec3 *> vertices;
	std::vector<Color *> colorsOfVertices;
	std::vector<Scaling *> scalings;
	std::vector<Rotation *> rotations;
	std::vector<Translation *> translations;
	std::vector<Mesh *> meshes;

	Scene(const char *xmlPath);

	void initializeImage(Camera *camera);
	int makeBetweenZeroAnd255(double value);
	void writeImageToPPMFile(Camera *camera);
	void convertPPMToPNG(std::string ppmFileName, int osType);
	Matrix4 getModelingTransformationMatrix(Mesh* mesh);
	std::vector<Vec4> getTransformedTriangleVertices(Triangle& triangle, Matrix4& transformationMatrix, std::vector<Vec3 *>& vertices);
	void rasterizeTriangle(std::vector<Vec4>& transformed_vertices, std::vector<Color>& triangleVertexColors, Camera* camera, Matrix4& viewportTransformationMatrix, std::vector<std::vector<double>>& depth);
	void processWireframeMesh(std::vector<Vec4>& transformed_vertices, std::vector<Color>& triangleVertexColors, Camera* camera, Matrix4& viewportTransformationMatrix, std::vector<std::vector<double>>& depth);
	bool visible(double den, double num, double& tEnter, double& tLeave);
	void rasterizeLine(bool clipped, std::vector<Vec4>& vertices, std::vector<Color>& colors, Matrix4& viewportTransformationMatrix, std::vector<std::vector<double>>& depth);
	bool clip_line(std::vector<Vec4>& vertices, std::vector<Color>& colors, Matrix4& viewportTransformationMatrix);
	void forwardRenderingPipeline(Camera *camera);
};

#endif