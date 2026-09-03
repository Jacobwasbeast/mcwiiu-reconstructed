#include "game/client/render/FireballRenderer.hpp"

#include "game/util/ResourceLocation.hpp"

FireballRenderer::FireballRenderer(EntityRenderDispatcher* dispatcher,
                                   float shadowRadius)
    : EntityRenderer(dispatcher) {
    mShadowRadius = shadowRadius;
}

FireballRenderer::~FireballRenderer() {
}

void FireballRenderer::render(boost::shared_ptr<Entity>& entity, double x,
                              double y, double z, float yaw,
                              float partialTick) {
    EntityRenderer::render(entity, x, y, z, yaw, partialTick);
}

void FireballRenderer::renderFlame(boost::shared_ptr<Entity>& entity, double x,
                                   double y, double z, float partialTick) {
    EntityRenderer::renderFlame(entity, x, y, z, partialTick);
}

const ResourceLocation* FireballRenderer::getTextureLocation(
    boost::shared_ptr<Entity>& entity) {
    static ResourceLocation location;
    return &location;
}
