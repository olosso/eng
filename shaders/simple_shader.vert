#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

// This is forwarded to the fragment shader.
layout(location = 0) out vec3 fragColor;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projectionViewTransform;
    vec4 ambientLightColor;
    vec4 pointLightPosition;
    vec4 pointLightColor;
    float time;
} ubo;

layout(push_constant) uniform Push {
    mat4 modelTransform; // From model to world space
    mat4 normalMatrix;
} push;

void main() {
    vec4 vertexWorldPosition = push.modelTransform * vec4(position, 1.0);
    // vertexWorldPosition.y = -1 + vertexWorldPosition.y + cos(ubo.time);
    vec3 directionToPointLight = ubo.pointLightPosition.xyz - vertexWorldPosition.xyz;
    vec3 directionToPointLightUnit = normalize(ubo.pointLightPosition.xyz - vertexWorldPosition.xyz);
    float attenuation = dot(directionToPointLight, directionToPointLight);

    // Model -> world -> camera -> view
    gl_Position = ubo.projectionViewTransform * vertexWorldPosition;

    // Move the normals in sync with the model
    // Normals are directional, so they aren't changed by translation.
    // -> Use only the scale and rotation parts of the model transform.
    vec3 normalWorldSpace = normalize(mat3(push.normalMatrix) * normal);

    vec3 ambientLightColor = ubo.ambientLightColor.w * ubo.ambientLightColor.xyz;
    vec3 pointLightColor = ubo.pointLightColor.w * ubo.pointLightColor.xyz;
    vec3 diffuseLightColor = max(dot(normalWorldSpace, directionToPointLightUnit), 0) * pointLightColor;
    diffuseLightColor = diffuseLightColor / attenuation;

    fragColor = (diffuseLightColor + ambientLightColor) * color;
}
