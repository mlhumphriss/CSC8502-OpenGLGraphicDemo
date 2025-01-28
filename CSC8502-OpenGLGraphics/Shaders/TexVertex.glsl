#version 330 core
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform mat4 textureMatrix;

in vec3 position;
in vec2 texCoord;
//in vec3 colour;

out Vertex {
vec2 texCoord;
//vec3 colour;
} OUT;
void main(void){
    //OUT.colour = colour;
	mat4 mvp = projMatrix*viewMatrix*modelMatrix;
	gl_Position = mvp * vec4(position, 1.0);
	
	//vec4 worldPos = (modelMatrix*vec4(position,1));
	//gl_Position = (projMatrix*viewMatrix)*worldPos;
	//OUT.texCoord = texCoord;

	OUT.texCoord = (textureMatrix* vec4(texCoord, 0.0,1.0)).xy;
}