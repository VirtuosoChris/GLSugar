#version 450 core

precision highp float;

layout(local_size_x=16, local_size_y=16, local_size_z=1) in;

uniform sampler2D imgIn;
uniform int lod;
uniform vec2 texelSize;
uniform float offset;

layout (binding = 0, r11f_g11f_b10f) uniform image2D imgOut;

void main()
{
    const vec2 texcoord = texelSize * (vec2(.5) + vec2(gl_GlobalInvocationID.xy));
    const vec2 halftexel = .5 * texelSize;
    const vec2 halftexel2 = vec2(halftexel.x, -halftexel.y);
    const vec2 halftexel3 = vec2(-halftexel.x, halftexel.y);
    const vec2 tx = vec2(2.0 * halftexel.x, 0.0);
    const vec2 ty = vec2(0.0, 2.0*halftexel.y);

    vec3 fetch1 = textureLod(imgIn, texcoord + halftexel3 * offset, lod).xyz;
    vec3 fetch2 = textureLod(imgIn, texcoord + halftexel * offset, lod).xyz;
    vec3 fetch3 = textureLod(imgIn, texcoord - halftexel * offset,lod).xyz;
    vec3 fetch4 = textureLod(imgIn, texcoord + halftexel2 * offset,lod).xyz;

    vec3 fetch5 = textureLod(imgIn, texcoord + tx * offset, lod).xyz;
    vec3 fetch6 = textureLod(imgIn, texcoord - tx * offset, lod).xyz;
    vec3 fetch7 = textureLod(imgIn, texcoord + ty * offset, lod).xyz;
    vec3 fetch8 = textureLod(imgIn, texcoord - ty * offset, lod).xyz;

    vec3 val = 
    (fetch1 * 2.0
    + fetch2 * 2.0
    + fetch3 * 2.0
    + fetch4 * 2.0
    + fetch5
    + fetch6
    + fetch7
    + fetch8)
    * .08333333333; // 1 / 12

    imageStore(imgOut, ivec2(gl_GlobalInvocationID.xy),vec4(val, 1.0));
}
