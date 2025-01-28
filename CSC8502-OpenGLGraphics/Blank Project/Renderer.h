#pragma once
#include "../nclgl/OGLRenderer.h"
#include "../nclgl/Frustum.h"
#include "../nclgl/SceneNode.h"
class Camera;
class Shader;
class HeightMap;
class Mesh;
class MeshAnimation;
class MeshMaterial;
class SceneNode;


class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);
	void RenderScene() override;
	void UpdateScene(float dt) override;
	void ToggleCamera();

protected:
	void BuildNodeLists(SceneNode* from);
	void SortNodeLists();
	void ClearNodeLists();
	void DrawHeightmap();
	void DrawWater();
	void DrawSkybox();
	void DrawShadowScene();
	void DrawCube();
	void DrawNodes();
	void DrawNode(SceneNode* n);
	void RenderMeshMat();
	void PresentScene();
	void DrawPostprocess();
	void CameraTrackProcess();

	Shader* lightShader;
	Shader* reflectShader;
	Shader* skyboxShader;
	Shader* shadowShader;
	Shader* cubeShader;
	Shader* flatTexShader;
	Shader* skelShader;
	Shader* processShader;
	Shader* sceneShader;

	GLuint shadowTex;
	GLuint shadowFBO;
	GLuint texture;
	MeshMaterial* meshMaterial;


	HeightMap* heightMap;
	Mesh* quad;
	Mesh* quad2;

	Light* light;
	Camera* camera;
	Light* lhLight;

	SceneNode* root;
	Frustum frameFrustum;
	vector<SceneNode*> transparentNodeList;
	vector<SceneNode*> nodeList;

	Mesh* test;

	Mesh* fish;
	MeshAnimation* fishAnim;
	MeshMaterial* fishMat;
	vector<GLuint> matTextures;

	Mesh* lHouse;
	MeshMaterial* lHouseMat;

	Mesh* house13;
	MeshMaterial* house13Mat;

	Mesh* house15;
	MeshMaterial* house15Mat;

	Mesh* boat;
	MeshMaterial* boatMat;


	GLuint cubeMap;
	GLuint cubeMapNight;
	GLuint waterTex;
	GLuint earthTex;
	GLuint earthBump;
	GLuint squareTex;
	GLuint squareBump;
	GLuint boatTex;
	GLuint brickWallTex;
	GLuint brickWallBump;

	GLuint bufferFBO;
	GLuint processFBO;
	GLuint bufferColourTex[2];
	GLuint bufferDepthTex;

	float waterRotate;
	float waterCycle;
	float waterBob;
	int currentFrame;
	float frameTime;
	bool cameraTrack;
	float cameraTrackTimer;
};
