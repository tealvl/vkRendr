#pragma once
#include "renderer.hpp"
#include "utility.hpp"

struct MeshWithTextureObj : public rendr::DrawableObj{
    rendr::Mesh<rendr::VertexPTN> mesh;
    rendr::Image* texture;
};