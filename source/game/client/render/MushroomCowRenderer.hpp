#ifndef GAME_CLIENT_RENDER_MUSHROOMCOWRENDERER_HPP
#define GAME_CLIENT_RENDER_MUSHROOMCOWRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/CowModel.hpp"

class Entity;
class EntityRenderDispatcher;
class Model;
class ModelDefinition;
class ResourceLocation;

class MushroomCowRenderer : public CowModel
{
public:
    MushroomCowRenderer(EntityRenderDispatcher* dispatcher);
    virtual ~MushroomCowRenderer();

    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual Model* createModel(const ModelDefinition& definition);
};

#endif
