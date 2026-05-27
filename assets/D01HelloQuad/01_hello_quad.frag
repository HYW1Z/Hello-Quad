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

in vec3 vColor;
in vec2 vTextureCoordinates;

uniform sampler2D uTex2d;
uniform float     uTime;

layout(location = 0) out vec4 fFragmentColor;

void main()
{
    vec3 texColor    = texture(uTex2d, vTextureCoordinates).rgb;
    vec3 vertColor   = vColor;

    // Blend between texture and vertex colors over time
    float mixFactor  = 0.5 + 0.5 * sin(uTime * 1.5);
    vec3  finalColor = mix(vertColor, texColor, mixFactor);

    fFragmentColor = vec4(finalColor, 1.0);
}
