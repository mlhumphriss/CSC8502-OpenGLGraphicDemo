#version 330 core

uniform sampler2D diffuseTex;

uniform int isVertical;

in Vertex{
	vec2 texCoord;
	//vec3 colour;
}IN;

out vec4 fragColor;



void main(void){

	fragColor = texture(diffuseTex, IN.texCoord)*1.5;
	
}