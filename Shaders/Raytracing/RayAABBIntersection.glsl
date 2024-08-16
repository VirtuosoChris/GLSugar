float min3(vec3 x)
{
    return min(min(x.x, x.y), x.z);
}

float max3(vec3 x)
{
    return max(max(x.x, x.y), x.z);
}

float RayAABBIntersection(vec3 rayOrigin, vec3 rayDirectionNorm, vec3 aabbMin, vec3 aabbMax)
{
    vec3 invDir = 1.0 / rayDirectionNorm;
    vec3 t1 = (aabbMin - rayOrigin) * invDir;
    vec3 t2 = (aabbMax - rayOrigin) * invDir;

    vec3 tMins = min(t1,t2);
    vec3 tMaxes = max(t1,t2);

    float maxOfMins = max3(tMins);
    float minOfMaxes = min3(tMaxes);

    if (maxOfMins > minOfMaxes) return -1.0;

    return maxOfMins;
}

bool RayIntersectsAABB(vec3 rayOrigin, vec3 rayDirectionNorm, vec3 aabbMin, vec3 aabbMax)
{
    vec3 invDir = 1.0 / rayDirectionNorm;
    vec3 t1 = (aabbMin - rayOrigin) * invDir;
    vec3 t2 = (aabbMax - rayOrigin) * invDir;

    vec3 tMins = min(t1,t2);
    vec3 tMaxes = max(t1,t2);

    float maxOfMins = max3(tMins);
    float minOfMaxes = min3(tMaxes);

    return maxOfMins <= minOfMaxes;
}
