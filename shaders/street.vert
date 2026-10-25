#version 330 core
layout(location=0) in vec3 pos;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 texCoord;
uniform mat4 model, view, projection, lightSpace;
out vec3 FragPos, Normal;
out vec2 TexCoord;
out vec4 ShadowPos;
void main() {
    vec4 world = model * vec4(pos,1.0);
    FragPos = world.xyz;
    Normal = mat3(transpose(inverse(model))) * normal;
    TexCoord = texCoord;
    ShadowPos = lightSpace * world;
    gl_Position = projection * view * world;
}
