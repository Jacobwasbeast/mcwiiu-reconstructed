#ifndef GAME_CORE_VERTEXFORMAT_HPP
#define GAME_CORE_VERTEXFORMAT_HPP

#include <string>
#include <vector>

class VertexFormatElement;

class VertexFormat
{
public:
    VertexFormat();
    VertexFormat(VertexFormat* other);

    void clear();
    VertexFormat* addElement(const VertexFormatElement* element);

    std::vector<const VertexFormatElement*>& getElements();
    int getElementCount();
    const VertexFormatElement* getElement(int index);
    int getOffset(int index);

    int getVertexSize();
    int getIntegerSize();

    bool hasPositionElement();
    bool hasNormal();
    int getNormalOffset();
    bool hasColor();
    int getColorOffset();
    bool hasUv(int index);
    int getUvOffset(int index);

    bool equals(VertexFormat* other);
    std::wstring toString();

private:
    std::vector<const VertexFormatElement*> elements;
    std::vector<int> offsets;
    int vertexSize;
    int colorOffset;
    std::vector<int> uvOffsets;
    int normalOffset;
};

#endif
