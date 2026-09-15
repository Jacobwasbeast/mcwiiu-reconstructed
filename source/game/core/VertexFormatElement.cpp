#include "game/core/VertexFormatElement.hpp"

VertexFormatElement::Type::Type(int elementSize, const std::wstring& elementName,
                                int elementGlType, EType elementType)
    : size(elementSize), name(elementName), glType(elementGlType), type(elementType)
{
}

int VertexFormatElement::Type::getSize() const { return size; }

VertexFormatElement::Usage::Usage(const std::wstring& usageName, EUsage usageId)
    : name(usageName), usage(usageId)
{
}

int VertexFormatElement::Usage::getId() const { return static_cast<int>(usage); }

VertexFormatElement::VertexFormatElement(const VertexFormatElement& source)
    : type(source.type), usage(source.usage), index(source.index), count(source.count)
{
}

VertexFormatElement::VertexFormatElement(int elementIndex, const Type* elementType,
                                         const Usage* elementUsage, int elementCount)
    : type(elementType), usage(elementUsage), index(elementIndex), count(elementCount)
{
    if (!supportsUsage(elementIndex, elementUsage))
    {
        usage = 0;
    }
}

bool VertexFormatElement::isPosition() const { return usage == 0; }
const VertexFormatElement::Usage* VertexFormatElement::getUsage() const { return usage; }
int VertexFormatElement::getIndex() const { return index; }
int VertexFormatElement::getByteSize() const { return type->getSize() * count; }
bool VertexFormatElement::supportsUsage(int elementIndex, const Usage* elementUsage) const
{
    return elementIndex == 0 || elementUsage == 0;
}
