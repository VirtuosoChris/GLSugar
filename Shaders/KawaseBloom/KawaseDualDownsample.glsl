#version 450 core

precision highp float;

layout(local_size_x=16, local_size_y=16, local_size_z=1) in;

uniform sampler2D imgIn;
uniform int lod;
uniform vec2 texelSize;
uniform bool brightPass;
uniform float offset;

layout (binding = 0, r11f_g11f_b10f) uniform image2D imgOut;

vec3 brightPassTexel(vec3 lookup)
{
    const vec3 toLum2 = vec3(0.2126, 0.7152, 0.0722);
    float lum2 = dot(lookup.xyz,toLum2);
    const float thresh = 1.05;
    lookup.xyz *= smoothstep(.95*thresh, thresh, lum2);
    return lookup;
}

void main()
{
    const vec2 texcoord = texelSize * (vec2(.5) + vec2(gl_GlobalInvocationID.xy));
    const vec2 halftexel = .5 * texelSize;
    const vec2 halftexel2 = vec2(halftexel.x, -halftexel.y);

    vec3 fetch1 = textureLod(imgIn, texcoord, lod).xyz;
    vec3 fetch2 = textureLod(imgIn, texcoord + halftexel  * offset, lod).xyz;
    vec3 fetch3 = textureLod(imgIn, texcoord - halftexel  * offset, lod).xyz;
    vec3 fetch4 = textureLod(imgIn, texcoord + halftexel2 * offset, lod).xyz;
    vec3 fetch5 = textureLod(imgIn, texcoord - halftexel2 * offset, lod).xyz;

    if (brightPass)
    {
        fetch1 = brightPassTexel(fetch1);
        fetch2 = brightPassTexel(fetch2);
        fetch3 = brightPassTexel(fetch3);
        fetch4 = brightPassTexel(fetch4);
        fetch5 = brightPassTexel(fetch5);
    }

    vec3 val = (4.0 * fetch1 + fetch2 + fetch3 + fetch4 + fetch5) * .125;

    imageStore(imgOut, ivec2(gl_GlobalInvocationID.xy), vec4(val,1.0));
}
