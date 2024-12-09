#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <cstring>
#include <string>
#include <vector>
#include <cmath>

#include "tinyxml2.h"
#include "Triangle.h"
#include "Helpers.h"
#include "Scene.h"

using namespace tinyxml2;
using namespace std;
#define MAX_DEPTH 99.99


/*
	Parses XML file
*/
Scene::Scene(const char *xmlPath)
{
	const char *str;
	XMLDocument xmlDoc;
	XMLElement *xmlElement;

	xmlDoc.LoadFile(xmlPath);

	XMLNode *rootNode = xmlDoc.FirstChild();

	// read background color
	xmlElement = rootNode->FirstChildElement("BackgroundColor");
	str = xmlElement->GetText();
	sscanf(str, "%lf %lf %lf", &backgroundColor.r, &backgroundColor.g, &backgroundColor.b);

	// read culling
	xmlElement = rootNode->FirstChildElement("Culling");
	if (xmlElement != NULL)
	{
		str = xmlElement->GetText();

		if (strcmp(str, "enabled") == 0)
		{
			this->cullingEnabled = true;
		}
		else
		{
			this->cullingEnabled = false;
		}
	}

	// read cameras
	xmlElement = rootNode->FirstChildElement("Cameras");
	XMLElement *camElement = xmlElement->FirstChildElement("Camera");
	XMLElement *camFieldElement;
	while (camElement != NULL)
	{
		Camera *camera = new Camera();

		camElement->QueryIntAttribute("id", &camera->cameraId);

		// read projection type
		str = camElement->Attribute("type");

		if (strcmp(str, "orthographic") == 0)
		{
			camera->projectionType = ORTOGRAPHIC_PROJECTION;
		}
		else
		{
			camera->projectionType = PERSPECTIVE_PROJECTION;
		}

		camFieldElement = camElement->FirstChildElement("Position");
		str = camFieldElement->GetText();
		sscanf(str, "%lf %lf %lf", &camera->position.x, &camera->position.y, &camera->position.z);

		camFieldElement = camElement->FirstChildElement("Gaze");
		str = camFieldElement->GetText();
		sscanf(str, "%lf %lf %lf", &camera->gaze.x, &camera->gaze.y, &camera->gaze.z);

		camFieldElement = camElement->FirstChildElement("Up");
		str = camFieldElement->GetText();
		sscanf(str, "%lf %lf %lf", &camera->v.x, &camera->v.y, &camera->v.z);

		camera->gaze = normalizeVec3(camera->gaze);
		camera->u = crossProductVec3(camera->gaze, camera->v);
		camera->u = normalizeVec3(camera->u);

		camera->w = inverseVec3(camera->gaze);
		camera->v = crossProductVec3(camera->u, camera->gaze);
		camera->v = normalizeVec3(camera->v);

		camFieldElement = camElement->FirstChildElement("ImagePlane");
		str = camFieldElement->GetText();
		sscanf(str, "%lf %lf %lf %lf %lf %lf %d %d",
			   &camera->left, &camera->right, &camera->bottom, &camera->top,
			   &camera->near, &camera->far, &camera->horRes, &camera->verRes);

		camFieldElement = camElement->FirstChildElement("OutputName");
		str = camFieldElement->GetText();
		camera->outputFilename = string(str);

		this->cameras.push_back(camera);

		camElement = camElement->NextSiblingElement("Camera");
	}

	// read vertices
	xmlElement = rootNode->FirstChildElement("Vertices");
	XMLElement *vertexElement = xmlElement->FirstChildElement("Vertex");
	int vertexId = 1;

	while (vertexElement != NULL)
	{
		Vec3 *vertex = new Vec3();
		Color *color = new Color();

		vertex->colorId = vertexId;

		str = vertexElement->Attribute("position");
		sscanf(str, "%lf %lf %lf", &vertex->x, &vertex->y, &vertex->z);

		str = vertexElement->Attribute("color");
		sscanf(str, "%lf %lf %lf", &color->r, &color->g, &color->b);

		this->vertices.push_back(vertex);
		this->colorsOfVertices.push_back(color);

		vertexElement = vertexElement->NextSiblingElement("Vertex");

		vertexId++;
	}

	// read translations
	xmlElement = rootNode->FirstChildElement("Translations");
	XMLElement *translationElement = xmlElement->FirstChildElement("Translation");
	while (translationElement != NULL)
	{
		Translation *translation = new Translation();

		translationElement->QueryIntAttribute("id", &translation->translationId);

		str = translationElement->Attribute("value");
		sscanf(str, "%lf %lf %lf", &translation->tx, &translation->ty, &translation->tz);

		this->translations.push_back(translation);

		translationElement = translationElement->NextSiblingElement("Translation");
	}

	// read scalings
	xmlElement = rootNode->FirstChildElement("Scalings");
	XMLElement *scalingElement = xmlElement->FirstChildElement("Scaling");
	while (scalingElement != NULL)
	{
		Scaling *scaling = new Scaling();

		scalingElement->QueryIntAttribute("id", &scaling->scalingId);
		str = scalingElement->Attribute("value");
		sscanf(str, "%lf %lf %lf", &scaling->sx, &scaling->sy, &scaling->sz);

		this->scalings.push_back(scaling);

		scalingElement = scalingElement->NextSiblingElement("Scaling");
	}

	// read rotations
	xmlElement = rootNode->FirstChildElement("Rotations");
	XMLElement *rotationElement = xmlElement->FirstChildElement("Rotation");
	while (rotationElement != NULL)
	{
		Rotation *rotation = new Rotation();

		rotationElement->QueryIntAttribute("id", &rotation->rotationId);
		str = rotationElement->Attribute("value");
		sscanf(str, "%lf %lf %lf %lf", &rotation->angle, &rotation->ux, &rotation->uy, &rotation->uz);

		this->rotations.push_back(rotation);

		rotationElement = rotationElement->NextSiblingElement("Rotation");
	}

	// read meshes
	xmlElement = rootNode->FirstChildElement("Meshes");

	XMLElement *meshElement = xmlElement->FirstChildElement("Mesh");
	while (meshElement != NULL)
	{
		Mesh *mesh = new Mesh();

		meshElement->QueryIntAttribute("id", &mesh->meshId);

		// read projection type
		str = meshElement->Attribute("type");

		if (strcmp(str, "wireframe") == 0)
		{
			mesh->type = WIREFRAME_MESH;
		}
		else
		{
			mesh->type = SOLID_MESH;
		}

		// read mesh transformations
		XMLElement *meshTransformationsElement = meshElement->FirstChildElement("Transformations");
		XMLElement *meshTransformationElement = meshTransformationsElement->FirstChildElement("Transformation");

		while (meshTransformationElement != NULL)
		{
			char transformationType;
			int transformationId;

			str = meshTransformationElement->GetText();
			sscanf(str, "%c %d", &transformationType, &transformationId);

			mesh->transformationTypes.push_back(transformationType);
			mesh->transformationIds.push_back(transformationId);

			meshTransformationElement = meshTransformationElement->NextSiblingElement("Transformation");
		}

		mesh->numberOfTransformations = mesh->transformationIds.size();

		// read mesh faces
		char *row;
		char *cloneStr;
		int v1, v2, v3;
		XMLElement *meshFacesElement = meshElement->FirstChildElement("Faces");
		str = meshFacesElement->GetText();
		cloneStr = strdup(str);

		row = strtok(cloneStr, "\n");
		while (row != NULL)
		{
			int result = sscanf(row, "%d %d %d", &v1, &v2, &v3);

			if (result != EOF)
			{
				mesh->triangles.push_back(Triangle(v1, v2, v3));
			}
			row = strtok(NULL, "\n");
		}
		mesh->numberOfTriangles = mesh->triangles.size();
		this->meshes.push_back(mesh);

		meshElement = meshElement->NextSiblingElement("Mesh");
	}
}

/*
	Initializes image with background color
*/
void Scene::initializeImage(Camera *camera)
{
	if (this->image.empty())
	{
		for (int i = 0; i < camera->horRes; i++)
		{
			vector<Color> rowOfColors;

			for (int j = 0; j < camera->verRes; j++)
			{
				rowOfColors.push_back(this->backgroundColor);
			}

			this->image.push_back(rowOfColors);
		}
	}
	else
	{
		for (int i = 0; i < camera->horRes; i++)
		{
			for (int j = 0; j < camera->verRes; j++)
			{
				this->image[i][j].r = this->backgroundColor.r;
				this->image[i][j].g = this->backgroundColor.g;
				this->image[i][j].b = this->backgroundColor.b;
			}
		}
	}
}

/*
	If given value is less than 0, converts value to 0.
	If given value is more than 255, converts value to 255.
	Otherwise returns value itself.
*/
int Scene::makeBetweenZeroAnd255(double value)
{
	if (value >= 255.0)
		return 255;
	if (value <= 0.0)
		return 0;
	return (int)(value);
}


/*
	Writes contents of image (Color**) into a PPM file.
*/
void Scene::writeImageToPPMFile(Camera *camera)
{
	ofstream fout;

	fout.open(camera->outputFilename.c_str());

	fout << "P3" << endl;
	fout << "# " << camera->outputFilename << endl;
	fout << camera->horRes << " " << camera->verRes << endl;
	fout << "255" << endl;

	for (int j = camera->verRes - 1; j >= 0; j--)
	{
		for (int i = 0; i < camera->horRes; i++)
		{
			fout << makeBetweenZeroAnd255(this->image[i][j].r) << " "
				 << makeBetweenZeroAnd255(this->image[i][j].g) << " "
				 << makeBetweenZeroAnd255(this->image[i][j].b) << " ";
		}
		fout << endl;
	}
	fout.close();
}

/*
	Converts PPM image in given path to PNG file, by calling ImageMagick's 'convert' command.
	os_type == 1 		-> Ubuntu
	os_type == 2 		-> Windows
	os_type == other	-> No conversion
*/
void Scene::convertPPMToPNG(string ppmFileName, int osType)
{
	string command;

	// call command on Ubuntu
	if (osType == 1)
	{
		command = "./magick " + ppmFileName + " " + ppmFileName + ".png";
		system(command.c_str());
	}

	// call command on Windows
	else if (osType == 2)
	{
		command = "magick " + ppmFileName + " " + ppmFileName + ".png";
		system(command.c_str());
	}

	// default action - don't do conversion
	else
	{
	}
}

Matrix4 Scene::getModelingTransformationMatrix(Mesh* mesh) {
	Matrix4 modelingTransformationMatrix = getIdentityMatrix();

	for(int i = 0; i < mesh->numberOfTransformations; i++) {
		int transformation_id = mesh->transformationIds[i];
		char transformation_type = mesh->transformationTypes[i];
		if(transformation_type == 't') {
			Translation* translation = this->translations[transformation_id-1];
			modelingTransformationMatrix = translation->doTranslation(modelingTransformationMatrix);
		}
		else if(transformation_type == 's') {
			Scaling* scaling = this->scalings[transformation_id-1];
			modelingTransformationMatrix = scaling->doScaling(modelingTransformationMatrix);
		}
		else if(transformation_type == 'r') {
			Rotation* rotation = this->rotations[transformation_id-1];
			modelingTransformationMatrix = rotation->doRotation(modelingTransformationMatrix);
		}
	}
	return modelingTransformationMatrix;
}

std::vector<Vec4> Scene::getTransformedTriangleVertices(Triangle& triangle, Matrix4& transformationMatrix, std::vector<Vec3 *>& vertices) {
	std::vector<Vec4> transformed_vertices;

	Vec3 vertex_0 = *vertices[triangle.vertexIds[0] - 1];
	Vec3 vertex_1 = *vertices[triangle.vertexIds[1] - 1];
	Vec3 vertex_2 = *vertices[triangle.vertexIds[2] - 1];

	Vec4 transformed_vertex_0 = multiplyMatrixWithVec4(transformationMatrix, Vec4(vertex_0.x, vertex_0.y, vertex_0.z, 1));
	Vec4 transformed_vertex_1 = multiplyMatrixWithVec4(transformationMatrix, Vec4(vertex_1.x, vertex_1.y, vertex_1.z, 1));
	Vec4 transformed_vertex_2 = multiplyMatrixWithVec4(transformationMatrix, Vec4(vertex_2.x, vertex_2.y, vertex_2.z, 1));


	transformed_vertex_0.x /= transformed_vertex_0.t;
	transformed_vertex_0.y /= transformed_vertex_0.t;
	transformed_vertex_0.z /= transformed_vertex_0.t;
	transformed_vertex_0.t = 1;

	transformed_vertex_1.x /= transformed_vertex_1.t;
	transformed_vertex_1.y /= transformed_vertex_1.t;
	transformed_vertex_1.z /= transformed_vertex_1.t;
	transformed_vertex_1.t = 1;

	transformed_vertex_2.x /= transformed_vertex_2.t;
	transformed_vertex_2.y /= transformed_vertex_2.t;
	transformed_vertex_2.z /= transformed_vertex_2.t;
	transformed_vertex_2.t = 1;

	transformed_vertices.push_back(transformed_vertex_0);
	transformed_vertices.push_back(transformed_vertex_1);
	transformed_vertices.push_back(transformed_vertex_2);

	return transformed_vertices;
}

double f_xy(double x, double y, double x0, double y0, double x1, double y1) {
	return x * (y0 - y1) + y * (x1 - x0) + (x0 * y1 - y0 * x1);
}

void Scene::rasterizeTriangle(std::vector<Vec4>& transformed_vertices, std::vector<Color>& triangleVertexColors, Camera* camera, Matrix4& viewportTransformationMatrix, std::vector<std::vector<double>>& depth) {
	for (int i = 0; i < transformed_vertices.size(); i++) {
		transformed_vertices[i] = multiplyMatrixWithVec4(viewportTransformationMatrix, transformed_vertices[i]);
	}
	
	double x_min = min(transformed_vertices[0].x, min(transformed_vertices[1].x, transformed_vertices[2].x));
	double x_max = max(transformed_vertices[0].x, max(transformed_vertices[1].x, transformed_vertices[2].x));
	double y_min = min(transformed_vertices[0].y, min(transformed_vertices[1].y, transformed_vertices[2].y));
	double y_max = max(transformed_vertices[0].y, max(transformed_vertices[1].y, transformed_vertices[2].y));

	// Clamp coordinates to valid screen space
	x_min = max(0.0, ceil(x_min));
	x_max = min(camera->horRes - 1.0, floor(x_max));
	y_min = max(0.0, ceil(y_min));
	y_max = min(camera->verRes - 1.0, floor(y_max));

	// Early exit if triangle is completely outside screen
	if (x_min > x_max || y_min > y_max) {
		return;
	}

	Color color1, color2, color3, color;
	color1 = triangleVertexColors[0];
	color2 = triangleVertexColors[1];
	color3 = triangleVertexColors[2];

	double f01_2 = f_xy(transformed_vertices[2].x, transformed_vertices[2].y, transformed_vertices[0].x, transformed_vertices[0].y, transformed_vertices[1].x, transformed_vertices[1].y);
	double f12_0 = f_xy(transformed_vertices[0].x, transformed_vertices[0].y, transformed_vertices[1].x, transformed_vertices[1].y, transformed_vertices[2].x, transformed_vertices[2].y);
	double f20_1 = f_xy(transformed_vertices[1].x, transformed_vertices[1].y, transformed_vertices[2].x, transformed_vertices[2].y, transformed_vertices[0].x, transformed_vertices[0].y);
	
	// Check if triangle is degenerate
	if (abs(f01_2) < 1e-6 || abs(f12_0) < 1e-6 || abs(f20_1) < 1e-6) {
		return;
	}

	double alpha, beta, gamma;
	for(int y = y_min; y <= y_max; y++) {
		for(int x = x_min; x <= x_max; x++) {
				alpha = f_xy(x, y, transformed_vertices[1].x, transformed_vertices[1].y, transformed_vertices[2].x, transformed_vertices[2].y) / f12_0;
				beta = f_xy(x, y, transformed_vertices[2].x, transformed_vertices[2].y, transformed_vertices[0].x, transformed_vertices[0].y) / f20_1;
				gamma = f_xy(x, y, transformed_vertices[0].x, transformed_vertices[0].y, transformed_vertices[1].x, transformed_vertices[1].y) / f01_2;

				if(alpha >= 0 && beta >= 0 && gamma >= 0) {
					double z_value = alpha * transformed_vertices[0].z + beta * transformed_vertices[1].z + gamma * transformed_vertices[2].z;
					if(z_value < depth[x][y]) {
						depth[x][y] = z_value;
						color = Color(alpha * color1.r + beta * color2.r + gamma * color3.r, 
									alpha * color1.g + beta * color2.g + gamma * color3.g, 
									alpha * color1.b + beta * color2.b + gamma * color3.b);
						this->image[x][y] = color;
					}
				}
		}
	}
}

bool Scene::visible(double den, double num, double& tEnter, double& tLeave) {
	if(den > 0) { // potentially entering
		double t = num / den;
		if(t > tLeave) return false;
		if(t > tEnter) tEnter = t;
	}
	else if(den < 0) { // potentially leaving
		double t = num / den;
		if(t < tEnter) return false;
		if(t < tLeave) tLeave = t;
	}
	else if(num > 0) return false; // parallel
	return true;
}

bool Scene::clip_line(std::vector<Vec4>& vertices, std::vector<Color>& colors, Matrix4& viewportTransformationMatrix) {
	double tEnter = 0.0;
	double tLeave = 1.0;

	Vec4 v0 = vertices[0];
	Vec4 v1 = vertices[1];

	Color color_v0 = colors[0];
	Color color_v1 = colors[1];

	double dx = v1.x - v0.x;
	double dy = v1.y - v0.y;
	double dz = v1.z - v0.z;

	bool is_visible = false;
	if(visible(dx, (-1 - v0.x), tEnter, tLeave)) { // left 
		if(visible(-dx, (v0.x - 1), tEnter, tLeave)) { // right
			if(visible(dy, (-1 - v0.y), tEnter, tLeave)) { // bottom
				if(visible(-dy, (v0.y - 1), tEnter, tLeave)) { // top
					if(visible(dz, (-1 - v0.z), tEnter, tLeave)) { // back
						if(visible(-dz, (v0.z - 1), tEnter, tLeave)) { // front
							is_visible = true;
							if(tLeave < 1) {
								v1.x = v0.x + dx*tLeave;
								v1.y = v0.y + dy*tLeave;
								v1.z = v0.z + dz*tLeave;

								colors[1] = Color(color_v0.r + (color_v1.r - color_v0.r) * tLeave, 
													color_v0.g + (color_v1.g - color_v0.g) * tLeave, 
													color_v0.b + (color_v1.b - color_v0.b) * tLeave);
							}
							if(tEnter > 0) {
								v0.x = v0.x + dx*tEnter;
								v0.y = v0.y + dy*tEnter;
								v0.z = v0.z + dz*tEnter;

								colors[0] = Color(color_v0.r + (color_v1.r - color_v0.r) * tEnter, 
													color_v0.g + (color_v1.g - color_v0.g) * tEnter, 
													color_v0.b + (color_v1.b - color_v0.b) * tEnter);
							}
						}
					}
				}
			}
		}
	}

	return is_visible;
}

void Scene::rasterizeLine(bool clipped, std::vector<Vec4>& vertices, std::vector<Color>& colors, Matrix4& viewportTransformationMatrix, std::vector<std::vector<double>>& depth) {
	if(!clipped) return;
	
	// Apply viewport transformation after clipping
	vertices[0] = multiplyMatrixWithVec4(viewportTransformationMatrix, vertices[0]);
	vertices[1] = multiplyMatrixWithVec4(viewportTransformationMatrix, vertices[1]);

	int x0 = round(vertices[0].x);
	int y0 = round(vertices[0].y);
	int x1 = round(vertices[1].x);
	int y1 = round(vertices[1].y);

	int dx = abs(x1 - x0);
	int dy = abs(y1 - y0);
	double m = dx == 0 ? INFINITY : (double)(y1 - y0) / (x1 - x0);

	if(abs(m) <= 1) {
		// Handle horizontal-ish lines
		if(x0 > x1) {
			std::swap(x0, x1);
			std::swap(y0, y1);
			std::swap(colors[0], colors[1]);
			std::swap(vertices[0].z, vertices[1].z);
		}
		
		int y = y0;
		int d = 2*dy - dx;
		int increment = (y1 > y0) ? 1 : -1;
		
		for(int x = x0; x <= x1; x++) {
			if(x >= 0 && x < depth.size() && y >= 0 && y < depth[0].size()) {
				double t = dx == 0 ? 0 : (double)(x - x0) / dx;
				double z = vertices[0].z * (1-t) + vertices[1].z * t;
				
				if(z < depth[x][y]) {
					depth[x][y] = z;
					Color color = Color(
						colors[0].r * (1-t) + colors[1].r * t,
						colors[0].g * (1-t) + colors[1].g * t,
						colors[0].b * (1-t) + colors[1].b * t
					);
					this->image[x][y] = color;
				}
			}
			
			if(d < 0) {
				d += 2*dy;
			} else {
				y += increment;
				d += 2*(dy - dx);
			}
		}
	} else {
		// Handle vertical-ish lines
		if(y0 > y1) {
			std::swap(x0, x1);
			std::swap(y0, y1);
			std::swap(colors[0], colors[1]);
			std::swap(vertices[0].z, vertices[1].z);
		}
		
		int x = x0;
		int d = 2*dx - dy;
		int increment = (x1 > x0) ? 1 : -1;
		
		for(int y = y0; y <= y1; y++) {
			if(x >= 0 && x < depth.size() && y >= 0 && y < depth[0].size()) {
				double t = dy == 0 ? 0 : (double)(y - y0) / dy;
				double z = vertices[0].z * (1-t) + vertices[1].z * t;
				
				if(z < depth[x][y]) {
					depth[x][y] = z;
					Color color = Color(
						colors[0].r * (1-t) + colors[1].r * t,
						colors[0].g * (1-t) + colors[1].g * t,
						colors[0].b * (1-t) + colors[1].b * t
					);
					this->image[x][y] = color;
				}
			}
			
			if(d < 0) {
				d += 2*dx;
			} else {
				x += increment;
				d += 2*(dx - dy);
			}
		}
	}
}

void Scene::processWireframeMesh(std::vector<Vec4>& transformed_vertices, std::vector<Color>& triangleVertexColors, Camera* camera, Matrix4& viewportTransformationMatrix, std::vector<std::vector<double>>& depth) {
	bool clipped_line0_v0_to_v1;
	bool clipped_line1_v1_to_v2;
	bool clipped_line2_v2_to_v0;
	
	// first line to clip -> v0 to v1
	std::vector<Vec4> line0_vertices = copyVec4(transformed_vertices, 0, 1);
	std::vector<Color> line0_colors = copyColor(triangleVertexColors, 0, 1);
	clipped_line0_v0_to_v1 = clip_line(line0_vertices, line0_colors, viewportTransformationMatrix);

	// second line to clip -> v1 to v2
	std::vector<Vec4> line1_vertices = copyVec4(transformed_vertices, 1, 2);
	std::vector<Color> line1_colors = copyColor(triangleVertexColors, 1, 2);
	clipped_line1_v1_to_v2 = clip_line(line1_vertices, line1_colors, viewportTransformationMatrix);

	// third line to clip -> v2 to v0
	std::vector<Vec4> line2_vertices = copyVec4(transformed_vertices, 2, 0);
	std::vector<Color> line2_colors = copyColor(triangleVertexColors, 2, 0);
	clipped_line2_v2_to_v0 = clip_line(line2_vertices, line2_colors, viewportTransformationMatrix);

	rasterizeLine(clipped_line0_v0_to_v1, line0_vertices, line0_colors, viewportTransformationMatrix, depth);
	rasterizeLine(clipped_line1_v1_to_v2, line1_vertices, line1_colors, viewportTransformationMatrix, depth);
	rasterizeLine(clipped_line2_v2_to_v0, line2_vertices, line2_colors, viewportTransformationMatrix, depth);
}

/*
	T"r"ansformations, clipping, culling, rasterization are done here.
*/
void Scene::forwardRenderingPipeline(Camera *camera)
{
	std::vector<std::vector<double>> depth(camera->horRes, std::vector<double>(camera->verRes, MAX_DEPTH));
	// ***** Viewing Transformation ***** //

	// Camera Transformation
	Matrix4 cameraTransformationMatrix = camera->getCameraTransformationMatrix();

	// Projection Transformation
	Matrix4 projectionTransformationMatrix = camera->getProjectionTransformationMatrix();

	// Viewport Transformation
	Matrix4 viewportTransformationMatrix = camera->getViewportTransformationMatrix();

	// ***** End of Viewing Transformation ***** //

	Matrix4 modelingTransformationMatrix = getIdentityMatrix();
	for(Mesh* mesh : this->meshes) {
		modelingTransformationMatrix = getModelingTransformationMatrix(mesh);

		// compose transformation matrix
		Matrix4 transformationMatrix = multiplyMatrixWithMatrix(cameraTransformationMatrix, modelingTransformationMatrix);
		transformationMatrix = multiplyMatrixWithMatrix(projectionTransformationMatrix, transformationMatrix);

		for(Triangle& triangle : mesh->triangles) {
			vector<Vec4> transformed_vertices = getTransformedTriangleVertices(triangle, transformationMatrix, this->vertices);

			// Backface Culling
			if(this->cullingEnabled) {
				Vec3 vertex_0 = Vec3(transformed_vertices[0].x, transformed_vertices[0].y, transformed_vertices[0].z);
				Vec3 vertex_1 = Vec3(transformed_vertices[1].x, transformed_vertices[1].y, transformed_vertices[1].z);
				Vec3 vertex_2 = Vec3(transformed_vertices[2].x, transformed_vertices[2].y, transformed_vertices[2].z);

				Vec3 v1_minus_v0 = subtractVec3(vertex_1, vertex_0);
				Vec3 v2_minus_v0 = subtractVec3(vertex_2, vertex_0);

				Vec3 normal = normalizeVec3(crossProductVec3(v1_minus_v0, v2_minus_v0));
				if(dotProductVec3(normal, vertex_0) < 0) continue;
			}

			vector<Color> triangleVertexColors;
			for(int i = 0; i < 3; i++) {
				triangleVertexColors.push_back(*this->colorsOfVertices[triangle.vertexIds[i] - 1]);
			}

			if (mesh->type == SOLID_MESH){
				rasterizeTriangle(transformed_vertices, triangleVertexColors, camera, viewportTransformationMatrix, depth);
			}
			else if (mesh->type == WIREFRAME_MESH) {
				processWireframeMesh(transformed_vertices, triangleVertexColors, camera, viewportTransformationMatrix, depth);
			}
		}
	}

}
