#version 460 core
out vec4 FragColor;

#define NR_POINT_LIGHTS 1

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    float shininess;
};

struct DirLight {
	vec3 direction;
	
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight {
	vec3 position;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct SpotLight {
	vec3 position;
	vec3 direction;
	float cutOff;
	float outerCutOff;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
};

in vec3 FragPos;
in vec2 TexCoords;
in vec3 Normal;

uniform vec3 viewPos;
uniform Material material;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform bool useToon;

uniform bool useTexture;
uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;
uniform vec3 Ns;

uniform sampler2D toonTexture;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
float CalcDetail(float z, vec3 normal, vec3 viewDir);

void main() {
	vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = CalcDirLight(dirLight, norm, viewDir);

    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);    

	//result += CalcSpotLight(spotLight, norm, FragPos, viewDir);

	float detail = CalcDetail(length(viewPos - FragPos), norm, viewDir);

	vec4 toonLight = texture(toonTexture, vec2((result.x + result.y + result.z) / 3.0f, detail));
	vec4 textureColor = useTexture ? texture(material.texture_diffuse1, TexCoords) : vec4(Kd, 1.0f);

	if (useToon)
		FragColor = toonLight * textureColor;
	else
		FragColor = vec4(result, 1.0f) * textureColor;
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
	vec3 lightDir = normalize(-light.direction);

	float diff = max(dot(normal, lightDir), 0.0);

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

	vec3 ambient = light.ambient;
	vec3 diffuse = light.diffuse * diff;
	vec3 specular = light.specular * spec;

	return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    

    vec3 ambient  = light.ambient;
    vec3 diffuse  = light.diffuse * diff;
    vec3 specular = light.specular * spec;

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

	float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = (light.cutOff - light.outerCutOff);
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

	vec3 ambient = light.ambient;
	vec3 diffuse = light.diffuse * diff;
	vec3 specular = light.specular * spec;

    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return (ambient + diffuse + specular);
}

float CalcDetail(float z, vec3 normal, vec3 viewDir){
	 float z_min = 3;
	 float z_max = 15;
	 if(z > z_max)
		z = z_max;
	 float r = z_max/z_min;
	 float depthDetail = 1- (log(z/z_min) / log(r));

	 float orientationDetailMagnitude = 1;
	 float orientationDetail = pow(dot(normal, viewDir), orientationDetailMagnitude);

	return min(depthDetail, orientationDetail);
}