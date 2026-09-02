#ifndef GAME_CLIENT_RENDER_C4JRENDER_HPP
#define GAME_CLIENT_RENDER_C4JRENDER_HPP

namespace C4JRender
{
    enum eVertexType
    {
        eVertexType_Default,
        eVertexType_Terrain,
        eVertexType_DefaultLighting,
        eVertexType_TexGen,
        eVertexType_UI,
        eVertexType_ScalingGridUI,
        eVertexType_TiledUI,
        eVertexType_ColouredUI,
        eVertexType_Entity,
        eVertexType_EntityLighting,
        eVertexType_Count
    };

    enum ePixelShaderType
    {
        ePixelShaderType_Default,
        ePixelShaderType_DefaultTwoLayers,
        ePixelShaderType_DefaultThreeLayers,
        ePixelShaderType_DefaultFourLayers,
        ePixelShaderType_Terrain,
        ePixelShaderType_TerrainForceLod,
        ePixelShaderType_OrderIndependentAlpha,
        ePixelShaderType_LayeredColourMask,
        ePixelShaderType_UIColourAdd,
        ePixelShaderType_UI,
        ePixelShaderType_UIAlphaTest,
        ePixelShaderType_EntityChangeColour,
        ePixelShaderType_EntityEmissiveAlpha,
        ePixelShaderType_EntityEmissiveAlphaOnly,
        ePixelShaderType_Count
    };

    enum ePrimitiveType
    {
        ePrimitiveType_Triangles,
        ePrimitiveType_TriangleStrip,
        ePrimitiveType_TriangleFan,
        ePrimitiveType_Quads,
        ePrimitiveType_Lines,
        ePrimitiveType_LineStrip
    };

    enum eViewportType
    {
        eViewportType_FullScreen,
        eViewportType_TopHalf,
        eViewportType_BottomHalf,
        eViewportType_LeftHalf,
        eViewportType_RightHalf,
        eViewportType_TopLeftQuarter,
        eViewportType_TopRightQuarter,
        eViewportType_BottomLeftQuarter,
        eViewportType_BottomRightQuarter
    };

    enum eTextureFormat
    {
        eTextureFormat_RGBA8
    };
}

#endif
