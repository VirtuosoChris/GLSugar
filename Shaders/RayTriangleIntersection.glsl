bool rayTriangleIntersectionValid(vec4 albegat);
bool rayTriangleIntersectionValidMaxT(vec4 albegat, float maxT);
vec4 rayTriangleIntersection(vec3 rayOrigin, vec3 rayDirectionNorm, vec3 vertex0, vec3 vertex1, vec3 vertex2);

// returns packed barycentric coords, t of intersection as vec4
vec4 rayPlaneIntersection(vec3 rayOrigin, vec3 rayDirectionNorm, vec3 vertex0, vec3 vertex1, vec3 vertex2)
{
    mat3 barycentricMatrix = inverse(mat3(vertex1 - vertex0, vertex2 - vertex0, -rayDirectionNorm));

    vec3 bgt = barycentricMatrix * (rayOrigin - vertex0);

    // return alpha beta gamma (barycentric) and t
    return vec4(1.0 - bgt.x - bgt.y, bgt);
}

bool isRayTriangleIntersectionValid(vec4 albegat)
{
    return all(greaterThan(albegat, vec4(0.0))) && all(lessThan(albegat.xyz, vec3(1.0)));
}

bool isRayTriangleIntersectionValid(vec4 albegat, float maxT)
{
    return all(greaterThan(albegat, vec4(0.0))) && all(lessThan(albegat, vec4(vec3(1.0), maxT)));
}

// -- returns packed barycentric coords, t of intersection as vec4.  t < 0 means no valid intersection
vec4 rayTriangleIntersection(vec3 rayOrigin, vec3 rayDirectionNorm, vec3 vertex0, vec3 vertex1, vec3 vertex2)
{
    vec4 intersection = rayPlaneIntersection(rayOrigin, rayDirectionNorm, vertex0, vertex1, vertex2);
    intersection.w = isRayTriangleIntersectionValid(intersection) ? intersection.w : -1.0;
    return intersection;
}

// -- returns packed barycentric coords, t of intersection as vec4.  t < 0 means no valid intersection
vec4 rayTriangleIntersection(vec3 rayOrigin, vec3 rayDirectionNorm, float maxT, vec3 vertex0, vec3 vertex1, vec3 vertex2)
{
    vec4 intersection = rayPlaneIntersection(rayOrigin, rayDirectionNorm, vertex0, vertex1, vertex2);
    intersection.w = isRayTriangleIntersectionValid(intersection, maxT) ? intersection.w : -1.0;
    return intersection;
}