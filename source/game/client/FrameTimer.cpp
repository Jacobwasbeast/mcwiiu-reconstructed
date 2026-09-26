#include "game/client/FrameTimer.hpp"

#include <cstring>

void FindFrameBoundary(BcfStream* stream)
{
    if (stream->pendingBytes != 0)
    {
        std::memmove(stream->buffer,
                     stream->buffer + stream->pendingBytes,
                     stream->bufferLength - stream->pendingBytes);
        stream->bufferLength -= stream->pendingBytes;
        stream->pendingBytes = 0;
    }

    std::uint32_t candidates[8] = {};
    std::uint32_t candidateCount = 0;
    std::uint32_t available = stream->bufferLength;

    for (;;)
    {
        std::uint32_t scan = stream->scanStart;
        std::uint32_t candidateBase = 0;
        int bytesRead;

        if (available < 5)
        {
            scan += 4;
            if (available <= scan)
            {
                bytesRead = stream->read(stream->readContext,
                                         stream->buffer + available,
                                         stream->bufferCapacity - available,
                                         -1);
            }
            else
            {
                std::memmove(stream->buffer,
                             stream->buffer + scan,
                             available - scan);
                stream->bufferLength = available - scan;
                bytesRead = stream->read(stream->readContext,
                                         stream->buffer + stream->bufferLength,
                                         stream->bufferCapacity - stream->bufferLength,
                                         -1);
            }
        }
        else
        {
            scan = 4;
            candidateBase = 0x40000;
            do
            {
                std::uint8_t* header = stream->buffer + scan - 4;
                const std::uint16_t marker =
                    static_cast<std::uint16_t>(header[0]) |
                    (static_cast<std::uint16_t>(header[1]) << 8);
                const std::uint32_t frameLength =
                    static_cast<std::uint32_t>(header[2]) |
                    (static_cast<std::uint32_t>(header[3]) << 8);

                if (marker == 0x9999 && frameLength <= stream->scanStart)
                {
                    if (available < scan + frameLength + 8)
                    {
                        if (candidateCount < 8)
                        {
                            candidates[candidateCount++] =
                                frameLength | candidateBase;
                            available = stream->bufferLength;
                        }
                    }
                    else
                    {
                        std::uint8_t* nextHeader =
                            header + frameLength + 4;
                        const std::uint16_t nextMarker =
                            static_cast<std::uint16_t>(nextHeader[0]) |
                            (static_cast<std::uint16_t>(nextHeader[1]) << 8);
                        if (nextMarker == 0x9999)
                        {
                            std::memmove(stream->buffer,
                                         header,
                                         available - (scan - 4));
                            stream->frameState = 0;
                            stream->bufferLength = available - (scan - 4);
                            return;
                        }
                    }
                }

                ++candidateBase;
                candidateBase += 0x10000 - 1;
                scan += 1;
            }
            while (scan < available);

            scan = stream->scanStart + 4;
            if (scan < available)
            {
                std::memmove(stream->buffer,
                             stream->buffer + scan,
                             available - scan);
                stream->bufferLength = available - scan;
                bytesRead = stream->read(stream->readContext,
                                         stream->buffer + stream->bufferLength,
                                         stream->bufferCapacity - stream->bufferLength,
                                         -1);
            }
            else
            {
                bytesRead = stream->read(stream->readContext,
                                         stream->buffer + available,
                                         stream->bufferCapacity - available,
                                         -1);
            }
        }

        if (bytesRead == 0)
        {
            stream->frameState = 0;
            stream->bufferLength = 0;
            return;
        }

        stream->bufferLength += bytesRead;
        available = stream->bufferLength;

        for (std::uint32_t i = 0; i < candidateCount; ++i)
        {
            const std::uint32_t candidate = candidates[i];
            const std::uint32_t frameLength = candidate & 0xffff;
            const std::uint32_t candidateOffset = candidate >> 16;
            if (stream->scanStart <= candidateOffset)
            {
                const std::uint32_t frameStart =
                    candidateOffset + frameLength - stream->scanStart;
                if (frameStart + 4 >= available)
                {
                    continue;
                }
                std::uint8_t* header = stream->buffer +
                    frameStart;
                const std::uint16_t marker =
                    static_cast<std::uint16_t>(header[0]) |
                    (static_cast<std::uint16_t>(header[1]) << 8);
                if (marker == 0x9999)
                {
                    std::memmove(stream->buffer,
                                 stream->buffer + candidateOffset - 4,
                                 available - candidateOffset);
                    stream->frameState = 0;
                    stream->bufferLength -= candidateOffset;
                    return;
                }
            }
        }

    }
}
