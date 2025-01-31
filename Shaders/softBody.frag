#version 460 core

precision highp float;
layout(binding = 0) uniform highp sampler2DArray diffuseSmpl;
layout(binding = 1) uniform highp sampler2DArray specularSmpl;

struct Material {
  highp sampler2DArray diffuse;
  highp sampler2DArray specular;
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

  float constant;
  float linear;
  float quadratic;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

#define NR_DIR_LIGHTS 100
#define NR_POINT_LIGHTS 100
#define NR_SPOT_LIGHTS 100

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in float tDepth;

uniform vec3 viewPos;
uniform DirLight dirLights[NR_DIR_LIGHTS];
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLights[NR_SPOT_LIGHTS];
uniform Material material;
uniform int nPointLights = 10;
uniform int nSpotLights = 10;
uniform int nDirLights = 10;
uniform vec3 colour = vec3(1);

out vec4 FragColour;

// function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main() {
  // properties
  vec3 norm = normalize(Normal);
  vec3 viewDir = normalize(viewPos - FragPos);

  // directional lights
  vec3 result = vec3(0.0);
  for (int i = 0; i < nDirLights; i++)
    result += CalcDirLight(dirLights[i], norm, viewDir);

  // point lights
  for (int i = 0; i < nPointLights; i++)
    result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);

  // spot lights
  for (int i = 0; i < nSpotLights; i++)
    result += CalcSpotLight(spotLights[i], norm, FragPos, viewDir);

  FragColour = vec4(result, 1.0);
//   FragColour = vec4(norm, 1.0);
//   FragColour = vec4(.2, .8, .2, 1.0);
}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
  vec3 lightDir = normalize(-light.direction);
  // diffuse shading
  float diff = max(dot(normal, lightDir), 0.0);
  // specular shading
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
  // combine results
  vec3 ambient = light.ambient * colour;
  vec3 diffuse = light.diffuse * diff * colour;
  vec3 specular = light.specular * spec * colour;
  return (ambient + diffuse + specular);
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
  vec3 lightDir = normalize(light.position - fragPos);
  // diffuse shading
  float diff = max(dot(normal, lightDir), 0.0);
  // specular shading
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
  // attenuation
  float distance = length(light.position - fragPos);
  float attenuation = 1.0 / (light.constant + light.linear * distance +
                             light.quadratic * (distance * distance));
  // combine results
  vec3 ambient = light.ambient * colour;
  vec3 diffuse = light.diffuse * diff * colour;
  vec3 specular = light.specular * spec * colour;
  ambient *= attenuation;
  diffuse *= attenuation;
  specular *= attenuation;
  return (ambient + diffuse + specular);
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
  vec3 lightDir = normalize(light.position - fragPos);
  float diff = max(dot(normal, lightDir), 0.0);
  vec3 halfwayDir = normalize(lightDir + viewDir);
  float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);

  float distance = length(light.position - fragPos);
  float attenuation = 1.0 / (light.constant + light.linear * distance +
                             light.quadratic * (distance * distance));

  float theta = dot(lightDir, normalize(-light.direction));
  float epsilon = light.cutOff - light.outerCutOff;
  float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
  // combine results
  vec3 ambient = light.ambient * colour;
  vec3 diffuse = light.diffuse * diff * colour;
  vec3 specular = light.specular * spec * colour;
  ambient *= attenuation * intensity;
  diffuse *= attenuation * intensity;
  specular *= attenuation * intensity;
  return ambient + diffuse + specular;
}
