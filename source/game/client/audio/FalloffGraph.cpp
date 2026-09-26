#include "game/client/audio/FalloffGraph.hpp"
#include "game/core/PODHandleArray.hpp"

#include <stddef.h>

struct HandleEntry
{
    int size;
    unsigned int generation;
    void* data;
};

static unsigned int sHandleCount = 0;
static unsigned char* sHandleTableEnd = NULL;

void* Handle_Get(Handle handle)
{
    if (handle.index != 0 && handle.index <= sHandleCount)
    {
        const HandleEntry* entry = reinterpret_cast<const HandleEntry*>(
            sHandleTableEnd - handle.index * sizeof(HandleEntry));
        if (entry->generation == handle.generation)
        {
            return entry->data;
        }
    }

    return NULL;
}

void AIL_set_sample_3D_volume_falloff(_SAMPLE* sample, const FalloffGraphPoint* points, int count);
void AIL_set_sample_3D_lowpass_falloff(_SAMPLE* sample, const FalloffGraphPoint* points, int count);
void AIL_set_sample_3D_exclusion_falloff(_SAMPLE* sample, const FalloffGraphPoint* points, int count);

void FalloffGraph_Set(_SAMPLE* sample, int flags, FalloffGraph* graph)
{
    if (Handle_Get(sample->falloffHandle) == NULL)
    {
        return;
    }

    if (graph == NULL)
    {
        if ((flags & 1) != 0)
        {
            AIL_set_sample_3D_volume_falloff(sample, NULL, 0);
        }
        if ((flags & 2) != 0)
        {
            AIL_set_sample_3D_lowpass_falloff(sample, NULL, 0);
        }
        if ((flags & 4) != 0)
        {
            AIL_set_sample_3D_exclusion_falloff(sample, NULL, 0);
        }
    }

    sample->falloffMode = graph->mode;

    if ((flags & 1) != 0)
    {
        AIL_set_sample_3D_volume_falloff(sample, graph->points, graph->volumePointCount);
    }
    if ((flags & 2) != 0)
    {
        AIL_set_sample_3D_lowpass_falloff(sample, graph->points + graph->volumePointCount,
                                          graph->lowpassPointCount);
    }
    if ((flags & 4) != 0)
    {
        AIL_set_sample_3D_exclusion_falloff(
            sample, graph->points + graph->volumePointCount + graph->lowpassPointCount,
            graph->exclusionPointCount);
    }

    sample->falloffApplied = 1;
}
