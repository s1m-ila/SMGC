#version 460 core
in vec2 fragLocal;
flat in vec4 fragShapeColor;
flat in vec4 fragParams;
flat in int fragShape;
out vec4 fragColor;

uniform float iOutline;

#define SHAPE_POLYGON      0
#define SHAPE_CIRCLE       1
#define SHAPE_ELLIPSE      2
#define SHAPE_ROUNDED_RECT 3
#define SHAPE_RING         4
#define SHAPE_LINE         5

float circleSDF(vec2 p, float radius) {
    return length(p) - radius;
}

float ellipseSDF(vec2 p, vec2 radii) {
    float k0 = length(p / radii);
    float k1 = length(p / (radii * radii));
    if (k1 == 0.0) return -min(radii.x, radii.y);
    return k0 * (k0 - 1.0) / k1;
}

float roundedRectSDF(vec2 p, vec2 halfSize, float radius) {
    vec2 q = abs(p) - halfSize + radius;
    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - radius;
}

float ringSDF(vec2 p, float radius, float thickness) {
    return abs(length(p) - radius + thickness * 0.5) - thickness * 0.5;
}

float lineSDF(vec2 p, vec2 b, float halfThickness) {
    float h = clamp(dot(p, b) / max(dot(b, b), 1e-8), 0.0, 1.0);
    return length(p - b * h) - halfThickness;
}

void main() {
    if (iOutline > 0.5) {
        fragColor = vec4(1.0, 0.5, 0.0, 1.0);
        return;
    }

    float d;
    if      (fragShape == SHAPE_CIRCLE)       d = circleSDF(fragLocal, fragParams.x);
    else if (fragShape == SHAPE_ELLIPSE)      d = ellipseSDF(fragLocal, fragParams.xy);
    else if (fragShape == SHAPE_ROUNDED_RECT) d = roundedRectSDF(fragLocal, fragParams.xy, fragParams.z);
    else if (fragShape == SHAPE_RING)         d = ringSDF(fragLocal, fragParams.x, fragParams.y);
    else if (fragShape == SHAPE_LINE)         d = lineSDF(fragLocal, fragParams.xy, fragParams.z);
    else {
        fragColor = fragShapeColor;
        return;
    }

    float coverage = clamp(0.5 - d, 0.0, 1.0);
    if (coverage <= 0.0) discard;
    fragColor = vec4(fragShapeColor.rgb, fragShapeColor.a * coverage);
}
