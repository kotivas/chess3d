#version 420 core
out vec4 FragColor;

struct DirLight {
	bool draw;

	vec3 direction;
	
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
}; // 80
struct PointLight {
	bool draw;

	vec3 position;
	
	float constant;
	float linear;
	float quadratic;
	
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
}; // 96
struct SpotLight {
	bool draw;

	vec3 position;
	vec3 direction;
	float cutOff;
	float outerCutOff;
  
	float constant;
	float linear;
	float quadratic;
  
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;       
}; // 112

layout(std140, binding = 1) uniform Lights {
	DirLight dirLight;
	PointLight pointLight;
	SpotLight spotLight;
};

layout(std140, binding = 2) uniform Data {
	vec3 viewPos;
};


void main() {    
	FragColor = vec4(1, 1, 1, 1);
}