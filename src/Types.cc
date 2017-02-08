/*
    Types.cc
*/

#include "Types.h"

BasicVertex::BasicVertex(   float px, float py, float pz,
                            float ux, float uy)
{
    Position = glm::vec3(px, py, pz);
    Uv = glm::vec2(ux, uy);
}
