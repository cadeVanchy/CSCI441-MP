#version 410 core

// Gouraud Shading with the Phong Reflectance Model
// All lighting is computed HERE, per vertex, in world space.
// The fragment shader only outputs the interpolated color.

//vertex position in object space
layout(location = 0) in vec3 vPos;
//vertex normal in object space
layout(location = 1) in vec3 vNormal;

uniform mat4 mvpMatrix;
uniform mat4 modelMatrix;
uniform mat3 normalMatrix;

uniform vec3  materialDiffuse;
uniform vec3  materialAmbient;
uniform vec3  materialSpecular;
uniform float materialShininess;

//camera position in world space
uniform vec3 eyePos;

// Lights

/// \brief the sun type light
struct DirectionalLight {
    vec3 direction;
    vec3 color;
};//end struct DirectionalLight

struct PointLight {
    vec3 position;
    vec3 color;
    vec3 attenuation;
};//end struct PointLight

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;
    vec3 attenuation;
    float cosInnerCutoff;
    float cosOuterCutoff;
};//end struct SpotLight

uniform SpotLight spotLight;

uniform PointLight pointLight;

//Varyings
uniform DirectionalLight dirLight;

layout(location = 0) out vec4 vertexColor;

// Phong Reflectance Model for a single light
//   lightColor - color of the light reaching this vertex
//   L          - unit vector from the vertex TOWARD the light
//   N          - unit surface normal (world space)
//   V          - unit vector from the vertex TOWARD the camera
// returns ambient + diffuse + specular contribution of this light
vec3 phongReflectance(vec3 lightColor, vec3 L, vec3 N, vec3 V) {

    //ambient
    vec3 ambient = lightColor * materialAmbient;

    //diffuse
    float NdotL = max(dot(N, L), 0.0);
    vec3 diffuse = lightColor * materialDiffuse * NdotL;

    //specular
    vec3 specular = vec3(0.0);
    if(NdotL > 0.0) {
        vec3 R = reflect(-L, N);
        float RdotV = max(dot(R, V), 0.0);

        specular = lightColor * materialSpecular * pow(RdotV, max(materialShininess, 1.0));
    }//end if

    return ambient + diffuse + specular;
}//end phongReflectance()

void main() {
    //transform and output the vertex in clip space
    gl_Position = mvpMatrix * vec4(vPos, 1.0);

    //transform vertex information into world space
    vec3 P = vec3(modelMatrix * vec4(vPos, 1.0));
    vec3 N = normalize(normalMatrix * vNormal);
    vec3 V = normalize(eyePos - P);

    vec3 totalColor = vec3(0.0);

    //directional light
    vec3 dirL = normalize(-dirLight.direction);
    totalColor += phongReflectance(dirLight.color, dirL, N, V);

    //point light
    vec3 toPoint = pointLight.position - P;
    float pointDist = length(toPoint);
    vec3 pointL = toPoint / pointDist;
    float pointAtten = 1.0 / (pointLight.attenuation.x
    + pointLight.attenuation.y * pointDist
    + pointLight.attenuation.z * pointDist * pointDist);
    totalColor += pointAtten * phongReflectance(pointLight.color, pointL, N, V);

    //spotlight
    vec3 toSpot = spotLight.position - P;
    float spotDist = length(toSpot);
    vec3 spotL = toSpot / spotDist;
    float spotAtten = 1.0 / (spotLight.attenuation.x + spotLight.attenuation.y * spotDist + spotLight.attenuation.z * spotDist * spotDist);

    //angle between the cone's center and the ray light
    float cosTheta = dot(-spotL, normalize(spotLight.direction));

    //fade between inner code and outercone
    float spotCone = smoothstep(spotLight.cosOuterCutoff, spotLight.cosInnerCutoff, cosTheta);
    totalColor += spotAtten * spotCone * phongReflectance(spotLight.color, spotL, N, V);

    vertexColor = vec4(totalColor, 1.0);
    
}//end main()
