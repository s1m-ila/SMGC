#version 460 core
layout(early_fragment_tests) in;
in vec3 fragPos; // Интерполированная позиция фрагмента
flat in int fragObjectIdx;
flat in float fragPolygonal;
out vec4 fragColor;

#define ITERATIONS 1000
#define EPSILON iEpsilon

uniform float iEpsilon;
uniform float iTime;
uniform vec2 iResolution;
uniform vec3 iCameraPos;
uniform vec3 iCameraRot;
uniform float iOutline;
uniform float iMaxDist;
uniform float iShadows;

struct Object {
    vec4 data1;
    vec4 data2;
    vec4 data3;
    vec4 invRot[3];
};

layout(std430, binding = 0) buffer ObjectsBuffer {
    Object items[];
} ObjectsData;

float squareSDF(float radius, vec2 p) {
    vec2 d = abs(p) - radius;
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0));
}

float crossSDF(in vec3 p) {
    float da = max(abs(p.x), abs(p.y));
    float db = max(abs(p.y), abs(p.z));
    float dc = max(abs(p.z), abs(p.x));
    return min(da, min(db, dc)) - 1.0;
}

float boxSDF(vec3 p, vec3 b) {
    vec3 q = abs(p) - b;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}

float sphereSDF(float radius, vec3 p) {
    return length(p) - radius;
}

float cubeSDF(float radius, vec3 p) {
    vec3 d = abs(p) - radius;
    return min(max(d.x, max(d.y, d.z)), 0.0) + length(max(d, 0.0));
}

float cubeOutSDF(float radius, vec3 p) {
    return length(max(abs(p) - radius, 0.0));
}

float cubeInSDF(float radius, vec3 p) {
    vec3 d = abs(p) - radius;
    return min(max(d.x, max(d.y, d.z)), 0.0);
}

float capsuleSDF(float radius, float height, vec3 p) {
    p.y = max(abs(p.y - height + radius) - height * 0.5, 0.0f);
    return sphereSDF(radius, p);
}

float mandelbulbSDF(float i_radius, vec3 p) {
    // power = 8

    float boundingSphereRadius = 1.05;

    float boundingSphere = sphereSDF(boundingSphereRadius * i_radius, p);
    if (boundingSphere >= EPSILON) return boundingSphere;
    
    float radius = i_radius / boundingSphereRadius;
    vec3 q = p / radius;

    vec3 z = q;
    float dr = 1.0;
    float r = 0.0;

    for (int i = 0; i < 10; i++) {
        float r2 = dot(z, z);
        r = sqrt(r2);
        if (r > 2.0) break;

        float cT = z.y / r;
        float sT = sqrt(max(0.0, 1.0 - cT * cT));
        float r_xz = sqrt(max(1e-20, r2 - z.y * z.y));
        float inv_rxz = 1.0 / r_xz;
        float sP = z.x * inv_rxz;
        float cP = z.z * inv_rxz;

        for (int j = 0; j < 3; j++) {
            float ncT = 2.0 * cT * cT - 1.0;
            float nsT = 2.0 * sT * cT;
            float ncP = 2.0 * cP * cP - 1.0;
            float nsP = 2.0 * sP * cP;
            cT = ncT; sT = nsT;
            cP = ncP; sP = nsP;
        }

        // r^4, r^7, r^8 без pow()
        float r4 = r2 * r2;
        float r7 = r4 * r2 * r;
        float r8 = r4 * r4;

        dr = 8.0 * r7 * dr + 1.0;
        z  = r8 * vec3(sT * sP, cT, sT * cP) + q;
    }

    return 0.5 * log(r) * r / dr * radius;
}

float mengerSpongeSDF(float radius, vec3 p) {
    vec3 q = p / radius;

    float distOut = cubeOutSDF(1.0, q);
    if (distOut * radius >= EPSILON) return distOut * radius;

    float dist = distOut + cubeInSDF(1.0, q);

    float scale = 1.0;
    for (int i = 0; i < 5; i++) {
        vec3 a = mod(q * scale, 2.0) - 1.0;
        scale *= 3.0;
        vec3 r = abs(1.0 - 3.0 * abs(a));

        float c = crossSDF(r) / scale;
        dist = max(dist, c);
    }

        return dist * radius;
}

float planeSDF(vec3 p) {
    return abs(p.y) - 0.5 * EPSILON;
}

vec3 tileColor(vec2 xz) {
    vec3 colA = vec3(0.333);
    vec3 colB = vec3(0.333 * 0.5);
    float tile = step(clamp(abs(squareSDF(0.5, mod(xz, 1.0) - 0.5)), 0.0, 1.0), 0.025);

    return mix(colA, colB, tile);
}

vec4 tilePlane(vec3 p) {
    float dist = planeSDF(p);

    if (dist >= EPSILON) return vec4(0.0, 0.0, 0.0, dist);

    vec3 col = tileColor(p.xz);

    return vec4(col, dist);
    /*vec3 col = colA;
    colB = vec3(0.666, 0.333, 0.333);
    vec3 colC = vec3(0.333, 0.333, 0.666);

    if (tile < 0.0) col = colC;
    else if (tile > 0.0) col = colB;

    return vec4(col * clamp(abs(tile) * 0.5 + 0.5, 0.0, 1.0), dist);*/
}

struct mapResult {
    float dist;
    vec4 col;
    bool is_fractal;
};

mapResult map(vec3 p, bool simple) {
    vec4 plane = tilePlane(p + 1.0);
    vec4 col = vec4(plane.rgb, 1.0);
    float dist = plane.w;

    bool fractal = false;

    for (int i = 0; i < ObjectsData.items.length(); ++i) {
        Object obj = ObjectsData.items[i];
        if (obj.invRot[0].w > 0.5) continue;
        int type = int(round(obj.data1.x));
        vec3 lp = mat3(obj.invRot[0].xyz, obj.invRot[1].xyz, obj.invRot[2].xyz) * (p - obj.data1.yzw);
        float dx = iMaxDist;
        int a_type = abs(type);
        bool fractal_x = type < 0;
        if (!fractal_x || simple) {
            if (a_type == 1) {
                dx = sphereSDF(obj.data2.x, lp);
            } else if (a_type == 2) {
                dx = cubeSDF(obj.data2.x, lp);
            }
            else if (a_type == 3) {
                dx = capsuleSDF(obj.data2.x, obj.data2.w, lp);
            }
        } else {
            if (a_type == 1) {
                dx = mandelbulbSDF(obj.data2.x, lp);
            } else if (a_type == 2) {
                dx = mengerSpongeSDF(obj.data2.x, lp);
            } 
        }
        if (dx < dist) {
            dist = dx;
            col = obj.data3;
            fractal = fractal_x;
        }
    }

    mapResult res;
    res.dist = dist;
    res.col = col;
    res.is_fractal = fractal;
    return res;
}

vec3 normal(vec3 p) {
    const vec2 k = vec2(1.0, -1.0);
    return normalize(
        k.xyy * map(p + k.xyy * EPSILON, false).dist +
        k.yyx * map(p + k.yyx * EPSILON, false).dist +
        k.yxy * map(p + k.yxy * EPSILON, false).dist +
        k.xxx * map(p + k.xxx * EPSILON, false).dist
    );
}

float calcShadow(vec3 ro, vec3 rd, float mint, float maxt, float k) {
    float res = 1.0;
    float t = mint;
    
    for (int i = 0; i < 40; i++) {
        float h = map(ro + rd * t, true).dist;
        
        if (h < EPSILON) {
            return 0.0; 
        }
        
        res = min(res, k * h / t);
        t += h;
        
        if (t > maxt) break;
    }
    return clamp(res, 0.0, 1.0);
}

float light(vec3 p, bool shadows) {
    vec3 light_pos = vec3(2.0, 4.0, 2.0);
    vec3 l = normalize(light_pos - p);
    vec3 normal = normal(p);
    float lambert = clamp(dot(normal, l) * 0.5 + 0.5, 0.0, 1.0);
    if (!shadows) return lambert;
    float shadow = calcShadow(p + normal * 0.02, l, 0.01, 20.0, 16.0) * 0.5 + 0.5;
    return min(lambert, shadow);
}

vec3 calcColor(vec3 p, vec3 ro, int i, float d) {
    // base color
    mapResult map_result = map(p, false);
    vec3 col = map_result.col.rgb;

    // light
    col *= light(p, !map_result.is_fractal && iShadows > 0.5);

    // ambient occlusion
    col *= pow(1.0 - (float(i) / float(ITERATIONS) * 2.0), 2.0);

    // fog
    float fog = pow(clamp(distance(p, ro) / iMaxDist, 0.0, 1.0), 2.0);
    col = col * (1.0 - fog) + 0.28 * fog;
    
    return col;
}

vec3 raymarch(vec3 ro, vec3 rd, float start) {
    float total_distance_traveled = start;
    if (total_distance_traveled > iMaxDist) return vec3(0.28);
    for (int i = 0; i < ITERATIONS; i++)
    {
        vec3 p = ro + total_distance_traveled * rd;
        float d = map(p, false).dist;
        if (total_distance_traveled > iMaxDist || d < EPSILON) {
            return calcColor(p, ro, i, d);
        }
        total_distance_traveled += d;
    }
    return vec3(0.0);
}

vec3 polygonColor() {
    if (fragObjectIdx == -2) return vec3(0.28);

    vec3 p = iCameraPos + fragPos;

    vec3 n = normalize(cross(dFdx(fragPos), dFdy(fragPos)));
    if (dot(n, fragPos) > 0.0) n = -n;

    vec3 col = (fragObjectIdx >= 0 && fragObjectIdx < ObjectsData.items.length())
        ? ObjectsData.items[fragObjectIdx].data3.rgb
        : tileColor(p.xz + 1.0);

    vec3 l = normalize(vec3(2.0, 4.0, 2.0) - p);
    col *= clamp(dot(n, l) * 0.5 + 0.5, 0.0, 1.0);

    float fog = pow(clamp(length(fragPos) / iMaxDist, 0.0, 1.0), 2.0);
    return col * (1.0 - fog) + 0.28 * fog;
}

void main() {
    if (iOutline > 0.5) {
        fragColor = vec4(1.0, 0.5, 0.0, 1.0);
        return;
    }

    vec3 col;
    if (fragPolygonal > 0.5) {
        col = polygonColor();
    } else {
        vec3 rd = normalize(fragPos);
        col = raymarch(iCameraPos, rd, length(fragPos));
    }

    // Дизеринг
    float noise = fract(sin(dot(gl_FragCoord.xy, vec2(12.9898, 78.233))) * 43758.5453);
    col += (noise - 0.5) / 255.0;

    // Output to screen
    fragColor = vec4(col, 1.0);
}
