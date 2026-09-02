#include "game/core/VertexFormat.hpp"

#include "game/core/VertexFormatElement.hpp"

VertexFormat::VertexFormat()
{
    vertexSize = 0;
    colorOffset = -1;
    normalOffset = -1;
}

bool VertexFormat::hasPositionElement()
{
    int count = (int)elements.size();

    for (int i = 0; i < count; ++i)
    {
        if (elements[i]->isPosition())
        {
            return true;
        }
    }

    return false;
}

VertexFormat* VertexFormat::addElement(const VertexFormatElement* element)
{
    if (element->isPosition() && hasPositionElement())
    {
        return this;
    }

    VertexFormatElement* newElement = new VertexFormatElement(*element);

    elements.push_back(newElement);
    offsets.push_back(vertexSize);

    switch (newElement->getUsage()->getId())
    {
    case VertexFormatElement::Usage::eUsage_Normal:
        normalOffset = vertexSize;
        break;

    case VertexFormatElement::Usage::eUsage_Color:
        colorOffset = vertexSize;
        break;

    case VertexFormatElement::Usage::eUsage_Uv:
        uvOffsets.insert(uvOffsets.begin() + newElement->getIndex(), vertexSize);
        break;

    default:
        break;
    }

    vertexSize += newElement->getByteSize();

    delete newElement;

    return this;
}
