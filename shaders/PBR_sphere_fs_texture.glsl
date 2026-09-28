#version 330 core

#define MAX_LIGHTS 10

out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;
  
uniform vec3 camPos;

uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;

uniform vec3 albedo = vec3(0,0,0);
uniform float metallic = 0;
uniform float roughness = 0;
uniform float ao = 1;

uniform bool useAlbedoMap = false;
uniform bool useNormalMap = false;
uniform bool useMetallicMap = false;
uniform bool useRoughnessMap = false;
uniform bool useAoMap = false;

struct Light {
    vec3 position;
    vec3 color;
};

uniform int lightCount;
uniform Light lights[MAX_LIGHTS];

const float PI = 3.14159265359;

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

void main()
{
    vec3 albedoVal = albedo;
    if (useAlbedoMap) {
        albedoVal = pow(texture(albedoMap, TexCoords).rgb, vec3(2.2));
    }

    vec3 N = normalize(Normal);
    if (useNormalMap) {
        N = getNormalFromMap();
    }

    float metallicVal = metallic;
    if (useMetallicMap) {
        metallicVal  = texture(metallicMap, TexCoords).r;
    }

    float roughnessVal = roughness;
    if (useRoughnessMap) {
        roughnessVal = texture(roughnessMap, TexCoords).r;
    }

    float aoVal = ao;
    if (useAoMap) {
        aoVal = texture(aoMap, TexCoords).r;
    }

    vec3 V = normalize(camPos - WorldPos);

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedoVal, metallicVal);
	           
    // reflectance equation
    vec3 Lo = vec3(0.0);

    for(int i = 0; i < lightCount; ++i)
    {
        // calculate per-light radiance
        vec3 L = normalize(lights[i].position - WorldPos);
        vec3 H = normalize(V + L);
        float distance    = length(lights[i].position - WorldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance     = lights[i].color * attenuation;        
        
        // cook-torrance brdf
        float NDF = DistributionGGX(N, H, roughnessVal);        
        float G   = GeometrySmith(N, V, L, roughnessVal);      
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       
        
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallicVal;	  
        
        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular     = numerator / denominator;  
            
        // add to outgoing radiance Lo
        float NdotL = max(dot(N, L), 0.0);                
        Lo += (kD * albedoVal / PI + specular) * radiance * NdotL; 
    }
  
    vec3 ambient = vec3(0.03) * albedoVal * aoVal;
    vec3 color = ambient + Lo;
	
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));  
   
    FragColor = vec4(color, 1.0);
} 
