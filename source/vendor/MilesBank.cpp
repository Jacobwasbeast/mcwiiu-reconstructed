#include "vendor/MilesBank.hpp"

#include "game/util/LegacyUtilityFunctions.hpp"
#include "vendor/MilesApi.hpp"
#include "vendor/MilesAsync.hpp"
#include "vendor/MilesEventExec.hpp"
#include "vendor/MilesIO.hpp"

#include <math.h>
#include <stddef.h>
#include <string.h>


namespace
{
const unsigned int BANK_WAIT_FOREVER = 0xFFFFFFFF;
const unsigned int BANK_READ_CLOSE_WHEN_DONE = 1;
const unsigned int BANK_READ_APPEND = 4;
const int BANK_READ_COMPLETE = 1;
const int BANK_HEADER_READ_LINE = 0x11e;
const int BANK_IMAGE_ALLOC_LINE = 0x147;
const int BANK_BODY_READ_LINE = 0x156;
const int BANK_PATH_ALLOC_LINE = 0x182;
const int ENVIRONMENT_ROOM_BUS = 0;
const int ENVIRONMENT_SPEAKER_COUNT = 0;
const float ENVIRONMENT_DECIBELS_PER_DECADE = 20.0f;
const double ENVIRONMENT_DECADE_BASE = 10.0;
const float DEFAULT_REVERB_TIME = 1.493f;
const float DEFAULT_REVERB_PREDELAY = 0.011f;
const float DEFAULT_REVERB_DAMPING = 0.5f;
const float DEFAULT_REVERB_DRY = 1.0f;
const float DEFAULT_REVERB_WET = 0.0f;
const float DEFAULT_MASTER_VOLUME = 1.0f;
const float DEFAULT_ROLLOFF_FACTOR = 1.0f;
const float DEFAULT_DOPPLER_FACTOR = 1.0f;
const float DEFAULT_SPEAKER_RADIUS = 3.0f;
const char HLBANK_FILE[] = "C:\\devel\\projects\\mss\\src\\sdk\\shared\\hlbank.cpp";
const char EVENT_NOT_FOUND_ERROR[] = "Event Not Found.";
const char ENVIRONMENT_NOT_FOUND_ERROR[] = "Environment Not Found.";
const char OPEN_FAILED_ERROR[] = "Failed to open sound bank.";
const char NOT_A_BANK_ERROR[] = "Sound file isn't a bank.";
const char BANK_VERSION_ERROR[] = "Bank file version doesn't match this runtime.";
const char AUDITION_NAME_ERROR[] =
    "Audition bank name provided doesn't match the bank name in the loaded file. Are you loading the "
    "bank you think you are?";

union BankHeaderBuffer
{
    SoundBankImage image;
    unsigned char bytes[BANK_HEADER_READ_SIZE];
};

EnvironmentRecord* NextRecord(EnvironmentRecord* record, int pairCount)
{
    return reinterpret_cast<EnvironmentRecord*>(record->pairs + pairCount);
}

SoundBank::AssetEntry* Relocate(SoundBankImage* bank, SoundBank::AssetEntry* offset)
{
    return reinterpret_cast<SoundBank::AssetEntry*>(reinterpret_cast<char*>(bank) +
                                                    reinterpret_cast<int>(offset));
}
}

extern "C" void AIL_close_soundbank(SoundBank* bank)
{
    if (bank != NULL)
    {
        AIL_mem_free_lock(reinterpret_cast<SoundBankImage*>(bank)->path);
        AIL_mem_free_lock(bank);
    }
}

extern "C" SoundBank* AIL_open_soundbank(const char* fileName, const char* bankName)
{
    if (fileName == NULL)
    {
        return NULL;
    }

    MilesAsyncRead request;
    BankHeaderBuffer header;

    memset(&request, 0, sizeof(request));
    request.pBuffer = &header;
    strcpy(request.szFileName, fileName);
    request.iLengthHigh = 0;
    request.iLength = BANK_HEADER_READ_SIZE;
    request.iOwnerLine = BANK_HEADER_READ_LINE;
    request.pOwnerName = HLBANK_FILE;
    request.uFlags = BANK_READ_CLOSE_WHEN_DONE;
    MilesAsyncFileRead(&request);
    if (MilesAsyncFileStatus(&request, BANK_WAIT_FOREVER) != BANK_READ_COMPLETE)
    {
        CloseAsync(&request);
        AIL_set_error(OPEN_FAILED_ERROR);
        return NULL;
    }

    if (header.image.magic != SOUND_BANK_MAGIC)
    {
        CloseAsync(&request);
        AIL_set_error(NOT_A_BANK_ERROR);
        return NULL;
    }

    if (header.image.version != SOUND_BANK_VERSION)
    {
        CloseAsync(&request);
        AIL_set_error(BANK_VERSION_ERROR);
        return NULL;
    }

    if (bankName != NULL && AIL_stricmp(header.image.name, bankName) != 0)
    {
        CloseAsync(&request);
        AIL_set_error(AUDITION_NAME_ERROR);
        return NULL;
    }

    SoundBankImage* bank = static_cast<SoundBankImage*>(
        AIL_mem_alloc_lock_info(header.image.memoryUsage, HLBANK_FILE, BANK_IMAGE_ALLOC_LINE));
    int copySize = request.iBytesRead;
    if (header.image.memoryUsage < copySize)
    {
        copySize = header.image.memoryUsage;
    }

    memcpy(bank, &header, copySize);
    if (request.iBytesRead < header.image.memoryUsage)
    {
        request.iLength = header.image.memoryUsage - request.iBytesRead;
        request.pOwnerName = HLBANK_FILE;
        request.uFlags = BANK_READ_APPEND;
        request.pBuffer = reinterpret_cast<char*>(bank) + request.iBytesRead;
        request.iOwnerLine = BANK_BODY_READ_LINE;
        request.iLengthHigh = request.iLength >> 31;
        request.iBytesRead = 0;
        MilesAsyncFileRead(&request);
        if (MilesAsyncFileStatus(&request, BANK_WAIT_FOREVER) != BANK_READ_COMPLETE)
        {
            AIL_mem_free_lock(bank);
            return NULL;
        }
    }
    else
    {
        CloseAsync(&request);
    }

    bank->events = Relocate(bank, bank->events);
    bank->environments = Relocate(bank, bank->environments);
    bank->sounds = Relocate(bank, bank->sounds);
    bank->presets = Relocate(bank, bank->presets);
    if (bank->settings != NULL)
    {
        bank->settings = reinterpret_cast<EventSettings*>(reinterpret_cast<char*>(bank) +
                                                          reinterpret_cast<int>(bank->settings));
        AIL_set_event_settings(bank->settings);
    }

    const size_t pathLength = strlen(fileName);
    bank->path = static_cast<char*>(
        AIL_mem_alloc_lock_info(pathLength + 1, HLBANK_FILE, BANK_PATH_ALLOC_LINE));
    memcpy(bank->path, fileName, pathLength + 1);
    bank->pathLength = pathLength;
    return reinterpret_cast<SoundBank*>(bank);
}

extern "C" int AIL_get_event_contents(SoundBank* bank, const char* name,
                                      const unsigned char** contents)
{
    SoundBankImage* image = reinterpret_cast<SoundBankImage*>(bank);
    if (bank != NULL && contents != NULL && name != NULL && image->magic == SOUND_BANK_MAGIC)
    {
        SoundBank::AssetEntry* entry = reinterpret_cast<SoundBank::AssetEntry*>(
            FindAsset(bank, name, image->events, image->eventCount));
        if (entry == NULL)
        {
            AIL_set_error(EVENT_NOT_FOUND_ERROR);
            return 0;
        }

        *contents = reinterpret_cast<const unsigned char*>(image) + entry->value;
        return 1;
    }

    return 0;
}

extern "C" int AIL_resolve_raw_environment_preset(void* preset, PresetEvalFunc evaluate)
{
    EnvironmentPreset* environment = static_cast<EnvironmentPreset*>(preset);
    if (environment == NULL || evaluate == NULL || environment->magic != ENVIRONMENT_PRESET_MAGIC)
    {
        return 0;
    }

    EnvironmentRecord* record = environment->records;
    EnvironmentRecord* end = reinterpret_cast<EnvironmentRecord*>(
        reinterpret_cast<char*>(environment) + environment->size);
    while (record != end)
    {
        const unsigned int kind = record->kind;
        if (kind == ENVIRONMENT_ROOM_TYPE)
        {
            record = NextRecord(record, 1);
        }
        else if (kind == ENVIRONMENT_MASTER_REVERB)
        {
            for (int i = 0; i < 3; ++i)
            {
                if (record->pairs[i].expression != 0)
                {
                    evaluate(NULL,
                             reinterpret_cast<const char*>(environment) + record->pairs[i].expression,
                             -1, &record->pairs[i].value, 1);
                }
            }

            record = NextRecord(record, 3);
        }
        else if (kind == ENVIRONMENT_MASTER_REVERB_LEVELS)
        {
            for (int i = 0; i < 2; ++i)
            {
                if (record->pairs[i].expression != 0)
                {
                    evaluate(NULL,
                             reinterpret_cast<const char*>(environment) + record->pairs[i].expression,
                             -1, &record->pairs[i].value, 1);
                }
            }

            record = NextRecord(record, 2);
        }
        else
        {
            if (kind < ENVIRONMENT_KIND_COUNT && record->pairs[0].expression != 0)
            {
                evaluate(NULL,
                         reinterpret_cast<const char*>(environment) + record->pairs[0].expression, -1,
                         &record->pairs[0].value, 1);
            }

            record = NextRecord(record, 1);
        }
    }

    return 1;
}

extern "C" int AIL_apply_raw_environment_preset(_DIG_DRIVER* driver, void* preset)
{
    EnvironmentPreset* environment = static_cast<EnvironmentPreset*>(preset);
    if (driver == NULL || environment == NULL || environment->magic != ENVIRONMENT_PRESET_MAGIC)
    {
        return 0;
    }

    EnvironmentRecord* record = environment->records;
    EnvironmentRecord* end = reinterpret_cast<EnvironmentRecord*>(
        reinterpret_cast<char*>(environment) + environment->size);
    while (record != end)
    {
        switch (record->kind)
        {
        case ENVIRONMENT_ROOM_TYPE:
            AIL_set_room_type(driver, ENVIRONMENT_ROOM_BUS, record->pairs[0].value.integer);
            record = NextRecord(record, 1);
            break;
        case ENVIRONMENT_MASTER_REVERB:
            AIL_set_digital_master_reverb(driver, ENVIRONMENT_ROOM_BUS, record->pairs[0].value.real,
                                          record->pairs[1].value.real, record->pairs[2].value.real);
            record = NextRecord(record, 3);
            break;
        case ENVIRONMENT_MASTER_REVERB_LEVELS:
        {
            const float dry = static_cast<float>(
                pow(ENVIRONMENT_DECADE_BASE,
                    static_cast<double>(record->pairs[1].value.real / ENVIRONMENT_DECIBELS_PER_DECADE)));
            const float wet = static_cast<float>(
                pow(ENVIRONMENT_DECADE_BASE,
                    static_cast<double>(record->pairs[0].value.real / ENVIRONMENT_DECIBELS_PER_DECADE)));
            AIL_set_digital_master_reverb_levels(driver, ENVIRONMENT_ROOM_BUS, dry, wet);
            record = NextRecord(record, 2);
            break;
        }
        case ENVIRONMENT_MASTER_VOLUME:
        {
            const float volume = static_cast<float>(
                pow(ENVIRONMENT_DECADE_BASE,
                    static_cast<double>(record->pairs[0].value.real / ENVIRONMENT_DECIBELS_PER_DECADE)));
            AIL_set_digital_master_volume_level(driver, volume);
            record = NextRecord(record, 1);
            break;
        }
        case ENVIRONMENT_ROLLOFF_FACTOR:
            AIL_set_3D_rolloff_factor(driver, record->pairs[0].value.real);
            record = NextRecord(record, 1);
            break;
        case ENVIRONMENT_DOPPLER_FACTOR:
            AIL_set_3D_doppler_factor(driver, record->pairs[0].value.real);
            record = NextRecord(record, 1);
            break;
        case ENVIRONMENT_SPEAKER_RADIUS:
            AIL_set_speaker_configuration(driver, NULL, ENVIRONMENT_SPEAKER_COUNT,
                                          record->pairs[0].value.real);
            record = NextRecord(record, 1);
            break;
        default:
            record = NextRecord(record, 1);
            break;
        }
    }

    return 1;
}

extern "C" void* AIL_find_environment_preset(SoundBank* bank, const char* name)
{
    SoundBankImage* image = reinterpret_cast<SoundBankImage*>(bank);
    if (name != NULL && bank != NULL && image->magic == SOUND_BANK_MAGIC)
    {
        SoundBank::AssetEntry* entry = reinterpret_cast<SoundBank::AssetEntry*>(
            FindAsset(bank, name, image->environments, image->environmentCount));
        if (entry == NULL)
        {
            AIL_set_error(ENVIRONMENT_NOT_FOUND_ERROR);
            return NULL;
        }

        return reinterpret_cast<char*>(image) + entry->value;
    }

    return NULL;
}

extern "C" int AIL_unapply_raw_environment_preset(_DIG_DRIVER* driver, void* preset)
{
    EnvironmentPreset* environment = static_cast<EnvironmentPreset*>(preset);
    if (driver == NULL || environment == NULL || environment->magic != ENVIRONMENT_PRESET_MAGIC)
    {
        return 0;
    }

    EnvironmentRecord* record = environment->records;
    EnvironmentRecord* end = reinterpret_cast<EnvironmentRecord*>(
        reinterpret_cast<char*>(environment) + environment->size);
    while (record != end)
    {
        switch (record->kind)
        {
        case ENVIRONMENT_ROOM_TYPE:
            AIL_set_room_type(driver, ENVIRONMENT_ROOM_BUS, 0);
            record = NextRecord(record, 1);
            break;
        case ENVIRONMENT_MASTER_REVERB:
            AIL_set_digital_master_reverb(driver, ENVIRONMENT_ROOM_BUS, DEFAULT_REVERB_TIME,
                                          DEFAULT_REVERB_PREDELAY, DEFAULT_REVERB_DAMPING);
            record = NextRecord(record, 3);
            break;
        case ENVIRONMENT_MASTER_REVERB_LEVELS:
            AIL_set_digital_master_reverb_levels(driver, ENVIRONMENT_ROOM_BUS, DEFAULT_REVERB_DRY,
                                                 DEFAULT_REVERB_WET);
            record = NextRecord(record, 2);
            break;
        case ENVIRONMENT_MASTER_VOLUME:
            AIL_set_digital_master_volume_level(driver, DEFAULT_MASTER_VOLUME);
            record = NextRecord(record, 1);
            break;
        case ENVIRONMENT_ROLLOFF_FACTOR:
            AIL_set_3D_rolloff_factor(driver, DEFAULT_ROLLOFF_FACTOR);
            record = NextRecord(record, 1);
            break;
        case ENVIRONMENT_DOPPLER_FACTOR:
            AIL_set_3D_doppler_factor(driver, DEFAULT_DOPPLER_FACTOR);
            record = NextRecord(record, 1);
            break;
        case ENVIRONMENT_SPEAKER_RADIUS:
            AIL_set_speaker_configuration(driver, NULL, ENVIRONMENT_SPEAKER_COUNT,
                                          DEFAULT_SPEAKER_RADIUS);
            record = NextRecord(record, 1);
            break;
        default:
            record = NextRecord(record, 1);
            break;
        }
    }

    return 1;
}

extern "C" int AIL_get_soundbank_mem_usage(SoundBank* bank)
{
    SoundBankImage* image = reinterpret_cast<SoundBankImage*>(bank);
    int usage = 0;
    if (image->magic == SOUND_BANK_MAGIC)
    {
        usage = image->memoryUsage;
    }

    return usage;
}
