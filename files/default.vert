#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in float aAttached;
layout (location = 2) in int aObjectIdx;
layout (location = 3) in float aPolygonal;

out vec3 fragPos;
flat out int fragObjectIdx;
flat out float fragPolygonal;
invariant gl_Position;

uniform vec3 iCameraPos;
uniform vec3 iCameraRot;
uniform vec2 iResolution;

mat3 cameraRotation(vec3 deg) {
    vec3 r = radians(deg);
    float cp = cos(r.x), sp = sin(r.x);
    float cy = cos(r.y), sy = sin(r.y);
    float cr = cos(r.z), sr = sin(r.z);

    mat3 Rx = mat3(1.0, 0.0, 0.0,
                   0.0,  cp,  sp,
                   0.0, -sp,  cp);
    mat3 Ry = mat3( cy, 0.0, -sy,
                   0.0, 1.0, 0.0,
                    sy, 0.0,  cy);
    mat3 Rz = mat3( cr,  sr, 0.0,
                   -sr,  cr, 0.0,
                   0.0, 0.0, 1.0);

    return Rz * Rx * Ry;
}

void main() {
    mat3 R = cameraRotation(iCameraRot);
    vec3 worldRel = aPos - iCameraPos;
    vec3 relPos  = mix(worldRel, transpose(R) * aPos, aAttached);
    vec3 viewPos = mix(R * worldRel, aPos, aAttached);

    fragPos = relPos;
    fragObjectIdx = aObjectIdx;
    fragPolygonal = aPolygonal;

    float minRes = min(iResolution.x, iResolution.y);
    float A = 2.0 * minRes / iResolution.x;
    float B = 2.0 * minRes / iResolution.y;

    const float NEAR = 0.01;
    const float FAR  = 1000.0;
    float C =  (FAR + NEAR) / (FAR - NEAR);
    float D = -2.0 * NEAR * FAR / (FAR - NEAR);

    gl_Position = vec4(
        A * viewPos.x,
        B * viewPos.y,
        C * viewPos.z + D,
        viewPos.z          // w = z_view (>0 впереди)
    );
}