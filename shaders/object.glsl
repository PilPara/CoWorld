#version 330 core

#define MAX_POINT_LIGHTS 21
#define MAX_SPOT_LIGHTS 5

struct Material {
  vec4 diffuse;
  vec4 specular;
  float shininess;
};

struct PointLight {
  vec3 position;

  float k0;
  float k1;
  float k2;

  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
};

struct DirectionalLight {
  vec3 direction;

  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
};

struct SpotLight {
  vec3 position;
  vec3 direction;

  float cutOff;
  float outerCutOff;

  float k0;
  float k1;
  float k2;

  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
};

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform Material material;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int nPointLights;
uniform DirectionalLight directionalLight;
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];
uniform int nSpotLights;
uniform vec3 viewPos;

uniform sampler2D diffuse0;
uniform sampler2D specular0;

uniform int noTex;

vec4 CalculatePointLight(int idx, vec3 norm, vec3 viewDir, vec4 diffMap, vec4 specMap);
vec4 CalculateDirectionalLight(vec3 norm, vec3 viewDir, vec4 diffMap, vec4 specMap);
vec4 CalculateSpotLight(int idx, vec3 norm, vec3 viewDir, vec4 diffMap, vec4 specMap);

void main() {

  vec3 norm = normalize(Normal);
  vec3 viewDir = normalize(viewPos - FragPos);

  vec4 diffMap;
  vec4 specMap;

  if (noTex == 1) {
    diffMap = material.diffuse;
    specMap = material.specular;
  }
  else {
    diffMap = texture(diffuse0, TexCoord);
    specMap = texture(specular0, TexCoord);
  }

  vec4 res = vec4(0.0);
    
  // directionalLight
  res = CalculateDirectionalLight(norm, viewDir, diffMap, specMap);

  // point lights
  for (int i = 0; i < nPointLights; i++) {
    res += CalculatePointLight(i, norm, viewDir, diffMap, specMap);
  }

  // spot lights
  for (int i = 0; i < nSpotLights; i++) {
    res += CalculateSpotLight(i, norm, viewDir, diffMap, specMap);
  }

  FragColor = res;
}

vec4 CalculatePointLight(int idx, vec3 norm, vec3 viewDir, vec4 diffMap, vec4 specMap) {
  // ambient
  vec4 ambient = pointLights[idx].ambient * diffMap;

  // diffuse
  vec3 lightDir = normalize(pointLights[idx].position - FragPos);
  float diff = max(dot(norm, lightDir), 0.0);
  vec4 diffuse = pointLights[idx].diffuse * (diff * diffMap);

  // specualr
  vec3 reflectDir = reflect(-lightDir, norm);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess * 128);
  vec4 specular = pointLights[idx].specular * (spec * specMap);

  float dist = length(pointLights[idx].position - FragPos);
  float attenuation = 1.0f / (pointLights[idx].k0 + pointLights[idx].k1 * dist + pointLights[idx].k2 * (dist * dist));

  return vec4(ambient + diffuse + specular) * attenuation;
}

vec4 CalculateDirectionalLight(vec3 norm, vec3 viewDir, vec4 diffMap, vec4 specMap) {
  // ambient
  vec4 ambient = directionalLight.ambient * diffMap;

  // diffuse
  vec3 lightDir = normalize(-directionalLight.direction);
  float diff = max(dot(norm, lightDir), 0.0);
  vec4 diffuse = directionalLight.diffuse * (diff * diffMap);

  // specualr
  vec3 reflectDir = reflect(-lightDir, norm);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess * 128);
  vec4 specular = directionalLight.specular * (spec * specMap);

  return vec4(ambient + diffuse + specular);
}

vec4 CalculateSpotLight(int idx, vec3 norm, vec3 viewDir, vec4 diffMap, vec4 specMap) {
  vec3 lightDir = normalize(spotLights[idx].position - FragPos);
  float theta = dot(lightDir, normalize(-spotLights[idx].direction));

  vec4 ambient = spotLights[idx].ambient * diffMap;

  if (theta > spotLights[idx].outerCutOff) {
    float diff = max(dot(norm, lightDir), 0.0);
    vec4 diffuse = spotLights[idx].diffuse * (diff * diffMap);
    
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess * 128);
    vec4 specular = spotLights[idx].specular * (spec * specMap);

    // float intensity = (theta - spotLights[idx].outerCutOff) / (spotLights[idx].cutOff - spotLights[idx].outerCutOff);

    float intensity = smoothstep(spotLights[idx].outerCutOff, spotLights[idx].cutOff, theta);
    intensity = clamp(intensity, 0.0, 1.0);
    diffuse *= intensity;
    specular *= intensity;

    float dist = length(spotLights[idx].position - FragPos);
    float attenuation = 1.0f / (spotLights[idx].k0 + spotLights[idx].k1 * dist + spotLights[idx].k2 * (dist * dist));

    return vec4(ambient + diffuse + specular) * attenuation;
  }
  else {
    return ambient;
  }
}

