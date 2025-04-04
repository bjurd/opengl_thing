#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;
in vec3 MaterialColor;
in vec3 MaterialAmbient;
in vec3 MaterialDiffuse;
in vec3 MaterialSpecular;
in float MaterialShininess;

uniform int useTexture;
uniform sampler2D ourTexture;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform float uMaterialAlpha;

void main()
{
	// ambient
	float ambientStrength = 0.1;
	vec3 ambient = ambientStrength * lightColor;

	// diffuse
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

	// specular
	float specularStrength = 0.5;
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * lightColor;

	vec3 lighting = ambient + diffuse + specular;

	vec3 baseColor = objectColor;

	if (useTexture != 1)
	{
		baseColor *= MaterialColor;
	}

	if (useTexture == 1)
	{
		vec4 texColor = texture(ourTexture, TexCoord);
		baseColor *= texColor.rgb;
	}

	vec3 finalColor = lighting * baseColor;
	FragColor = vec4(finalColor, 1.0);
}
