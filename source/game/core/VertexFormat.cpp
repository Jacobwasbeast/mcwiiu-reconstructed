#include "game/core/VertexFormat.hpp"
#include "game/core/VertexFormatElement.hpp"
#include <sstream>

VertexFormat::VertexFormat()
{
    vertexSize = 0;
    colorOffset = -1;
    normalOffset = -1;
}

VertexFormat::VertexFormat(VertexFormat* other)
{
    vertexSize = 0;
    colorOffset = -1;
    normalOffset = -1;
    for (int i = 0; i < other->getElementCount(); ++i)
    {
        addElement(other->getElement(i));
    }
}

int VertexFormat::getElementCount()
{
    return (int)elements.size();
}

const VertexFormatElement* VertexFormat::getElement(int index)
{
    return elements[index];
}

int VertexFormat::getVertexSize()
{
    return vertexSize;
}

void VertexFormat::clear()
{
    elements.clear();
    offsets.clear();
    uvOffsets.clear();
    vertexSize = 0;
    colorOffset = -1;
    normalOffset = -1;
}

bool VertexFormat::hasNormal()
{
    return normalOffset >= 0;
}

int VertexFormat::getNormalOffset()
{
    return normalOffset;
}

bool VertexFormat::hasColor()
{
    return colorOffset >= 0;
}

int VertexFormat::getColorOffset()
{
    return colorOffset;
}

bool VertexFormat::hasUv(int index)
{
    return index >= 0 && index < (int)uvOffsets.size() && uvOffsets[index] >= 0;
}

int VertexFormat::getUvOffset(int index)
{
    return uvOffsets[index];
}

std::wstring VertexFormat::toString()
{
    std::wostringstream stream;
    stream << L"VertexFormat[";
    for (int i = 0; i < getElementCount(); ++i)
    {
        if (i != 0)
        {
            stream << L", ";
        }
        stream << elements[i]->toString();
    }
    stream << L']';
    return stream.str();
}

int VertexFormat::getIntegerSize()
{
    return vertexSize / 4;
}

std::vector<const VertexFormatElement*>& VertexFormat::getElements()
{
    return elements;
}

int VertexFormat::getOffset(int index)
{
    return offsets[index];
}

bool VertexFormat::equals(VertexFormat* other)
{
    if (this == other)
    {
        return true;
    }
    if (getElementCount() != other->getElementCount())
    {
        return false;
    }
    for (int i = 0; i < getElementCount(); ++i)
    {
        if (!elements[i]->equals(const_cast<VertexFormatElement*>(other->getElement(i))))
        {
            return false;
        }
    }
    return true;
}

bool VertexFormat::hasPositionElement()
{
    for (std::vector<const VertexFormatElement*>::const_iterator it = elements.begin(); it != elements.end(); ++it)
    {
        if ((*it)->isPosition())
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
        if (newElement->getIndex() >= (int)uvOffsets.size())
        {
            uvOffsets.resize(newElement->getIndex() + 1, -1);
        }
        uvOffsets.insert(uvOffsets.begin() + newElement->getIndex(), vertexSize);
        break;
    default:
        break;
    }
    vertexSize += newElement->getByteSize();
    return this;
}
