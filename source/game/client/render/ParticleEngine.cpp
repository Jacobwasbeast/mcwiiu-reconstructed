#include "game/client/render/ParticleEngine.hpp"
#include "game/client/render/Particle.hpp"
#include "game/client/render/TerrainParticle.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/MemSect.hpp"
#include "game/core/Camera.hpp"
#include "game/core/Mth.hpp"
#include "game/core/TrackingEmitter.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"
#include "game/util/AABB.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/core/Direction.hpp"

#include "game/client/render/ExplodeParticle.hpp"
#include "game/client/render/SpitParticle.hpp"
#include "game/client/render/BubbleParticle.hpp"
#include "game/client/render/BubblePopParticle.hpp"
#include "game/client/render/WaterCurrentDownParticle.hpp"
#include "game/client/render/SplashParticle.hpp"
#include "game/client/render/WakeParticle.hpp"
#include "game/client/render/WaterDropParticle.hpp"
#include "game/client/render/SuspendedParticle.hpp"
#include "game/client/render/SuspendedTownParticle.hpp"
#include "game/client/render/CritParticle.hpp"
#include "game/client/render/SmokeParticle.hpp"
#include "game/client/render/LargeSmokeParticle.hpp"
#include "game/client/render/SpellParticle.hpp"
#include "game/client/render/DripParticle.hpp"
#include "game/client/render/HeartParticle.hpp"
#include "game/client/render/NoteParticle.hpp"
#include "game/client/render/NetherPortalParticle.hpp"
#include "game/client/render/EnchantmentTableParticle.hpp"
#include "game/client/render/DragonBreathParticle.hpp"
#include "game/client/render/EnderParticle.hpp"
#include "game/client/render/FlameParticle.hpp"
#include "game/client/render/LavaParticle.hpp"
#include "game/client/render/FootstepParticle.hpp"
#include "game/client/render/PlayerCloudParticle.hpp"
#include "game/client/render/RedDustParticle.hpp"
#include "game/client/render/FallingDustParticle.hpp"
#include "game/client/render/BreakingItemParticle.hpp"
#include "game/client/render/SnowShovelParticle.hpp"
#include "game/client/render/BarrierParticle.hpp"
#include "game/client/render/TerrainDustParticle.hpp"
#include "game/client/render/HugeExplosionSeedParticle.hpp"
#include "game/client/render/HugeExplosionParticle.hpp"
#include "game/client/render/FireworksParticles__Provider.hpp"
#include "game/client/render/MobAppearanceParticle.hpp"
#include "game/client/render/EndRodParticle.hpp"
#include "game/client/render/AttackSweepParticle.hpp"
#include "game/client/render/TotemParticle.hpp"
#include "game/client/render/InkParticle.hpp"
#include "game/client/render/ConduitParticle.hpp"
#include "game/client/render/ChestRefillParticle.hpp"

namespace
{
ResourceLocation particleTexture((_TEXTURE_NAME)20);
}

template <typename T>
std::wstring _toString(T value);

ParticleEngine::ParticleList::ParticleList()
    : particles(new Particle*[16384]), particleCount(0)
{
}

#if defined(RECON_NATIVE)
ParticleEngine::ParticleList::~ParticleList()
{
    ClearParticles();
    delete[] particles;
}
#endif

void ParticleEngine::ParticleList::ClearParticles()
{
    for (unsigned int i = 0; i < particleCount; ++i)
    {
        if (particles[i] != NULL)
        {
            delete particles[i];
        }
    }
    particleCount = 0;
}

Particle* ParticleEngine::ParticleList::at(int index)
{
    return particles[index];
}

unsigned int ParticleEngine::ParticleList::size()
{
    return particleCount;
}

void ParticleEngine::ParticleList::push_back(Particle* particle)
{
    particles[particleCount++] = particle;
}

void ParticleEngine::ParticleList::RemoveParticle(unsigned int index)
{
    particles[index] = particles[--particleCount];
}

bool ParticleEngine::ParticleList::empty()
{
    return particleCount == 0;
}

ParticleEngine::ParticleEngine(Level* level, Textures* textures)
    : level(level), textures(textures), translucent(false), opaque(false), particleCount(0)
{
    registerProviders();
}

void ParticleEngine::registerParticle(ePARTICLE_TYPE type, ParticleProvider* provider)
{
    providers[type] = provider;
}

void ParticleEngine::registerProviders()
{
    registerParticle(ePARTICLE_EXPLODE, new ExplodeParticle::Provider());
    registerParticle(ePARTICLE_SPIT, new SpitParticle::Provider());
    registerParticle(ePARTICLE_BUBBLE, new BubbleParticle::Provider());
    registerParticle(ePARTICLE_BUBBLE_POP, new BubblePopParticle::Provider());
    registerParticle(ePARTICLE_CURRENT_DOWN, new WaterCurrentDownParticle::Provider());
    registerParticle(ePARTICLE_SPLASH, new SplashParticle::Provider());
    registerParticle(ePARTICLE_WAKE, new WakeParticle::Provider());
    registerParticle(ePARTICLE_DROPLET, new WaterDropParticle::Provider());
    registerParticle(ePARTICLE_SUSPENDED, new SuspendedParticle::Provider());
    registerParticle(ePARTICLE_DEPTH_SUSPEND, new SuspendedTownParticle::Provider());
    registerParticle(ePARTICLE_CRIT, new CritParticle::Provider());
    registerParticle(ePARTICLE_MAGIC_CRIT, new CritParticle::MagicProvider());
    registerParticle(ePARTICLE_SMOKE, new SmokeParticle::Provider());
    registerParticle(ePARTICLE_LARGE_SMOKE, new LargeSmokeParticle::Provider());
    registerParticle(ePARTICLE_SPELL, new SpellParticle::Provider());
    registerParticle(ePARTICLE_INSTANT_SPELL, new SpellParticle::InstantProvider());
    registerParticle(ePARTICLE_MOB_SPELL, new SpellParticle::MobProvider());
    registerParticle(ePARTICLE_MOB_SPELL_AMBIENT, new SpellParticle::AmbientMobProvider());
    registerParticle(ePARTICLE_WITCH_MAGIC, new SpellParticle::WitchProvider());
    registerParticle(ePARTICLE_DRIP_WATER, new DripParticle::WaterProvider());
    registerParticle(ePARTICLE_DRIP_LAVA, new DripParticle::LavaProvider());
    registerParticle(ePARTICLE_ANGRY_VILLAGER, new HeartParticle::AngryVillagerProvider());
    registerParticle(ePARTICLE_HAPPY_VILLAGER, new SuspendedTownParticle::HappyVillagerProvider());
    registerParticle(ePARTICLE_TOWN_AURA, new SuspendedTownParticle::Provider());
    registerParticle(ePARTICLE_NOTE, new NoteParticle::Provider());
    registerParticle(ePARTICLE_PORTAL, new NetherPortalParticle::Provider());
    registerParticle(ePARTICLE_END_PORTAL, new SmokeParticle::Provider());
    registerParticle(ePARTICLE_ENCHANTMENT_TABLE, new EnchantmentTableParticle::Provider());
    registerParticle(ePARTICLE_DRAGON_BREATH, new DragonBreathParticle::Provider());
    registerParticle(ePARTICLE_ENDER, new EnderParticle::Provider());
    registerParticle(ePARTICLE_FLAME, new FlameParticle::Provider());
    registerParticle(ePARTICLE_LAVA, new LavaParticle::Provider());
    registerParticle(ePARTICLE_FOOTSTEP, new FootstepParticle::Provider());
    registerParticle(ePARTICLE_CLOUD, new PlayerCloudParticle::Provider());
    registerParticle(ePARTICLE_RED_DUST, new RedDustParticle::Provider());
    registerParticle(ePARTICLE_FALLING_DUST, new FallingDustParticle::Provider());
    registerParticle(ePARTICLE_SNOWBALL_POOF, new BreakingItemParticle::SnowballProvider());
    registerParticle(ePARTICLE_SNOW_SHOVEL, new SnowShovelParticle::Provider());
    registerParticle(ePARTICLE_SLIME, new BreakingItemParticle::SlimeProvider());
    registerParticle(ePARTICLE_HEART, new HeartParticle::Provider());
    registerParticle(ePARTICLE_BARRIER, new BarrierParticle::Provider());
    registerParticle(ePARTICLE_ICON_CRACK, new BreakingItemParticle::Provider());
    registerParticle(ePARTICLE_BLOCK_CRACK, new TerrainParticle::Provider());
    registerParticle(ePARTICLE_BLOCK_DUST, new TerrainDustParticle::Provider());
    registerParticle(ePARTICLE_HUGE_EXPLOSION, new HugeExplosionSeedParticle::Provider());
    registerParticle(ePARTICLE_LARGE_EXPLODE, new HugeExplosionParticle::Provider());
    registerParticle(ePARTICLE_FIREWORKS_SPARK, new FireworksParticles::Provider());
    registerParticle(ePARTICLE_MOB_APPEARANCE, new MobAppearanceParticle::Provider());
    registerParticle(ePARTICLE_END_ROD, new EndRodParticle::Provider());
    registerParticle(ePARTICLE_DAMAGE_INDICATOR, new CritParticle::DamageIndicatorProvider());
    registerParticle(ePARTICLE_SWEEP_ATTACK, new AttackSweepParticle::Provider());
    registerParticle(ePARTICLE_TOTEM, new TotemParticle::Provider());
    registerParticle(ePARTICLE_INK, new InkParticle::Provider());
    registerParticle(ePARTICLE_CONDUIT, new ConduitParticle::Provider());
    registerParticle(ePARTICLE_CHEST_REFILL, new ChestRefillParticle::Provider());
    registerParticle(ePARTICLE_WINNING_INSTANT_SPELL, new SpellParticle::WinningInstantProvider());
}

void ParticleEngine::createTrackingEmitter(boost::shared_ptr<Entity> entity, const ParticleType* type)
{
    createTrackingEmitter(entity, type, 3);
}

void ParticleEngine::createTrackingEmitter(boost::shared_ptr<Entity> entity, const ParticleType* type, int count)
{
    trackingEmitters.push_back(new TrackingEmitter(level, entity, type, count));
}

void ParticleEngine::add(Particle* particle)
{
    int texture = particle->getParticleTexture();
    Level* particleLevel = particle->getLevel();
    const DimensionType* dimensionType = particleLevel->mDimension->getType();
    int textureType = 0;
    if (dimensionType != DimensionType::OVERWORLD)
    {
        textureType = 2;
        if (dimensionType == DimensionType::NETHER)
        {
            textureType = 1;
        }
    }
    bool lit = particle->getAlpha() == 1.0;
    unsigned long long type = particle->GetType();
    int limit;
    if (type == 0x80221)
    {
        if (particleCount >= 2000)
        {
            delete particle;
            return;
        }
        ++particleCount;
        limit = 0x4000;
    }
    else if (type == 0x8021e)
    {
        limit = 1000;
    }
    else
    {
        limit = 0x4000;
    }
    if (particle->isSemiTransparent())
    {
        lit = false;
    }
    ParticleList& list = particles[(textureType * 5 + texture) * 2 + (lit ? 1 : 0)];
    if (type == 0x8021e)
    {
        if (list.size() >= static_cast<unsigned int>(limit))
        {
            return;
        }
    }
    else if (list.size() >= static_cast<unsigned int>(limit))
    {
        Particle* old = list.at(0);
        if (old->GetType() == 0x80221)
        {
            --particleCount;
        }
        delete old;
        list.RemoveParticle(0);
    }
    list.push_back(particle);
}

Particle* ParticleEngine::createParticle(ePARTICLE_TYPE type, double x, double y, double z,
                                         double xa, double ya, double za,
                                         arrayWithLength<int> parameters, Level* level)
{
    ParticleProvider*& provider = providers[type];
    if (provider == NULL)
    {
        return NULL;
    }
    Particle* particle = provider->createParticle(type, level, x, y, z, xa, ya, za, parameters);
    if (particle != NULL)
    {
        add(particle);
    }
    return particle;
}

void ParticleEngine::tickParticle(Particle* particle)
{
    particle->tick();
}

void ParticleEngine::tickParticleList(ParticleList* list)
{
    translucent = true;
    if (!list->empty())
    {
        unsigned int remaining = list->size();
        for (unsigned int i = 0; i < list->size() && remaining > 0; ++i, --remaining)
        {
            Particle* particle = list->at(i);
            tickParticle(particle);
            if (!particle->isAlive())
            {
                if (particle->GetType() == 0x80221)
                {
                    --particleCount;
                }
                delete particle;
                list->RemoveParticle(i--);
            }
        }
    }
    translucent = false;
}

void ParticleEngine::moveParticlesBetweenLists(ParticleList* first, ParticleList* second)
{
    translucent = true;
    for (unsigned int i = 0; i < first->size(); ++i)
    {
        Particle* particle = first->at(i);
        if (particle->getAlpha() == 1.0)
        {
            first->RemoveParticle(i);
            second->push_back(particle);
        }
    }
    for (unsigned int i = 0; i < second->size(); ++i)
    {
        Particle* particle = second->at(i);
        if (particle->getAlpha() < 1.0)
        {
            second->RemoveParticle(i);
            first->push_back(particle);
        }
    }
    translucent = false;
}

void ParticleEngine::tickTextureType(int textureType)
{
    for (int dimension = 0; dimension < 3; ++dimension)
    {
        ParticleList& first = particles[dimension * 10 + textureType * 2];
        ParticleList& second = particles[dimension * 10 + textureType * 2 + 1];
        tickParticleList(&first);
        tickParticleList(&second);
        moveParticlesBetweenLists(&first, &second);
    }
}

void ParticleEngine::tick()
{
    for (int i = 0; i < 5; ++i)
    {
        tickTextureType(i);
    }
    for (std::vector<TrackingEmitter*>::iterator it = trackingEmitters.begin(); it != trackingEmitters.end();)
    {
        TrackingEmitter* emitter = *it;
        emitter->tick();
        if (!emitter->isAlive())
        {
            it = trackingEmitters.erase(it);
            delete emitter;
        }
        else
        {
            ++it;
        }
    }
}

void ParticleEngine::render(boost::shared_ptr<Entity>& entity, float partialTick, int layer)
{
    if (opaque)
    {
        return;
    }

    const float cosYaw = gCameraXOff;
    const float cosPitch = gCameraYOff;
    const float sinYaw = gCameraZOff;
    const float cameraYOff = gCameraYOff2;
    const float cameraXOff = gCameraXOff2;

    Particle::setXOffset((entity->mX - entity->mXOld) * partialTick + entity->mXOld);
    Particle::setYOffset((entity->mY - entity->mYOld) * partialTick + entity->mYOld);
    Particle::setZOffset((entity->mZ - entity->mZOld) * partialTick + entity->mZOld);
    Particle::setCamerViewDirection(entity->getViewVector(partialTick));

    const DimensionType* dimensionType = level->mDimension->getType();
    int dimension = 0;
    if (dimensionType != DimensionType::OVERWORLD)
    {
        dimension = dimensionType == DimensionType::NETHER ? 1 : 2;
    }

    if (layer != 0)
    {
        GlStateManager::enableBlend();
        GlStateManager::blendFunc(4, 5);
        GlStateManager::alphaFunc(4, 0.003921568859368563f);
    }

    for (int textureType = 0; textureType < 5; ++textureType)
    {
        if (textureType == 3)
        {
            continue;
        }

        ParticleList& list = particles[(dimension * 5 + textureType) * 2 + layer];
        if (!list.empty())
        {
            if (layer < 2)
            {
                GlStateManager::depthMask(layer != 0);
            }

            MemSect(0x1f);
            if (textureType == 0 || textureType == 4)
            {
                textures->bindTexture(&particleTexture);
            }
            if (textureType == 1)
            {
                textures->bindTexture(&BLOCKS_LOCATION);
            }
            else if (textureType == 2)
            {
                textures->bindTexture(&ITEM_LOCATION);
            }
            MemSect(0);

            BufferBuilder* buffer = Tesselator::getInstance()->getBuilder();
            GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
            buffer->begin();
            for (unsigned int i = 0; i < list.size(); ++i)
            {
                if (buffer->hasMaxVertices())
                {
                    buffer->end();
                    buffer->begin();
                }
                list.at(i)->render(buffer, entity, partialTick, cosYaw, cosPitch, sinYaw,
                    cameraYOff, cameraXOff);
            }
            buffer->end();
        }
    }

    GlStateManager::disableBlend();
    GlStateManager::depthMask(true);
    GlStateManager::alphaFunc(4, 0.10000000149011612f);
}

void ParticleEngine::renderLit(boost::shared_ptr<Entity> entity, float partialTick, int layer)
{
    const float degToRad = 0.01745329238474369f;
    const float yaw = entity->mYRot * degToRad;
    const float pitch = entity->mXRot * degToRad;
    const float cosYaw = Mth::cos(yaw);
    const float sinYaw = Mth::sin(yaw);
    const float sinPitchY = Mth::sin(pitch);
    const float sinPitchX = Mth::sin(pitch);
    const float cosPitch = Mth::cos(pitch);
    const float cameraYOff = -sinYaw * sinPitchY;
    const float cameraXOff = cosYaw * sinPitchX;

    Particle::setXOffset((entity->mX - entity->mXOld) * partialTick + entity->mXOld);
    Particle::setYOffset((entity->mY - entity->mYOld) * partialTick + entity->mYOld);
    Particle::setZOffset((entity->mZ - entity->mZOld) * partialTick + entity->mZOld);

    Vec3* viewDirection = entity->getViewVector(partialTick);
    Particle::setCamerViewDirection(viewDirection);

    const DimensionType* dimensionType = level->mDimension->getType();
    int dimension = 0;
    if (dimensionType != DimensionType::OVERWORLD)
    {
        dimension = dimensionType == DimensionType::NETHER ? 1 : 2;
    }

    if (layer != 0)
    {
        GlStateManager::enableBlend();
        GlStateManager::blendFunc(4, 5);
        GlStateManager::alphaFunc(4, 0.003921568859368563f);
    }

    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);

    ParticleList& list = particles[dimension * 10 + 6 + layer];
    if (!list.empty())
    {
        Tesselator::getInstance();
        BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
        for (unsigned int i = 0; i < list.size(); ++i)
        {
            Particle* particle = list.at(i);
            particle->render(builder, entity, partialTick, cosYaw, cosPitch, sinYaw,
                cameraYOff, cameraXOff);
        }
        GlStateManager::disableBlend();
        GlStateManager::depthMask(true);
        GlStateManager::alphaFunc(4, 0.10000000149011612f);
    }
}

void ParticleEngine::destroy(const BlockPos& pos, const BlockState* state)
{
    Block* block = state->getBlock();
    if (block->getCraftingMaterial() != Material::AIR)
    {
        block = state->getBlock();
        const BlockState* particleState = block->fillVirtualBlockStateProperties(state, level, pos);
        for (int x = 0; x < 4; ++x)
        {
            const double xOffset = (x + 0.5) * 0.25;
            const double xVelocity = xOffset - 0.5;
            for (int y = 0; y < 4; ++y)
            {
                const double yOffset = (y + 0.5) * 0.25;
                const double yVelocity = yOffset - 0.5;
                const double xPosition = pos.x + xOffset;
                const double yPosition = pos.y + yOffset;
                const double zPosition = pos.z;

                TerrainParticle* particle = new TerrainParticle(level, xPosition, yPosition,
                    zPosition + 0.125, xVelocity, yVelocity, -0.375, particleState);
                add(particle->init(pos));

                particle = new TerrainParticle(level, xPosition, yPosition,
                    zPosition + 0.375, xVelocity, yVelocity, -0.125, particleState);
                add(particle->init(pos));

                particle = new TerrainParticle(level, xPosition, yPosition,
                    zPosition + 0.625, xVelocity, yVelocity, 0.125, particleState);
                add(particle->init(pos));

                particle = new TerrainParticle(level, xPosition, yPosition,
                    zPosition + 0.875, xVelocity, yVelocity, 0.375, particleState);
                add(particle->init(pos));
            }
        }
    }
}

void ParticleEngine::crack(const BlockPos& pos, const Direction* direction)
{
    const BlockState* state = level->getBlockState(pos);
    Block* block = state->getBlock();
    if (state->getRenderShape() == -1)
    {
        return;
    }

    const AABB* shape = block->getShape(state, level, pos);
    const double randomX = random.nextDouble();
    const double randomY = random.nextDouble();
    const double randomZ = random.nextDouble();
    double x = randomX * ((shape->mMaxX - shape->mMinX) - 0.2) + pos.x + 0.1 + shape->mMinX;
    double y = randomY * ((shape->mMaxY - shape->mMinY) - 0.2) + pos.y + 0.1 + shape->mMinY;
    double z = randomZ * ((shape->mMaxZ - shape->mMinZ) - 0.2) + pos.z + 0.1 + shape->mMinZ;

    if (direction == Direction::DOWN)
    {
        y = pos.y + shape->mMinY - 0.1;
    }
    if (direction == Direction::UP)
    {
        y = pos.y + shape->mMaxY + 0.1;
    }
    if (direction == Direction::NORTH)
    {
        z = pos.z + shape->mMinZ - 0.1;
    }
    if (direction == Direction::SOUTH)
    {
        z = pos.z + shape->mMaxZ + 0.1;
    }
    if (direction == Direction::WEST)
    {
        x = pos.x + shape->mMinX - 0.1;
    }
    if (direction == Direction::EAST)
    {
        x = pos.x + shape->mMaxX + 0.1;
    }

    TerrainParticle* particle = new TerrainParticle(level, x, y, z, 0.0, 0.0, 0.0, state);
    particle = particle->init(pos);
    particle->setPower(0.2f);
    particle->scale(0.6f);
    add(particle);
}

void ParticleEngine::setLevel(Level* newLevel)
{
    level = newLevel;
    if (level == NULL)
    {
        for (int i = 0; i < 30; ++i)
        {
            particles[i].ClearParticles();
        }
        for (std::vector<TrackingEmitter*>::iterator it = trackingEmitters.begin(); it != trackingEmitters.end(); ++it)
        {
            if (*it != NULL)
            {
                delete *it;
            }
        }
        trackingEmitters.clear();
    }
}

std::wstring ParticleEngine::countParticles()
{
    const DimensionType* dimensionType = level->mDimension->getType();
    int dimension = 0;
    if (dimensionType != DimensionType::OVERWORLD)
    {
        dimension = dimensionType == DimensionType::NETHER ? 1 : 2;
    }
    unsigned int count = 0;
    for (int i = dimension * 10; i < dimension * 10 + 10; ++i)
    {
        count += particles[i].size();
    }
    return _toString(count);
}
