// sphere position, radius encoded as a vec4

float RaySphereIntersection(vec3 rayBase, vec3 rayDirectionNorm, vec4 sphere)
{
    vec3 sphereDir = rayBase - sphere.xyz;

    float B = 2.0 * dot(rayDirectionNorm, sphereDir);

    float C = dot(sphereDir, sphereDir) - (sphere.w * sphere.w);

    float disc = B * B - 4.0 * C;

    if (disc < 0.0) return -1.0;

    float sqd = sqrt(disc);

    vec2 t = .5 * vec2(-B - sqd, -B + sqd);

    t.x = t.x < 0 ? (1.0/0.0) : t.x;
    t.y = t.y < 0 ? (1.0/0.0) : t.y;

    return min(t.x, t.y);
}

bool RaySphereIntersectionTest(vec3 rayBase, vec3 rayDirectionNorm, vec4 sphere)
{
    vec3 sphereDir = rayBase - sphere.xyz;

    float B = 2.0 * dot(rayDirectionNorm, sphereDir); 

    float C = dot(sphereDir, sphereDir) - (sphere.w * sphere.w);

    float disc = B * B - 4.0 * C;

    if(disc < 0.0) return false;

    float sqd = sqrt(disc);

    return ((-B - sqd) > 0) ||  ((-B + sqd) > 0);
}
