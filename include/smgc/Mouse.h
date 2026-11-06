#pragma once

#include <glm/glm.hpp>

struct Mouse {
    glm::vec2 pos;
    glm::vec3 state;
    glm::vec3 press;
    glm::vec3 release;
    float wheelMove;

    bool operator==(const Mouse& o) const {
        return pos       == o.pos
            && state     == o.state
            && press     == o.press
            && release   == o.release
            && wheelMove == o.wheelMove;
    }

    bool operator!=(const Mouse& o) const { return !(*this == o); }
};