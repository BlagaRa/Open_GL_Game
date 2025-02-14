#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;
in vec4 fFragPosLightSpace;

out vec4 fColor;

// Lighting
uniform vec3 lightDir;
uniform vec3 lightColor;

// Texture
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;


// Time for rain animation
uniform float time;

// Rain properties
uniform vec3 rainColor = vec3(0.4f, 0.4f, 1.0f); // Light blue rain
uniform float rainSpeed = 4.0f;                 // Reduced speed of rain
uniform float rainDensity = 0.02f;              // Increased density for larger drops
uniform float rainLength = 0.6f;                // Increased length for larger drops
uniform int rainisOn = 1;            // Length of raindrops
float shadow;
vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    normalizedCoords = normalizedCoords * 0.5 + 0.5;
    // Verifică dacă coordonatele sunt în afara intervalului valid
    if (normalizedCoords.x < 0.0 || normalizedCoords.x > 1.0 || 
        normalizedCoords.y < 0.0 || normalizedCoords.y > 1.0)
    {
        return 0.0; // În afara texturii de umbră, nu există umbră
    }
    float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
    float currentDepth = normalizedCoords.z;
    vec3 lightDirN = normalize(lightDir);
    float bias = max(0.005 * (1.0 - dot(fNormal, lightDirN)), 0.002);

    return currentDepth - bias > closestDepth ? 1.0 : 0.0;
}


void computeLightComponents()
{
    vec3 cameraPosEye = vec3(0.0f);
    vec3 normalEye = normalize(fNormal);
    vec3 lightDirN = normalize(lightDir);
    vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
    ambient = ambientStrength * lightColor;
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
    vec3 reflection = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
    specular = specularStrength * specCoeff * lightColor;
}

float RainEffect(vec2 uv)
{
    // Create a pseudo-random hash function
    float n = fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453);
    
    // Simulate falling rain
    float rainY = fract(uv.y + time * rainSpeed);
    
    // Check if this UV coordinate should have a raindrop
    return step(1.0 - rainDensity, n) * smoothstep(rainY, rainY + rainLength, 1.0);
}

void main()
{
    computeLightComponents();
    vec3 baseColor = vec3(0.9f, 0.35f, 0.0f); // Orange
    ambient *= texture(diffuseTexture, fTexCoords).rgb;
    diffuse *= texture(diffuseTexture, fTexCoords).rgb;
    specular *= texture(specularTexture, fTexCoords).rgb;
    shadow = ShadowCalculation(fFragPosLightSpace);
    vec3 lighting = min((ambient + (1.0f - shadow) * diffuse) + (1.0f - shadow) * specular, 1.0f);

    // Add rain effect
    float rainAlpha = rainisOn == 1 ? RainEffect(fTexCoords * 10.0) : 0.0; // Scale UV for denser rain
    vec3 finalColor = lighting ;
    finalColor = mix(finalColor, rainColor, rainAlpha);
    fColor = vec4(finalColor , 1.0f);
}
