#include <stdafx.h>
#include <MintRenderingBase/Include/FontRendererContext.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <algorithm>

#include <MintLibrary/Include/Algorithm.hpp>

#include <MintRenderingBase/Include/GraphicDevice.h>
#include <MintRenderingBase/Include/LowLevelRenderer.hpp>

#include <MintContainer/Include/Vector.hpp>
#include <MintContainer/Include/StringUtil.hpp>

#include <MintPlatform/Include/BinaryFile.hpp>
#include <MintPlatform/Include/FileUtil.hpp>

#include <MintMath/Include/Rect.h>


//#define MINT_FONT_RENDERER_SAVE_PNG_FOR_TEST
#define MINT_FONT_RENDERER_COMPRESS_AS_PNG


namespace mint
{
    namespace RenderingBase
    {
        GlyphInfo::GlyphInfo()
            : _charCode{}
            , _width{}
            , _height{}
            , _horiBearingX{}
            , _horiBearingY{}
            , _horiAdvance{}
        {
            __noop;
        }

        GlyphInfo::GlyphInfo(const wchar_t charCode, const FT_Glyph_Metrics* const ftGlyphMetrics)
            : _charCode{ charCode }
            , _width{ static_cast<GlyphMetricType>(ftGlyphMetrics->width >> 6) }
            , _height{ static_cast<GlyphMetricType>(ftGlyphMetrics->height >> 6) }
            , _horiBearingX{ static_cast<GlyphMetricType>(ftGlyphMetrics->horiBearingX >> 6) }
            , _horiBearingY{ static_cast<GlyphMetricType>(ftGlyphMetrics->horiBearingY >> 6) }
            , _horiAdvance{ static_cast<GlyphMetricType>(ftGlyphMetrics->horiAdvance >> 6) }
        {
            __noop;
        }


        GlyphRange::GlyphRange()
            : _startWchar{ 0 }
            , _endWchar{ 0 }
        {
            __noop;
        }

        GlyphRange::GlyphRange(const wchar_t startWchar, const wchar_t endWchar)
            : _startWchar{ startWchar }
            , _endWchar{ endWchar }
        {
            __noop;
        }

        const bool GlyphRange::operator<(const GlyphRange& rhs) const noexcept
        {
            return _startWchar < rhs._startWchar;
        }


        const uint32 FontRendererContext::FontData::getSafeGlyphIndex(const wchar_t wideChar) const noexcept
        {
            return (_charCodeToGlyphIndexMap.size() <= static_cast<uint32>(wideChar)) ? 0 : _charCodeToGlyphIndexMap[wideChar];
        }


        FontRendererContext::FontRendererContext(GraphicDevice* const graphicDevice)
            : FontRendererContext(graphicDevice, MINT_NEW(RenderingBase::LowLevelRenderer<RenderingBase::VS_INPUT_SHAPE>, graphicDevice))
        {
            _ownTriangleRenderer = true;
        }

        FontRendererContext::FontRendererContext(mint::RenderingBase::GraphicDevice* const graphicDevice, mint::RenderingBase::LowLevelRenderer<RenderingBase::VS_INPUT_SHAPE>* const triangleRenderer)
            : IRendererContext(graphicDevice)
            , _ftLibrary{ nullptr }
            , _ftFace{ nullptr }
            , _fontSize{ 16 }
            , _lowLevelRenderer{ triangleRenderer }
            , _ownTriangleRenderer{ false }
        {
            __noop;
        }

        FontRendererContext::~FontRendererContext()
        {
            if (_ownTriangleRenderer == true)
            {
                MINT_DELETE(_lowLevelRenderer);
            }

            deinitializeFreeType();
        }

        void FontRendererContext::pushGlyphRange(const GlyphRange& glyphRange)
        {
            _glyphRangeArray.push_back(glyphRange);

            const uint32 glyphRangeCount = _glyphRangeArray.size();
            if (2 <= glyphRangeCount)
            {
                mint::quickSort(_glyphRangeArray, mint::ComparatorAscending<mint::RenderingBase::GlyphRange>());

                mint::Vector<uint32> deletionList;
                for (uint32 glyphRangeIndex = 1; glyphRangeIndex < glyphRangeCount; ++glyphRangeIndex)
                {
                    GlyphRange& prev = _glyphRangeArray[glyphRangeIndex - 1];
                    GlyphRange& curr = _glyphRangeArray[glyphRangeIndex];

                    if (curr._startWchar <= prev._endWchar)
                    {
                        curr._startWchar = std::min(curr._startWchar, prev._startWchar);
                        curr._endWchar = std::max(curr._endWchar, prev._endWchar);

                        deletionList.push_back(glyphRangeIndex - 1);
                    }
                }

                const uint32 deletionCount = deletionList.size();
                for (uint32 deletionIndex = 0; deletionIndex < deletionCount; ++deletionIndex)
                {
                    _glyphRangeArray.erase(deletionList[deletionCount - deletionIndex - 1]);
                }
            }
        }

        const bool FontRendererContext::existsFontData(const char* const fontFileName)
        {
            std::string fontFileNameWithExtension = getFontDataFileNameWithExtension(fontFileName);
            return existsFontDataInternal(fontFileNameWithExtension.c_str());
        }

        const std::string FontRendererContext::getFontDataFileNameWithExtension(const char* const fontFileName) const noexcept
        {
            std::string fontFileNameWithExtension = fontFileName;
            mint::StringUtil::excludeExtension(fontFileNameWithExtension);
            fontFileNameWithExtension.append(kFontFileExtension);
            return fontFileNameWithExtension;
        }

        const bool FontRendererContext::existsFontDataInternal(const char* const fontFileNameWithExtension) const noexcept
        {
            return mint::FileUtil::exists(fontFileNameWithExtension);
        }

        const bool FontRendererContext::loadFontData(const char* const fontFileName)
        {
            std::string fontFileNameWithExtension = getFontDataFileNameWithExtension(fontFileName);
            if (existsFontDataInternal(fontFileNameWithExtension.c_str()) == false)
            {
                MINT_LOG_ERROR("김장원", "해당 FontFile 이 존재하지 않습니다: %s", fontFileNameWithExtension.c_str());
                return false;
            }

            mint::BinaryFileReader binaryFileReader;
            if (binaryFileReader.open(fontFileNameWithExtension.c_str()) == false)
            {
                MINT_LOG_ERROR("김장원", "해당 FontFile 을 여는 데 실패했습니다: %s", fontFileNameWithExtension.c_str());
                return false;
            }

            const char kMagicNumber[4]{ *binaryFileReader.read<char>(), *binaryFileReader.read<char>(), *binaryFileReader.read<char>(), *binaryFileReader.read<char>() };
            if (mint::StringUtil::strcmp(kMagicNumber, kFontFileMagicNumber) == false)
            {
                MINT_LOG_ERROR("김장원", "%s 파일이 아닙니다!", kFontFileMagicNumber);
                return false;
            }

            const int16 textureWidth = *binaryFileReader.read<int16>();
            const int16 textureHeight = *binaryFileReader.read<int16>();

            const uint32 glyphInfoCount = *binaryFileReader.read<uint32>();
            const uint32 charCodeToGlyphIndexMapSize = *binaryFileReader.read<uint32>();
            _fontData._glyphInfoArray.resize(glyphInfoCount);
            _fontData._charCodeToGlyphIndexMap.resize(charCodeToGlyphIndexMapSize);

            for (uint32 glyphIndex = 0; glyphIndex < glyphInfoCount; ++glyphIndex)
            {
                GlyphInfo& glyphInfo = _fontData._glyphInfoArray[glyphIndex];
                glyphInfo._charCode = *binaryFileReader.read<wchar_t>();
                glyphInfo._width = *binaryFileReader.read<GlyphMetricType>();
                glyphInfo._height = *binaryFileReader.read<GlyphMetricType>();
                glyphInfo._horiBearingX= *binaryFileReader.read<GlyphMetricType>();
                glyphInfo._horiBearingY = *binaryFileReader.read<GlyphMetricType>();
                glyphInfo._horiAdvance = *binaryFileReader.read<GlyphMetricType>();
                glyphInfo._uv0._x = *binaryFileReader.read<float>();
                glyphInfo._uv0._y = *binaryFileReader.read<float>();
                glyphInfo._uv1._x = *binaryFileReader.read<float>();
                glyphInfo._uv1._y = *binaryFileReader.read<float>();
                
                _fontData._charCodeToGlyphIndexMap[glyphInfo._charCode] = glyphIndex;
            }

            mint::Vector<byte> rawData;
#if defined MINT_FONT_RENDERER_COMPRESS_AS_PNG
            const int32 pngLength = *binaryFileReader.read<int32>();
            mint::Vector<byte> pngData(pngLength);
            for (int32 pngAt = 0; pngAt < pngLength; ++pngAt)
            {
                pngData[pngAt] = *binaryFileReader.read<byte>();
            }

            int32 width{};
            int32 height{};
            int32 comp{};
            int32 req_comp{ 1 };
            stbi_uc* const tempDataPtr = stbi_load_from_memory(&pngData[0], pngLength, &width, &height, &comp, req_comp);
            const int32 dimension = static_cast<int32>(static_cast<int64>(width) * height);
            rawData.resize(dimension);
            for (int32 at = 0; at < dimension; ++at)
            {
                rawData[at] = tempDataPtr[at];
            }
            stbi_image_free(tempDataPtr);
#else
            const uint32 pixelCount = *binaryFileReader.read<uint32>();
            rawData.resize(pixelCount);
            for (uint32 pixelIndex = 0; pixelIndex < pixelCount; ++pixelIndex)
            {
                rawData[pixelIndex] = *binaryFileReader.read<byte>();
            }
#endif
            
            mint::RenderingBase::DxResourcePool& resourcePool = _graphicDevice->getResourcePool();
            _fontData._fontTextureId = resourcePool.pushTexture2D(mint::RenderingBase::DxTextureFormat::R8_UNORM, &rawData[0], textureWidth, textureHeight);
            return true;
        }

        const bool FontRendererContext::loadFontData(const FontData& fontData)
        {
            if (fontData._fontTextureId.isValid() == false)
            {
                MINT_LOG_ERROR("김장원", "FontData 의 FontTexture 가 Invalid 합니다!");
                return false;
            }

            if (fontData._glyphInfoArray.empty() == true)
            {
                MINT_LOG_ERROR("김장원", "FontData 의 GlyphInfo 가 비어 있습니다!");
                return false;
            }

            _fontData = fontData;

            return true;
        }

        const bool FontRendererContext::bakeFontData(const char* const fontFaceFileName, const int16 fontSize, const char* const outputFileName, const int16 textureWidth, const int16 spaceLeft, const int16 spaceTop)
        {
            std::string fontFaceFileNameS = fontFaceFileName;
            if (mint::StringUtil::hasExtension(fontFaceFileNameS) == false)
            {
                fontFaceFileNameS.append(".ttf");
            }

            if (mint::FileUtil::exists(fontFaceFileNameS.c_str()) == false)
            {
                mint::StringUtil::excludeExtension(fontFaceFileNameS);
                fontFaceFileNameS.append(".otf");
            }

            if (initializeFreeType(fontFaceFileNameS.c_str(), fontSize) == false)
            {
                MINT_LOG_ERROR("김장원", "FreeType - 초기화에 실패했습니다.");
                return false;
            }

            static constexpr int16 kInitialHeight = 64;
            mint::Vector<uint8> pixelArray(static_cast<int64>(textureWidth) * kInitialHeight);

            _fontData._glyphInfoArray.clear();
            _fontData._charCodeToGlyphIndexMap.clear();

            int16 pixelX{ 0 };
            int16 pixelY{ 0 };
            wchar_t maxCharCode = 0;
            const uint32 glyphRangeCount = _glyphRangeArray.size();
            for (uint32 glyphRangeIndex = 0; glyphRangeIndex < glyphRangeCount; ++glyphRangeIndex)
            {
                const GlyphRange& glyphRange = _glyphRangeArray[glyphRangeIndex];
                maxCharCode = mint::max(maxCharCode, glyphRange._endWchar);
            }

            _fontData._charCodeToGlyphIndexMap.resize(maxCharCode + 1);

            for (uint32 glyphRangeIndex = 0; glyphRangeIndex < glyphRangeCount; ++glyphRangeIndex)
            {
                const GlyphRange& glyphRange = _glyphRangeArray[glyphRangeIndex];
                for (wchar_t wch = glyphRange._startWchar; wch <= glyphRange._endWchar; ++wch)
                {
                    bakeGlyph(wch, textureWidth, spaceLeft, spaceTop, pixelArray, pixelX, pixelY);
                }
            }

            const int32 textureHeight = static_cast<int32>(pixelArray.size() / textureWidth);
            completeGlyphInfoArray(textureWidth, textureHeight);

#if defined MINT_FONT_RENDERER_SAVE_PNG_FOR_TEST
            std::string pngFileName = outputFileName;
            mint::StringUtil::excludeExtension(pngFileName);
            pngFileName.append(".png");
            stbi_write_png(pngFileName.c_str(), textureWidth, textureHeight, 1, &pixelArray[0], textureWidth * 1);
#endif

            mint::BinaryFileWriter binaryFileWriter;
            writeMetaData(textureWidth, textureHeight, binaryFileWriter);

#if defined MINT_FONT_RENDERER_COMPRESS_AS_PNG
            int32 length{ 0 };
            unsigned char* png = stbi_write_png_to_mem(reinterpret_cast<unsigned char*>(&pixelArray[0]), textureWidth * 1, textureWidth, textureHeight, 1, &length);
            if (png == nullptr)
            {
                MINT_LOG_ERROR("김장원", "FreeType - 텍스처 정보를 추출하는 데 실패했습니다.");
                return false;
            }

            binaryFileWriter.write(length);
            for (int32 at = 0; at < length; ++at)
            {
                binaryFileWriter.write(png[at]);
            }
            STBIW_FREE(png);
#else
            const uint32 pixelCount = static_cast<uint32>(pixelArray.size());
            binaryFileWriter.write(pixelArray.size());
            for (uint32 pixelIndex = 0; pixelIndex < pixelCount; ++pixelIndex)
            {
                binaryFileWriter.write(pixelArray[pixelIndex]);
            }
#endif

            std::string outputFileNameS = outputFileName;
            mint::StringUtil::excludeExtension(outputFileNameS);
            outputFileNameS.append(kFontFileExtension);

            binaryFileWriter.save(outputFileNameS.c_str());

            return true;
        }

        const FontRendererContext::FontData& FontRendererContext::getFontData() const noexcept
        {
            return _fontData;
        }

        const int16 FontRendererContext::getFontSize() const noexcept
        {
            return _fontSize;
        }

        const bool FontRendererContext::initializeFreeType(const char* const fontFaceFileName, const int16 fontSize)
        {
            if (FT_Init_FreeType(&_ftLibrary))
            {
                MINT_LOG_ERROR("김장원", "FreeType - 라이브러리 초기화에 실패했습니다.");
                return false;
            }

            if (FT_New_Face(_ftLibrary, fontFaceFileName, 0, &_ftFace))
            {
                MINT_LOG_ERROR("김장원", "FreeType - 폰트를 읽어오는 데 실패했습니다.");
                return false;
            }

            _fontSize = fontSize;
            if (FT_Set_Pixel_Sizes(_ftFace, 0, fontSize))
            {
                MINT_LOG_ERROR("김장원", "FreeType - 폰트 크기를 지정하는 데 실패했습니다.");
                return false;
            }

            return true;
        }

        const bool FontRendererContext::deinitializeFreeType()
        {
            FT_Done_Face(_ftFace);

            FT_Done_FreeType(_ftLibrary);

            return true;
        }

        const bool FontRendererContext::bakeGlyph(const wchar_t wch, const int16 width, const int16 spaceLeft, const int16 spaceTop, mint::Vector<uint8>& pixelArray, int16& pixelPositionX, int16& pixelPositionY)
        {
            if (FT_Load_Glyph(_ftFace, FT_Get_Char_Index(_ftFace, wch), FT_LOAD_PEDANTIC | FT_LOAD_FORCE_AUTOHINT))
            {
                MINT_LOG_ERROR("김장원", "FreeType - Glyph 를 불러오는 데 실패했습니다.");
                return false;
            }

            if (FT_Render_Glyph(_ftFace->glyph, FT_RENDER_MODE_NORMAL))
            {
                MINT_LOG_ERROR("김장원", "FreeType - Glyph 를 렌더하는 데 실패했습니다.");
                return false;
            }

            const int16 rows = static_cast<int16>(_ftFace->glyph->bitmap.rows);
            const int16 cols = static_cast<int16>(_ftFace->glyph->bitmap.width);

            const int16 spacedWidth = spaceLeft + cols;
            const int16 spacedHeight = spaceTop + _fontSize + kSpaceBottom;
            if (width <= pixelPositionX + spacedWidth)
            {
                pixelPositionX = 0;
                pixelPositionY += spacedHeight;

                const int16 height = static_cast<int16>(pixelArray.size() / width);
                if (height <= pixelPositionY + spacedHeight)
                {
                    pixelArray.resize(static_cast<int32>(static_cast<int64>(width) * height * 2));
                }
            }

            for (int16 y = 0; y < rows; ++y)
            {
                for (int16 x = 0; x < cols; ++x)
                {
                    pixelArray[(spaceTop + pixelPositionY + y) * width + (spaceLeft + pixelPositionX + x)] = _ftFace->glyph->bitmap.buffer[y * cols + x];
                }
            }

            GlyphInfo glyphInfo{ wch, &_ftFace->glyph->metrics };
            glyphInfo._uv0._x = static_cast<float>(spaceLeft + pixelPositionX);
            glyphInfo._uv0._y = static_cast<float>(spaceTop + pixelPositionY);
            _fontData._glyphInfoArray.push_back(glyphInfo);
            _fontData._charCodeToGlyphIndexMap[wch] = _fontData._glyphInfoArray.size() - 1;
            
            pixelPositionX += spacedWidth;
            return true;
        }

        void FontRendererContext::completeGlyphInfoArray(const int16 textureWidth, const int16 textureHeight)
        {
            const double textureWidthF = static_cast<double>(textureWidth);
            const double textureHeightF = static_cast<double>(textureHeight);

            const uint32 glyphInfoCount = _fontData._glyphInfoArray.size();
            for (uint32 glyphIndex = 0; glyphIndex < glyphInfoCount; ++glyphIndex)
            {
                GlyphInfo& glyphInfo = _fontData._glyphInfoArray[glyphIndex];
                glyphInfo._uv0._x = static_cast<float>(static_cast<double>(glyphInfo._uv0._x) / textureWidthF);
                glyphInfo._uv0._y = static_cast<float>(static_cast<double>(glyphInfo._uv0._y) / textureHeightF);
                glyphInfo._uv1._x = static_cast<float>(static_cast<double>(glyphInfo._uv0._x) + (static_cast<double>(glyphInfo._width) / textureWidthF));
                glyphInfo._uv1._y = static_cast<float>(static_cast<double>(glyphInfo._uv0._y) + (static_cast<double>(glyphInfo._height) + kSpaceBottomForVisibility) / textureHeightF);
            }
        }

        void FontRendererContext::writeMetaData(const int16 textureWidth, const int16 textureHeight, mint::BinaryFileWriter& binaryFileWriter) const noexcept
        {
            binaryFileWriter.write("FNT");
            binaryFileWriter.write(textureWidth);
            binaryFileWriter.write(textureHeight);

            const uint32 glyphInfoCount = _fontData._glyphInfoArray.size();
            binaryFileWriter.write(glyphInfoCount);
            const uint32 charCodeToGlyphIndexMapSize = _fontData._charCodeToGlyphIndexMap.size();
            binaryFileWriter.write(charCodeToGlyphIndexMapSize);
            for (uint32 glyphIndex = 0; glyphIndex < glyphInfoCount; ++glyphIndex)
            {
                const GlyphInfo& glyphInfo = _fontData._glyphInfoArray[glyphIndex];
                binaryFileWriter.write(glyphInfo._charCode);
                binaryFileWriter.write(glyphInfo._width);
                binaryFileWriter.write(glyphInfo._height);
                binaryFileWriter.write(glyphInfo._horiBearingX);
                binaryFileWriter.write(glyphInfo._horiBearingY);
                binaryFileWriter.write(glyphInfo._horiAdvance);
                binaryFileWriter.write(glyphInfo._uv0);
                binaryFileWriter.write(glyphInfo._uv1);
            }
        }

        void FontRendererContext::initializeShaders() noexcept
        {
            _clipRect = _graphicDevice->getFullScreenClipRect();

            mint::RenderingBase::DxShaderPool& shaderPool = _graphicDevice->getShaderPool();

            // Compile vertex shader and create input layer
            {
                static constexpr const char kShaderString[]
                {
                    R"(
                    #include <ShaderStructDefinitions>
                    #include <ShaderConstantBuffers>
                    #include <ShaderStructuredBufferDefinitions>
                    
                    StructuredBuffer<SB_Transform> sbTransform : register(t0);
                    
                    VS_OUTPUT_SHAPE main(VS_INPUT_SHAPE input)
                    {
                        const uint packedInfo       = asuint(input._info.y);
                        const uint drawShade        = (packedInfo >> 30) & 3;
                        const uint transformIndex   = packedInfo & 0x3FFFFFFF;
                        
                        float4 transformedPosition = float4(input._position.xyz, 1.0);
                        transformedPosition = float4(mul(transformedPosition, sbTransform[transformIndex]._transformMatrix).xyz, 1.0);
                        transformedPosition = float4(mul(transformedPosition, _cb2DProjectionMatrix).xyz, 1.0);
                        
                        VS_OUTPUT_SHAPE result  = (VS_OUTPUT_SHAPE)0;
                        result._position        = transformedPosition;
                        result._color           = input._color;
                        result._texCoord        = input._texCoord;
                        result._info            = input._info;
                        result._info.x          = (float)drawShade;
                        result._viewportIndex   = 0;
                        return result;
                    }
                    )"
                };
                const CppHlsl::TypeMetaData& typeMetaData = _graphicDevice->getCppHlslSteamData().getTypeMetaData(typeid(mint::RenderingBase::VS_INPUT_SHAPE));
                _vertexShaderId = shaderPool.pushVertexShaderFromMemory("FontRendererVS", kShaderString, "main", &typeMetaData);
            }

            {
                static constexpr const char kShaderString[]
                {
                    R"(
                    #include <ShaderStructDefinitions>
                    
                    [maxvertexcount(3)]
                    void main(triangle VS_OUTPUT_SHAPE input[3], inout TriangleStream<VS_OUTPUT_SHAPE> OutputStream)
                    {
                        for (int i = 0; i < 3; ++i)
                        {
                            OutputStream.Append(input[i]);
                        }
                        OutputStream.RestartStrip();
                    }
                    )"
                };
                _geometryShaderId = shaderPool.pushNonVertexShaderFromMemory("FontRendererGS", kShaderString, "main", DxShaderType::GeometryShader);
            }

            // Compile pixel shader
            {
                static constexpr const char kShaderString[]
                {
                    R"(
                    #include <ShaderStructDefinitions>

                    sampler                 sampler0;
                    Texture2D<float>        texture0;
                
                    float4 main(VS_OUTPUT_SHAPE input) : SV_Target
                    {
                        const float sampled = texture0.Sample(sampler0, input._texCoord.xy);
                        float4 sampledColor = float4(input._color.xyz * ((0.0 < sampled) ? 1.0 : 0.0), sampled * input._color.a);
                        
                        const bool drawShade = (input._info.y == 1.0);
                        if (true == drawShade)
                        {
                            const float2 rbCoord = input._texCoord - float2(ddx(input._texCoord.x), ddy(input._texCoord.y));
                            const float rbSampled = texture0.Sample(sampler0, rbCoord);
                            if (0.0 < rbSampled)
                            {
                                const float3 rbColor = lerp(sampledColor.xyz * 0.25 * max(rbSampled, 0.25), sampledColor.xyz, sampled);
                                return float4(rbColor, saturate(sampled + rbSampled));
                            }
                        }
                        return sampledColor;
                    }
                    )"
                };
                _pixelShaderId = shaderPool.pushNonVertexShaderFromMemory("FontRendererPS", kShaderString, "main", DxShaderType::PixelShader);
            }
        }

        const bool FontRendererContext::hasData() const noexcept
        {
            return _lowLevelRenderer->isRenderable();
        }

        void FontRendererContext::flush() noexcept
        {
            _lowLevelRenderer->flush();

            flushTransformBuffer();
        }

        void FontRendererContext::render() noexcept
        {
            if (_lowLevelRenderer->isRenderable() == true)
            {
                prepareTransformBuffer();

                _graphicDevice->getResourcePool().bindToShader(_fontData._fontTextureId, mint::RenderingBase::DxShaderType::PixelShader, 0);

                mint::RenderingBase::DxShaderPool& shaderPool = _graphicDevice->getShaderPool();
                shaderPool.bindShaderIfNot(DxShaderType::VertexShader, _vertexShaderId);

                if (getUseMultipleViewports() == true)
                {
                    shaderPool.bindShaderIfNot(DxShaderType::GeometryShader, _geometryShaderId);
                }

                shaderPool.bindShaderIfNot(DxShaderType::PixelShader, _pixelShaderId);

                mint::RenderingBase::DxResourcePool& resourcePool = _graphicDevice->getResourcePool();
                mint::RenderingBase::DxResource& sbTransformBuffer = resourcePool.getResource(_graphicDevice->getCommonSbTransformId());
                sbTransformBuffer.bindToShader(DxShaderType::VertexShader, sbTransformBuffer.getRegisterIndex());

                _lowLevelRenderer->executeRenderCommands();

                if (getUseMultipleViewports() == true)
                {
                    shaderPool.unbindShader(DxShaderType::GeometryShader);
                }
            }
        }

        void FontRendererContext::renderAndFlush() noexcept
        {
            render();

            flush();
        }

        void FontRendererContext::drawDynamicText(const wchar_t* const wideText, const mint::Float4& position, const FontRenderingOption& fontRenderingOption)
        {
            const uint32 textLength = mint::StringUtil::wcslen(wideText);
            drawDynamicText(wideText, textLength, position, fontRenderingOption);
        }

        void FontRendererContext::drawDynamicText(const wchar_t* const wideText, const uint32 textLength, const mint::Float4& position, const FontRenderingOption& fontRenderingOption)
        {
            const float scaledTextWidth = calculateTextWidth(wideText, textLength) * fontRenderingOption._scale;
            const float scaledFontSize = _fontSize * fontRenderingOption._scale;
            
            mint::Float4 postTranslation;
            if (fontRenderingOption._directionHorz != TextRenderDirectionHorz::Rightward)
            {
                postTranslation._x -= (fontRenderingOption._directionHorz == TextRenderDirectionHorz::Centered) ? scaledTextWidth * 0.5f : scaledTextWidth;
            }
            if (fontRenderingOption._directionVert != TextRenderDirectionVert::Centered)
            {
                postTranslation._y += (fontRenderingOption._directionVert == TextRenderDirectionVert::Upward) ? -scaledFontSize * 0.5f : +scaledFontSize * 0.5f;
            }
            postTranslation._y += (-scaledFontSize * 0.5f - 1.0f);

            const uint32 vertexOffset = _lowLevelRenderer->getVertexCount();
            const uint32 indexOffset = _lowLevelRenderer->getIndexCount();

            mint::Float2 glyphPosition = mint::Float2(0.0f, 0.0f);
            for (uint32 at = 0; at < textLength; ++at)
            {
                drawGlyph(wideText[at], glyphPosition, fontRenderingOption._scale, fontRenderingOption._drawShade);
            }

            const uint32 indexCount = _lowLevelRenderer->getIndexCount() - indexOffset;
            _lowLevelRenderer->pushRenderCommandIndexed(RenderingPrimitive::TriangleList, 0, indexOffset, indexCount, _clipRect);

            const mint::Float4& preTranslation = position;
            pushTransformToBuffer(preTranslation, fontRenderingOption._transformMatrix, postTranslation);
        }

        const float FontRendererContext::calculateTextWidth(const wchar_t* const wideText, const uint32 textLength) const noexcept
        {
            int32 totalWidth = 0;
            for (uint32 textAt = 0; textAt < textLength; ++textAt)
            {
                const wchar_t& wideChar = wideText[textAt];
                
                const uint32 glyphIndex = _fontData.getSafeGlyphIndex(wideChar);
                const GlyphInfo& glyphInfo = _fontData._glyphInfoArray[glyphIndex];
                totalWidth += glyphInfo._horiAdvance;
            }
            return static_cast<float>(totalWidth);
        }

        const uint32 FontRendererContext::calculateIndexFromPositionInText(const wchar_t* const wideText, const uint32 textLength, const float positionInText) const noexcept
        {
            const int32 positionInTextInt = static_cast<int32>(positionInText);
            int32 totalWidth = 0;
            for (uint32 textAt = 0; textAt < textLength; ++textAt)
            {
                const wchar_t& wideChar = wideText[textAt];

                const uint32 glyphIndex = _fontData.getSafeGlyphIndex(wideChar);
                const GlyphInfo& glyphInfo = _fontData._glyphInfoArray[glyphIndex];
                totalWidth += glyphInfo._horiAdvance;

                if (positionInTextInt < totalWidth)
                {
                    return textAt;
                }
            }
            return textLength;
        }

        void FontRendererContext::pushTransformToBuffer(const mint::Float4& preTranslation, mint::Float4x4 transformMatrix, const mint::Float4& postTranslation)
        {
            mint::RenderingBase::SB_Transform transform;
            transform._transformMatrix.preTranslate(preTranslation.getXyz());
            transform._transformMatrix.postTranslate(postTranslation.getXyz());
            transform._transformMatrix *= transformMatrix;
            _sbTransformData.push_back(transform);
        }

        const DxObjectId& FontRendererContext::getFontTextureId() const noexcept
        {
            return _fontData._fontTextureId;
        }

        void FontRendererContext::drawGlyph(const wchar_t wideChar, mint::Float2& glyphPosition, const float scale, const bool drawShade)
        {
            const uint32 glyphIndex = _fontData.getSafeGlyphIndex(wideChar);
            const GlyphInfo& glyphInfo = _fontData._glyphInfoArray[glyphIndex];
            const float scaledFontHeight = static_cast<float>(_fontSize) * scale;
            mint::Rect glyphRect;
            glyphRect.left(glyphPosition._x + static_cast<float>(glyphInfo._horiBearingX) * scale);
            glyphRect.right(glyphRect.left() + static_cast<float>(glyphInfo._width) * scale);
            glyphRect.top(glyphPosition._y + scaledFontHeight - static_cast<float>(glyphInfo._horiBearingY) * scale);
            glyphRect.bottom(glyphRect.top() + static_cast<float>(glyphInfo._height) * scale);
            if (0.0f <= glyphRect.right() && glyphRect.left() <= _graphicDevice->getWindowSize()._x && 0.0f <= glyphRect.bottom() && glyphRect.top() <= _graphicDevice->getWindowSize()._y) // 화면을 벗어나면 렌더링 할 필요가 없으므로
            {
                auto& vertexArray = _lowLevelRenderer->vertices();

                mint::RenderingBase::VS_INPUT_SHAPE v;
                v._position._x = glyphRect.left();
                v._position._y = glyphRect.top();
                v._position._z = 0.0f;
                v._color = _defaultColor;
                v._texCoord._x = glyphInfo._uv0._x;
                v._texCoord._y = glyphInfo._uv0._y;
                //v._info._x = _viewportIndex;
                v._info._y = IRendererContext::packBits2_30AsFloat(drawShade, _sbTransformData.size());
                v._info._z = 1.0f; // used by ShapeFontRendererContext
                vertexArray.push_back(v);

                v._position._x = glyphRect.right();
                v._texCoord._x = glyphInfo._uv1._x;
                v._texCoord._y = glyphInfo._uv0._y;
                vertexArray.push_back(v);

                v._position._x = glyphRect.left();
                v._position._y = glyphRect.bottom();
                v._texCoord._x = glyphInfo._uv0._x;
                v._texCoord._y = glyphInfo._uv1._y;
                vertexArray.push_back(v);

                v._position._x = glyphRect.right();
                v._texCoord._x = glyphInfo._uv1._x;
                v._texCoord._y = glyphInfo._uv1._y;
                vertexArray.push_back(v);

                prepareIndexArray();
            }

            glyphPosition._x += static_cast<float>(glyphInfo._horiAdvance) * scale;
        }

        void FontRendererContext::prepareIndexArray()
        {
            const auto& vertexArray = _lowLevelRenderer->vertices();
            const uint32 currentTotalTriangleVertexCount = static_cast<uint32>(vertexArray.size());

            auto& indexArray = _lowLevelRenderer->indices();
            indexArray.push_back((currentTotalTriangleVertexCount - 4) + 0);
            indexArray.push_back((currentTotalTriangleVertexCount - 4) + 1);
            indexArray.push_back((currentTotalTriangleVertexCount - 4) + 2);

            indexArray.push_back((currentTotalTriangleVertexCount - 4) + 1);
            indexArray.push_back((currentTotalTriangleVertexCount - 4) + 3);
            indexArray.push_back((currentTotalTriangleVertexCount - 4) + 2);
        }
    }
}
