#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aMaterialColor;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out vec3 MaterialColor;
out vec3 MaterialAmbient;
out vec3 MaterialDiffuse;
out vec3 MaterialSpecular;
out float MaterialShininess;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	vec4 worldPos = model * vec4(aPos, 1.0);
	FragPos = worldPos.xyz;

	mat3 normalMatrix = mat3(transpose(inverse(model)));
	Normal = normalize(normalMatrix * aNormal);

	TexCoord = aTexCoord;
	MaterialColor = aMaterialColor;
	MaterialAmbient = aMaterialColor.xyz;
	MaterialDiffuse = aMaterialColor.xyz;
	MaterialSpecular = aMaterialColor.xyz;
	MaterialShininess = 32.0;

	gl_Position = projection * view * worldPos;
}
