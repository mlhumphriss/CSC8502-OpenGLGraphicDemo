#include "Renderer.h"
#include "../nclgl/Light.h"
#include "../nclgl/HeightMap.h"
#include "../nclgl/Shader.h"
#include "../nclgl/Camera.h"
#include "../nclgl/MeshAnimation.h"
#include "../nclgl/MeshMaterial.h"
#include <algorithm>

#define SHADOWSIZE 8192
const int POST_PASSES = 2;

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	
	reflectShader = new Shader("reflectVertex.glsl", "reflectFragment.glsl");

	skyboxShader = new Shader("skyboxVertex.glsl", "skyboxFragment.glsl");

	lightShader = new Shader("shadowscenevert.glsl", "shadowscenefrag.glsl");

	shadowShader = new Shader("shadowVert.glsl", "shadowFrag.glsl");

	cubeShader = new Shader("TexturedVertex.glsl", "TexturedFragment.glsl");

	flatTexShader = new Shader("noBumpShadowSceneVert.glsl", "noBumpShadowSceneFrag.glsl");

	skelShader = new Shader("SkinningVertex.glsl", "TexturedFragment.glsl");

	processShader = new Shader("TexVertex.glsl", "shadowProcessFrag.glsl");

	sceneShader = new Shader("TexturedVertex.glsl", "TexturedFragment.glsl");

	if (!reflectShader->LoadSuccess() || !skyboxShader->LoadSuccess() || !lightShader->LoadSuccess() || !shadowShader->LoadSuccess() || !cubeShader->LoadSuccess()) {
		return;
	}
	//Shadow buffer code
	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOWSIZE, SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_2D, 0);
	
	glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTex, 0);

	glDrawBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	/* Post Processing buffers */
	glGenTextures(1, &bufferDepthTex);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

	for (int i = 0; i < 2; ++i) {
		glGenTextures(1, &bufferColourTex[i]);
		glBindTexture(GL_TEXTURE_2D, bufferColourTex[i]);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}

	glGenFramebuffers(1, &bufferFBO);
	glGenFramebuffers(1, &processFBO);

	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[0], 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE || !bufferDepthTex || !bufferColourTex[0]) {
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//*/
	root = new SceneNode();

	quad = Mesh::GenerateQuad();
	quad2 = Mesh::GenerateQuad();

	test = Mesh::LoadFromMeshFile("Cube.msh");

	fish = Mesh::LoadFromMeshFile("Role_T.msh");
	fishAnim = new MeshAnimation("Role_T.anm");
	fishMat = new MeshMaterial("Role_T.mat");

	
	currentFrame = 0;
	frameTime = 0.0f;

	boat = Mesh::LoadFromMeshFile("boat_v3.msh");
	boatMat = new MeshMaterial("boat_v4.mat");

	lHouse = Mesh::LoadFromMeshFile("LightHouse.msh");
	lHouseMat = new MeshMaterial("LightHouse.mat");


	house13 = Mesh::LoadFromMeshFile("013Smallhouse1f52m2.msh");
	house13Mat = new MeshMaterial("013Smallhouse1f52m2.mat");

	house15 = Mesh::LoadFromMeshFile("015Smallhouse1f48m2.msh");
	house15Mat = new MeshMaterial("015Smallhouse1f48m2.mat");

	heightMap = new HeightMap(TEXTUREDIR"noise2.png");

	waterTex = SOIL_load_OGL_texture(TEXTUREDIR"water.TGA", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	earthTex = SOIL_load_OGL_texture(TEXTUREDIR"Rock3.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	earthBump = SOIL_load_OGL_texture(TEXTUREDIR"Rock3Bump.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	brickWallTex = SOIL_load_OGL_texture(TEXTUREDIR"Brick_Wall_DIFF.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	brickWallBump = SOIL_load_OGL_texture(TEXTUREDIR"Brick_Wall_DISP.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	boatTex = SOIL_load_OGL_texture(TEXTUREDIR"Planks_01_DIFF.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	cubeMap = SOIL_load_OGL_cubemap(TEXTUREDIR"rusted_west.jpg", TEXTUREDIR"rusted_east.jpg",
		TEXTUREDIR"rusted_up.jpg", TEXTUREDIR"rusted_down.jpg", TEXTUREDIR"rusted_south.jpg",
		TEXTUREDIR"rusted_north.jpg", SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	cubeMapNight = SOIL_load_OGL_cubemap(TEXTUREDIR"rusted_westNight.jpg", TEXTUREDIR"rusted_eastNight.jpg",
		TEXTUREDIR"rusted_upNight.jpg", TEXTUREDIR"rusted_downNight.jpg", TEXTUREDIR"rusted_southNight.jpg",
		TEXTUREDIR"rusted_northNight.jpg", SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	if (!earthTex || !earthBump || !cubeMap || !waterTex || !boatTex || !brickWallTex || !brickWallBump) {
		return;
	}

	setTextureRepeating(earthTex, true);
	setTextureRepeating(earthBump, true);
	setTextureRepeating(waterTex, true);
	setTextureRepeating(boatTex, true);
	setTextureRepeating(brickWallTex, true);
	setTextureRepeating(brickWallBump, true);
	

	Vector3 heightmapSize = heightMap->GetHeightmapSize();

	camera = new Camera(-45.0f, 0.0f, heightmapSize * Vector3(0.5f, 5.0f, 0.5f));

	light = new Light(heightmapSize * Vector3(2.0f, 10.0f, -1.0f), Vector4(1, 1, 1, 1), 4.0f * heightmapSize.x);



	projMatrix = Matrix4::Perspective(0.1f, 12000.0f, (float)width / (float)height, 45.0f);

	for (int i = 0; i < 5; ++i) {
		SceneNode* b = new SceneNode();
		b->SetTransform(Matrix4::Translation(heightmapSize * Vector3(0.687f + 0.02 *i, 0.5f, 0.4f + 0.03 * i)));
		b->SetModelScale(Vector3(50.0f, 50.0f, 50.0f));
		b->SetBoundingRadius(300.0f);
		b->SetMesh(boat);
		b->SetTexture(boatTex);
		root->AddChild(b);
	}

	SceneNode* l = new SceneNode();
	l->SetTransform(Matrix4::Translation(heightmapSize* Vector3(0.03f, 0.9f, 0.8f)));
	l->SetModelScale(Vector3(15.0f, 15.0f, 15.0f));
	l->SetBoundingRadius(800.0f);
	l->SetMesh(lHouse);
	l->SetTexture(brickWallTex);
	root->AddChild(l);

	
	for (int i = 0; i < 3; ++i){
		SceneNode* h = new SceneNode();
		float mod = 0.0f;
		if (i % 2 != 0) {
			mod = 0.04;
		}
		h->SetTransform(Matrix4::Translation(heightmapSize * Vector3(0.9f-0.1*i + (mod/2), 0.95f, 0.8f + 0.06 * i + mod)) * Matrix4::Rotation(-45.0f * i, Vector3(0, 1, 0)));
		h->SetModelScale(Vector3(2.0f, 2.0f, 2.0f));
		h->SetBoundingRadius(400.0f);
		h->SetMesh(house13);
		h->SetTexture(boatTex);
		root->AddChild(h);
	}

	for (int i = 0; i < 2; ++i) {
		SceneNode* f = new SceneNode();
		f->SetTransform(Matrix4::Translation(heightmapSize * Vector3(0.55f+0.06*i, 0.87f, 0.4f+ 0.1*i))* Matrix4::Rotation(190.0f, Vector3(0, 1, 0)));
		f->SetModelScale(Vector3(2.0f, 2.0f, 2.0f));
		f->SetBoundingRadius(700.0f);
		f->SetMesh(house15);
		f->SetTexture(boatTex);
		root->AddChild(f);
	}


	
	

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	cameraTrack = false;
	waterRotate = 0.0f;
	waterCycle = 0.0f;
	waterBob = 0.0f;
	init = true;
}
Renderer::~Renderer(void) {
	glDeleteTextures(1, &shadowTex);
	glDeleteFramebuffers(1, &shadowFBO);
	/**/
	glDeleteTextures(2, bufferColourTex);
	glDeleteTextures(1, &bufferDepthTex);
	glDeleteFramebuffers(1, &bufferFBO);
	glDeleteFramebuffers(1, &processFBO);
	//*/

	delete camera;
	delete heightMap;
	delete quad;
	delete quad2;
	delete test;
	delete reflectShader;
	delete skyboxShader;
	delete lightShader;
	delete shadowShader;
	delete cubeShader;
	delete flatTexShader;
	delete skelShader;
	delete sceneShader;
	delete processShader;

	delete light;

	delete fish;
	delete fishAnim;
	delete fishMat;
	delete boat;
	delete boatMat;
	delete lHouse;
	delete lHouseMat;
	delete house13;
	delete house13Mat;

	delete root;

}
void Renderer::ToggleCamera() {
	cameraTrack = !cameraTrack;
	
	if (cameraTrack == true) {
		camera->setPosition(heightMap->GetHeightmapSize() * Vector3(0.9f, 1.1f, 0.2f));
		camera->SetYaw(180.0f);
		camera->SetPitch(0.0f);
	}
	cameraTrackTimer = 0.0f;
}
void Renderer::CameraTrackProcess() {
	Vector3 pos = camera->GetPosition();
	Vector3 heightmapSize = heightMap->GetHeightmapSize();

	float direction = camera->GetYaw();
	direction -= 0.05f;
	camera->SetYaw(direction);
	cameraTrackTimer += 0.05f;

	if (pos.z < (0.8f * heightmapSize.z)) {
		pos.z = pos.z + 0.5f;
		camera->setPosition(pos);

		pos.x = pos.x - 0.05f;
		camera->setPosition(pos);
	}
	else if (pos.x > (0.2f * heightmapSize.x)) {
		pos.x = pos.x - 0.5f;
		camera->setPosition(pos);
		pos.y = pos.y + 0.05f;
		camera->setPosition(pos);
	}
	else if(cameraTrackTimer > 600.0f){
		cameraTrack = false;
	}
}
void Renderer::UpdateScene(float dt) {
	if (cameraTrack == true) {
		CameraTrackProcess();
	}
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();


	frameFrustum.FromMatrix(projMatrix * viewMatrix);
	waterRotate += dt * 0.2f;
	waterCycle += dt * 0.05f;
	waterBob = heightMap->GetHeightmapSize().y* 0.001f  * sin(5*waterCycle);
	
	root->Update(dt);
	frameTime -= dt;
	while (frameTime < 0.0f) {
		currentFrame = (currentFrame + 1) % fishAnim->GetFrameCount();
		frameTime += 1.0f / fishAnim->GetFrameRate();
	}
}
void Renderer::BuildNodeLists(SceneNode* from) {
	bool renderCheck = true;
	if (frameFrustum.InsideFrustum(*from)) {
		Vector3 dir = from->GetWorldTransform().GetPositionVector() - camera->GetPosition();
		from->SetCameraDistance(Vector3::Dot(dir, dir));

		if (from->GetColour().w < 1.0f) {
			transparentNodeList.push_back(from);
		}
		else {
			nodeList.push_back(from);
		}
	}
	for (vector<SceneNode*>::const_iterator i = from->GetChildIteratorStart(); i != from->GetChildIteratorEnd(); ++i) {
		BuildNodeLists((*i));
	}
}
void Renderer::SortNodeLists() {
	std::sort(transparentNodeList.rbegin(), transparentNodeList.rend(), SceneNode::CompareByCameraDistance);
	std::sort(nodeList.rbegin(), nodeList.rend(), SceneNode::CompareByCameraDistance);
}
void Renderer::RenderScene() {
	BuildNodeLists(root);
	SortNodeLists();

	//Commented out code in this section is for a post processing method which I have bugs in currently

	//glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT /* | GL_STENCIL_BUFFER_BIT/**/);

	

	DrawSkybox();
	DrawHeightmap();
	DrawWater();
	DrawNodes();
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

	DrawShadowScene();
	viewMatrix = camera->BuildViewMatrix();
	projMatrix = Matrix4::Perspective(0.1f, 12000.0f, (float)width / (float)height, 45.0f);
	
	/*
	DrawPostprocess();
	PresentScene();
	//*/
	ClearNodeLists();
}
void Renderer::DrawNodes() {
	for (const auto& i : nodeList) {
		DrawNode(i);
	}
	for (const auto& i : transparentNodeList) {
		DrawNode(i);
	}
}
void Renderer::DrawShadowScene() {
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);

	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	BindShader(shadowShader);
	viewMatrix = Matrix4::BuildViewMatrix(light->GetPosition(), Vector3(0.2f, 0.1f, 0.2f));

	projMatrix = Matrix4::Perspective(2000.0f, 12000.0f, (float)width / (float)height, 45.0f);
	shadowMatrix = projMatrix * viewMatrix;
	

	for (const auto& i : nodeList) {
		modelMatrix = i->GetWorldTransform() * Matrix4::Scale(i->GetModelScale());
		UpdateShaderMatrices();
		DrawNode(i);
	}

	modelMatrix.ToIdentity();
	UpdateShaderMatrices();
	heightMap->Draw();
	

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glViewport(0, 0, width, height);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void Renderer::DrawNode(SceneNode* n) {
	if (n->GetMesh()) {
		Matrix4 model;

		if (n->GetMesh() == boat) {
			 model = n->GetWorldTransform() * Matrix4::Translation(Vector3(1.0f, 50.0f * waterBob, 1.0f)) * Matrix4::Scale(n->GetModelScale());
		}
		else {
			 model = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
		}
		BindShader(flatTexShader);
		SetShaderLight(*light);

		glUniform1i(glGetUniformLocation(flatTexShader->GetProgram(), "diffuseTex"), 0);
		texture = n->GetTexture();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		glUniform1i(glGetUniformLocation(flatTexShader->GetProgram(), "shadowTex"), 2);

		glUniform3fv(glGetUniformLocation(flatTexShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, shadowTex);

		modelMatrix = model;
		UpdateShaderMatrices();
		n->Draw(*this);
	}
}
void Renderer::DrawSkybox() {
	glDepthMask(GL_FALSE);

	BindShader(skyboxShader);
	UpdateShaderMatrices();

	quad->Draw();

	glDepthMask(GL_TRUE);
}
void Renderer::DrawHeightmap() {
	BindShader(lightShader);
	SetShaderLight(*light);

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, earthTex);

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, earthBump);

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "shadowTex"), 2);

	glUniform3fv(glGetUniformLocation(lightShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	
	modelMatrix.ToIdentity();
	textureMatrix.ToIdentity();

	UpdateShaderMatrices();
	heightMap->Draw();



}

void Renderer::DrawCube() {
	BindShader(lightShader);

	SetShaderLight(*light);

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, brickWallTex);

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, brickWallBump);

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "shadowTex"), 2);
	glUniform3fv(glGetUniformLocation(lightShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
	glActiveTexture(GL_TEXTURE2);

	glBindTexture(GL_TEXTURE_2D, shadowTex);
	

	modelMatrix = Matrix4::Translation((heightMap->GetHeightmapSize()) * Vector3(0.4f, 2.0f, 0.2f)) * Matrix4::Scale(Vector3(100.0f,100.0f,100.0f));
	UpdateShaderMatrices();
	test->Draw();
}

void Renderer::DrawWater() {
	BindShader(reflectShader);

	glUniform3fv(glGetUniformLocation(reflectShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "cubeTex"), 2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, waterTex);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	Vector3 hSize = heightMap->GetHeightmapSize();

	modelMatrix = (Matrix4::Translation(hSize * 0.5f) * Matrix4::Translation(Vector3(1.0f, 50.0f * waterBob, 1.0f)) * Matrix4::Scale(hSize * 0.5f) * Matrix4::Rotation(90, Vector3(1, 0, 0)));

	textureMatrix = Matrix4::Translation(Vector3(waterCycle, 0.0f, waterCycle)) * Matrix4::Scale(Vector3(10, 10, 10))
		* Matrix4::Rotation(waterRotate, Vector3(0, 0, 1));

	UpdateShaderMatrices();

	quad->Draw();
}
void Renderer::ClearNodeLists() {
	transparentNodeList.clear();
	nodeList.clear();
}

void Renderer::DrawPostprocess() {
	glBindFramebuffer(GL_FRAMEBUFFER, processFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[1], 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	BindShader(processShader);
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	UpdateShaderMatrices();

	glDisable(GL_DEPTH_TEST);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(processShader->GetProgram(), "diffuseTex"), 0);

	for (int i = 0; i < POST_PASSES; ++i) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[1], 0);

		glUniform1i(glGetUniformLocation(processShader->GetProgram(), "isVertical"), 0);

		glBindTexture(GL_TEXTURE_2D, bufferColourTex[0]);

		quad2->Draw();

		glUniform1i(glGetUniformLocation(processShader->GetProgram(), "isVertical"), 1);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[0], 0);

		glBindTexture(GL_TEXTURE_2D, bufferColourTex[1]);

		quad2->Draw();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);
}
void Renderer::PresentScene() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	BindShader(sceneShader);
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	UpdateShaderMatrices();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bufferColourTex[0]);
	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "diffuseTex"), 0);



	quad2->Draw();
}
