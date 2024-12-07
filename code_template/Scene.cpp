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

double X_MAX = 1;
double X_MIN = -1;
double Y_MAX = 1;
double Y_MIN = -1;
double Z_MAX = 1;
double Z_MIN = -1;

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

vector<Vec4> Scene::getTransformedTriangleVertices(Triangle& triangle, Matrix4& transformationMatrix, vector<Vec3 *>& vertices) {
	vector<Vec4> transformed_vertices;

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

/*
	Transformations, clipping, culling, rasterization are done here.
*/
void Scene::forwardRenderingPipeline(Camera *camera)
{
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

			if(mesh->type == WIREFRAME_MESH) {

		}
	}





}
