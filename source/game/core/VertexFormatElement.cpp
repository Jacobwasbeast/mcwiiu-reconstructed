#include "game/core/VertexFormatElement.hpp"

const VertexFormatElement::Usage* VertexFormatElement::Usage::POSITION =
    new VertexFormatElement::Usage(L"Position", VertexFormatElement::Usage::eUsage_Position);
const VertexFormatElement::Usage* VertexFormatElement::Usage::NORMAL =
    new VertexFormatElement::Usage(L"Normal", VertexFormatElement::Usage::eUsage_Normal);
const VertexFormatElement::Usage* VertexFormatElement::Usage::COLOR =
    new VertexFormatElement::Usage(L"Vertex Color", VertexFormatElement::Usage::eUsage_Color);
const VertexFormatElement::Usage* VertexFormatElement::Usage::UV =
    new VertexFormatElement::Usage(L"UV", VertexFormatElement::Usage::eUsage_Uv);
const VertexFormatElement::Usage* VertexFormatElement::Usage::BONE_MATRIX =
    new VertexFormatElement::Usage(L"Bone Matrix", VertexFormatElement::Usage::eUsage_BoneMatrix);
const VertexFormatElement::Usage* VertexFormatElement::Usage::BLEND_WEIGHT =
    new VertexFormatElement::Usage(L"Blend Weight", VertexFormatElement::Usage::eUsage_BlendWeight);
const VertexFormatElement::Usage* VertexFormatElement::Usage::PADDING =
    new VertexFormatElement::Usage(L"Padding", VertexFormatElement::Usage::eUsage_Padding);

const VertexFormatElement::Type* VertexFormatElement::Type::FLOAT =
    new VertexFormatElement::Type(4, L"Float", 0, VertexFormatElement::Type::eType_Float);
const VertexFormatElement::Type* VertexFormatElement::Type::UBYTE =
    new VertexFormatElement::Type(1, L"Unsigned Byte", 0, VertexFormatElement::Type::eType_UnsignedByte);
const VertexFormatElement::Type* VertexFormatElement::Type::BYTE =
    new VertexFormatElement::Type(1, L"Byte", 0, VertexFormatElement::Type::eType_Byte);
const VertexFormatElement::Type* VertexFormatElement::Type::USHORT =
    new VertexFormatElement::Type(2, L"Unsigned Short", 0, VertexFormatElement::Type::eType_UnsignedShort);
const VertexFormatElement::Type* VertexFormatElement::Type::SHORT =
    new VertexFormatElement::Type(2, L"Short", 0, VertexFormatElement::Type::eType_Short);
const VertexFormatElement::Type* VertexFormatElement::Type::UINT =
    new VertexFormatElement::Type(4, L"Unsigned Int", 0, VertexFormatElement::Type::eType_UnsignedInt);
const VertexFormatElement::Type* VertexFormatElement::Type::INT =
    new VertexFormatElement::Type(4, L"Int", 0, VertexFormatElement::Type::eType_Int);

VertexFormatElement::Type::Type(int size, std::wstring name, int glType, EType id)
    : size(size),
      name(name),
      glType(glType),
      id(id)
{
}

int VertexFormatElement::Type::getSize() const
{
    return size;
}

VertexFormatElement::Usage::Usage(std::wstring name, EUsage id)
    : name(name),
      id(id)
{
}

VertexFormatElement::Usage::EUsage VertexFormatElement::Usage::getId() const
{
    return id;
}

VertexFormatElement::VertexFormatElement(const VertexFormatElement& other)
    : type(other.type),
      usage(other.usage),
      index(other.index),
      count(other.count)
{
}

VertexFormatElement::VertexFormatElement(int index, const Type* type, const Usage* usage, int count)
{
    if (!supportsUsage(index, usage))
    {
        usage = Usage::UV;
    }

    this->type = type;
    this->usage = usage;
    this->index = index;
    this->count = count;
}

const VertexFormatElement::Usage* VertexFormatElement::getUsage() const
{
    return usage;
}

int VertexFormatElement::getIndex() const
{
    return index;
}

int VertexFormatElement::getByteSize() const
{
    return type->getSize() * count;
}

bool VertexFormatElement::isPosition() const
{
    return usage == Usage::POSITION;
}

bool VertexFormatElement::supportsUsage(int index, const Usage* usage) const
{
    return index == 0 || usage == Usage::UV;
}
