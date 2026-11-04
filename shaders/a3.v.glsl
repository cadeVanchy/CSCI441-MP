#version 410 core

// uniform inputs
uniform mat4 mvpMatrix;
uniform mat4 modelMtx;
uniform mat3 normalMtx;
uniform vec3 materialColor;

uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform vec3 eyePosition;

// attribute inputs
layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;

// varying outputs
layout(location = 0) out vec3 color;

void main() {
    // transform and output the vertex in clip space
    gl_Position = mvpMatrix * vec4(vPos, 1.0);

    vec3 worldPos = vec3(modelMtx * vec4(vPos, 1.0));
    vec3 N = normalize(normalMtx * vNormal);
    vec3 L = normalize(-lightDirection);
    vec3 V = normalize(eyePosition - worldPos);
    vec3 R = reflect(-L, N);

    // ambient
    vec3 ambient = 0.15 * lightColor * materialColor;

    // diffuse
    float N_L = max(dot(N, L), 0.0);
    vec3 diffuse = lightColor * materialColor * N_L;

    // specular
    float specFactor = 0.0;
    if (N_L > 0.0) {
        specFactor = pow(max(dot(V, R), 0.0), 32.0);
    }
    vec3 specular = lightColor * 0.6 * specFactor;

    color = ambient + diffuse + specular;
}
