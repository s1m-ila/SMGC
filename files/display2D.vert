#version 460 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aLocal;
layout (location = 2) in vec4 aColor;
layout (location = 3) in vec4 aParams;
layout (location = 4) in int aShape;

out vec2 fragLocal;
flat out vec4 fragShapeColor;
flat out vec4 fragParams;
flat out int fragShape;

uniform vec2 iResolution;

void main() {
    fragLocal = aLocal;
    fragShapeColor = aColor;
    fragParams = aParams;
    fragShape = aShape;

    vec2 ndc = vec2(aPos.x / iResolution.x * 2.0 - 1.0, 1.0 - aPos.y / iResolution.y * 2.0);
    gl_Position = vec4(ndc, 0.0, 1.0);
}
