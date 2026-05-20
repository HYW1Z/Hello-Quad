#version 300 es
precision highp float;

/**
 * \file
 * \author Rudy Castan
 * \author Ginam Park
 * \date 2025 Spring
 * \par CS250 Computer Graphics II
 * \copyright DigiPen Institute of Technology
 */

layout(location = 0) in vec3 aVertexPosition;
layout(location = 1) in vec3 aVertexColor;
layout(location = 2) in vec2 aVertexTextureCoordinates;

uniform float uTime;
uniform vec2  uMouse;

out vec3 vColor;
out vec2 vTextureCoordinates;

void main()
{
    // Breathing scale
    float scale = 1.0 + 0.1 * sin(uTime * 2.0);
    vec2 pos = aVertexPosition.xy * scale;

    // Oscillating rotation
    float angle = sin(uTime * 0.5) * 6.28318;
    float cosA  = cos(angle);
    float sinA  = sin(angle);
    vec2 rotated;
    rotated.x = pos.x * cosA - pos.y * sinA;
    rotated.y = pos.x * sinA + pos.y * cosA;

    // Follow mouse
    rotated += uMouse;

    gl_Position         = vec4(rotated, aVertexPosition.z, 1.0);
    vColor              = aVertexColor;
    vTextureCoordinates = aVertexTextureCoordinates;
}
