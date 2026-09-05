#include "game/client/render/ParticleEngine.hpp"
#include "game/client/render/Particle.hpp"
#include "game/client/render/TerrainParticle.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/core/Direction.hpp"

template <typename T>
std::wstring _toString(T value);

ParticleEngine::ParticleList::ParticleList()
    : particles(new Particle*[65536]), particleCount(0)
{
}

void ParticleEngine::ParticleList::ClearParticles()
{
    unsigned int count = particleCount;
    for (unsigned int i = 0; i < count; ++i)
    {
        if (particles[i] != NULL)
        {
            particles[i]->remove();
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
    providers.clear();
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
    if (particle == NULL)
    {
        return;
    }

    int texture = particle->getParticleTexture();
    int textureType = 0;
    if (level != NULL)
    {
        textureType = level->getParticleTextureType();
    }
    bool lit = particle->getAlpha() == 1.0;
    int limit = texture == 0x80221 ? 2000 : (texture == 0x8021e ? 1000 : 0x4000);
    ParticleList& list = particles[(textureType * 5 + texture) * 2 + (lit ? 1 : 0)];
    if (texture == 0x80221)
    {
        if (particleCount >= 2000)
        {
            particle->remove();
            return;
        }
        ++particleCount;
    }
    if (list.size() >= static_cast<unsigned int>(limit))
    {
        if (texture != 0x8021e && !list.empty())
        {
            Particle* old = list.at(0);
            if (old->getParticleTexture() == 0x80221)
            {
                --particleCount;
            }
            old->remove();
            list.RemoveParticle(0);
        }
        else
        {
            return;
        }
    }
    list.push_back(particle);
}

Particle* ParticleEngine::createParticle(ePARTICLE_TYPE type, double x, double y, double z,
                                         double xa, double ya, double za,
                                         arrayWithLength<int> parameters, Level* level)
{
    boost::unordered_map<ePARTICLE_TYPE, ParticleProvider*>::iterator it = providers.find(type);
    if (it == providers.end() || it->second == NULL)
    {
        return NULL;
    }
    Particle* particle = it->second->createParticle(type, level, x, y, z, xa, ya, za, parameters);
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
    opaque = true;
    if (!list->empty())
    {
        unsigned int remaining = list->size();
        for (unsigned int i = 0; i < list->size() && remaining > 0; ++i, --remaining)
        {
            Particle* particle = list->at(i);
            tickParticle(particle);
            if (!particle->isAlive())
            {
                if (particle->getParticleTexture() == 0x80221)
                {
                    --particleCount;
                }
                particle->remove();
                list->RemoveParticle(i--);
            }
        }
    }
    opaque = false;
}

void ParticleEngine::moveParticlesBetweenLists(ParticleList* first, ParticleList* second)
{
    translucent = true;
    for (unsigned int i = 0; i < first->size(); ++i)
    {
        Particle* particle = first->at(i);
        if (particle->getAlpha() == 1.0)
        {
            first->RemoveParticle(i--);
            second->push_back(particle);
        }
    }
    for (unsigned int i = 0; i < second->size(); ++i)
    {
        Particle* particle = second->at(i);
        if (particle->getAlpha() < 1.0)
        {
            second->RemoveParticle(i--);
            first->push_back(particle);
        }
    }
    translucent = false;
}

void ParticleEngine::tickTextureType(int textureType)
{
    for (int i = 0; i < 5; ++i)
    {
        ParticleList& first = particles[textureType * 10 + i * 2];
        ParticleList& second = particles[textureType * 10 + i * 2 + 1];
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
            emitter->remove();
            it = trackingEmitters.erase(it);
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
    int type = 0;
    for (int i = 0; i < particles[type * 10 + layer * 2].size(); ++i)
    {
        particles[type * 10 + layer * 2].at(i)->render(NULL, entity, partialTick, 0, 0, 0, 0, 0, 0);
    }
}

void ParticleEngine::renderLit(boost::shared_ptr<Entity> entity, float partialTick, int layer)
{
    render(entity, partialTick, layer);
}

void ParticleEngine::destroy(const BlockPos& pos, const BlockState* state)
{
    if (level == NULL || state == NULL || !state->isAir())
    {
        return;
    }
    for (int x = 0; x < 4; ++x)
    {
        for (int z = 0; z < 4; ++z)
        {
            TerrainParticle* particle = new TerrainParticle(level, pos.x + (x + 0.5) * 0.25,
                pos.y + (random.nextDouble() * 0.5 + 0.5), pos.z + (z + 0.5) * 0.25,
                (x - 1.5) * 0.25, 0, (z - 1.5) * 0.25, state);
            particle->init(pos);
            add(particle);
        }
    }
}

void ParticleEngine::crack(const BlockPos& pos, const Direction* direction)
{
    if (level == NULL)
    {
        return;
    }
    const BlockState* state = level->getBlockState(pos);
    if (state == NULL)
    {
        return;
    }
    TerrainParticle* particle = new TerrainParticle(level, pos.x + 0.5, pos.y + 0.5, pos.z + 0.5,
        0, 0, 0, state);
    particle->init(pos);
    particle->setParticleSpeed(0.2f, 0.6f);
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
            (*it)->remove();
        }
        trackingEmitters.clear();
    }
}

std::wstring ParticleEngine::countParticles()
{
    unsigned int count = 0;
    for (int i = 0; i < 30; ++i)
    {
        count += particles[i].size();
    }
    return _toString(count);
}
