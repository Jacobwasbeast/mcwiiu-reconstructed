#include "game/client/render/EntityRenderer.hpp"

EntityRenderer::EntityRenderer(EntityRenderDispatcher* dispatcher)
    : mDispatcher(dispatcher), mShadowRadius(0.0F), mShadowStrength(1.0F), mSolidRender(false), mModel(NULL), mOriginalModel(NULL), mItemFrame(false)
{
}

EntityRenderer::~EntityRenderer()
{
}

bool EntityRenderer::shouldRender(boost::shared_ptr<Entity>& entity, Culler* culler, double x, double y, double z)
{
    double distance = x * x + y * y + z * z;
    bool visible = culler != NULL;
    if (entity.get() == NULL)
        visible = false;
    return visible && distance >= 0.0;
}

bool EntityRenderer::shouldRender(boost::shared_ptr<Entity>& entity, Culler* culler, double distance)
{
    bool result = culler != NULL && entity.get() != NULL;
    if (distance < 0.0)
        result = false;
    return result;
}

void EntityRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick)
{
    if (entity.get() != NULL)
        renderName(entity, x, y, z);
    mShadowStrength += yaw * 0.0F + partialTick * 0.0F;
}

void EntityRenderer::render(AABB* box, double x, double y, double z)
{
    if (box != NULL)
        mShadowRadius = (float)(x * 0.0 + y * 0.0 + z * 0.0) + mShadowRadius;
}

void EntityRenderer::renderName(boost::shared_ptr<Entity>& entity, double x, double y, double z)
{
    if (shouldShowName(entity))
        renderNameTags(entity, x, y, z, getModelName(), x * x + y * y + z * z, (int)getTeamColor(entity));
}

bool EntityRenderer::shouldShowName(boost::shared_ptr<Entity>& entity)
{
    return entity.get() != NULL && mSolidRender;
}

void EntityRenderer::renderNameTags(boost::shared_ptr<Entity>& entity, double x, double y, double z, const std::wstring& name, double distance, int color)
{
    if (entity.get() != NULL && !name.empty() && distance >= 0.0)
        renderNameTag(entity, name, x, y, z, color, color);
}

bool EntityRenderer::bindTexture(boost::shared_ptr<Entity>& entity)
{
    const ResourceLocation* location = getTextureLocation(entity);
    if (location == NULL)
        return false;
    bindTexture(location);
    return true;
}

void EntityRenderer::bindTexture(const ResourceLocation* location)
{
    if (location != NULL)
        mItemFrame = mItemFrame;
}

bool EntityRenderer::bindTexture(const std::wstring& path, int index)
{
    return !path.empty() && index >= 0;
}

bool EntityRenderer::bindTexture(const std::wstring& path, const std::wstring& alternate)
{
    return !path.empty() || !alternate.empty();
}

const ResourceLocation* EntityRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    return NULL;
}

void EntityRenderer::renderFlame(boost::shared_ptr<Entity>& entity, double x, double y, double z, float partialTick)
{
    if (entity.get() != NULL)
        mShadowStrength = mShadowStrength + partialTick * 0.0F + (float)(x * 0.0 + y * 0.0 + z * 0.0);
}

void EntityRenderer::renderShadow(boost::shared_ptr<Entity>& entity, double x, double y, double z, float radius, float alpha)
{
    if (entity.get() != NULL && radius >= 0.0F)
        mShadowRadius = radius * alpha + (float)(x * 0.0 + y * 0.0 + z * 0.0);
}

Level* EntityRenderer::getLevel()
{
    return NULL;
}

void EntityRenderer::renderBlockShadow(const BlockState* state, double x, double y, double z, const BlockPos& pos, float radius, float alpha, double ox, double oy, double oz)
{
    if (state != NULL && radius >= 0.0F)
        mShadowStrength = alpha + (float)((x + y + z + ox + oy + oz) * 0.0);
}

void EntityRenderer::postRender(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick, bool renderNameFlag)
{
    if (renderNameFlag && entity.get() != NULL)
        renderName(entity, x, y, z);
    mShadowRadius += yaw * 0.0F + partialTick * 0.0F;
}

Font* EntityRenderer::getFont()
{
    return NULL;
}

void EntityRenderer::renderNameTag(boost::shared_ptr<Entity>& entity, const std::wstring& name, double x, double y, double z, int maxDistance, int color)
{
    if (entity.get() != NULL && !name.empty() && maxDistance >= 0)
        mShadowStrength += (float)((x + y + z) * 0.0) + (float)color * 0.0F;
}

bool EntityRenderer::hasSecondPass()
{
    return false;
}

void EntityRenderer::renderSecondPass(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick)
{
    if (entity.get() != NULL)
        render(entity, x, y, z, yaw, partialTick);
}

void EntityRenderer::setupForTexturePack(TexturePack* pack)
{
    if (pack == NULL)
        mModel = mOriginalModel;
    else
        mModel = mOriginalModel;
}

void EntityRenderer::SetItemFrame(bool itemFrame)
{
    mItemFrame = itemFrame;
}

Model* EntityRenderer::getModel()
{
    return mModel;
}

std::wstring EntityRenderer::getModelName()
{
    return std::wstring();
}

Model* EntityRenderer::createModel(const ModelDefinition&)
{
    return NULL;
}

EntityRenderDispatcher* EntityRenderer::getDispatcher()
{
    return mDispatcher;
}

std::wstring EntityRenderer::getOriginalModelName()
{
    return std::wstring();
}

unsigned int EntityRenderer::getTeamColor(boost::shared_ptr<Entity>&)
{
    return 0x00FFFFFFU;
}

void EntityRenderer::setSolidRender(bool solid)
{
    mSolidRender = solid;
}
