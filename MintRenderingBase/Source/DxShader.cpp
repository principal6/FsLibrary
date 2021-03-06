#include <stdafx.h>
#include <MintRenderingBase/Include/DxShader.h>

#include <d3dcompiler.h>

#include <MintLibrary/Include/Algorithm.hpp>

#include <MintContainer/Include/ScopeString.hpp>
#include <MintContainer/Include/StringUtil.hpp>
#include <MintContainer/Include/Vector.hpp>

#include <MintPlatform/Include/TextFile.h>
#include <MintPlatform/Include/FileUtil.hpp>

#include <MintRenderingBase/Include/GraphicDevice.h>
#include <MintRenderingBase/Include/DxShaderHeaderMemory.h>

#include <MintRenderingBase/Include/CppHlsl/Parser.h>


#pragma comment(lib, "d3dcompiler.lib")


namespace mint
{
    namespace RenderingBase
    {
#pragma region Static function definitions
        template<uint32 BufferSize>
        static void makeShaderVersion(mint::ScopeStringA<BufferSize>& out, const DxShaderType shaderType, const DxShaderVersion shaderVersion)
        {
            out.clear();

            if (shaderType == DxShaderType::VertexShader)
            {
                out.append("vs_");
            }
            else if (shaderType == DxShaderType::GeometryShader)
            {
                out.append("gs_");
            }
            else if (shaderType == DxShaderType::PixelShader)
            {
                out.append("ps_");
            }

            if (shaderVersion == DxShaderVersion::v_4_0)
            {
                out.append("4_0");
            }
            else if (shaderVersion == DxShaderVersion::v_5_0)
            {
                out.append("5_0");
            }
        }
#pragma endregion


        const DxShader DxShader::kNullInstance(nullptr, DxShaderType::VertexShader);
        DxShader::DxShader(GraphicDevice* const graphicDevice, const DxShaderType shaderType)
            : IDxObject(graphicDevice, DxObjectType::Shader), _shaderType{ shaderType }
        {
            __noop;
        }

        void DxShader::bind() const noexcept
        {
            if (_shaderType == DxShaderType::VertexShader)
            {
                _graphicDevice->getStateManager().setVsShader(static_cast<ID3D11VertexShader*>(_shader.Get()));
                _graphicDevice->getStateManager().setIaInputLayout(_inputLayout.Get());
            }
            else if (_shaderType == DxShaderType::GeometryShader)
            {
                _graphicDevice->getStateManager().setGsShader(static_cast<ID3D11GeometryShader*>(_shader.Get()));
            }
            else if (_shaderType == DxShaderType::PixelShader)
            {
                _graphicDevice->getStateManager().setPsShader(static_cast<ID3D11PixelShader*>(_shader.Get()));
            }
        }

        void DxShader::unbind() const noexcept
        {
            if (_shaderType == DxShaderType::VertexShader)
            {
                _graphicDevice->getStateManager().setVsShader(nullptr);
                _graphicDevice->getStateManager().setIaInputLayout(nullptr);
            }
            else if (_shaderType == DxShaderType::GeometryShader)
            {
                _graphicDevice->getStateManager().setGsShader(nullptr);
            }
            else if (_shaderType == DxShaderType::PixelShader)
            {
                _graphicDevice->getStateManager().setPsShader(nullptr);
            }
        }


        DxShaderPool::DxShaderPool(GraphicDevice* const graphicDevice, DxShaderHeaderMemory* const shaderHeaderMemory, const DxShaderVersion shaderVersion)
            : IDxObject(graphicDevice, DxObjectType::Pool)
            , _shaderHeaderMemory{ shaderHeaderMemory }
            , _shaderVersion{ shaderVersion }
        {
            __noop;
        }

        const DxObjectId& DxShaderPool::pushVertexShaderFromMemory(const char* const shaderIdentifier, const char* const textContent, const char* const entryPoint, const mint::CppHlsl::TypeMetaData* const inputElementTypeMetaData)
        {
            DxShader shader(_graphicDevice, DxShaderType::VertexShader);

            DxShaderCompileParam compileParam;
            compileParam._shaderIdentifier = shaderIdentifier;
            compileParam._shaderTextContent = textContent;
            if (compileShaderInternalXXX(DxShaderType::VertexShader, compileParam, entryPoint, shader._shaderBlob.ReleaseAndGetAddressOf()) == false)
            {
                return DxObjectId::kInvalidObjectId;
            }
            shader._hlslFileName = shaderIdentifier;

            return pushVertexShaderInternal(shader, inputElementTypeMetaData);
        }

        const DxObjectId& DxShaderPool::pushNonVertexShaderFromMemory(const char* const shaderIdentifier, const char* const textContent, const char* const entryPoint, const DxShaderType shaderType)
        {
            DxShader shader(_graphicDevice, shaderType);

            DxShaderCompileParam compileParam;
            compileParam._shaderIdentifier = shaderIdentifier;
            compileParam._shaderTextContent = textContent;
            if (compileShaderInternalXXX(shaderType, compileParam, entryPoint, shader._shaderBlob.ReleaseAndGetAddressOf()) == false)
            {
                return DxObjectId::kInvalidObjectId;
            }
            shader._hlslFileName = shaderIdentifier;

            return pushNonVertexShaderInternal(shader, shaderType);
        }

        const DxObjectId& DxShaderPool::pushVertexShader(const char* const inputDirectory, const char* const inputShaderFileName, const char* const entryPoint, const mint::CppHlsl::TypeMetaData* const inputElementTypeMetaData, const char* const outputDirectory)
        {
            DxShader shader(_graphicDevice, DxShaderType::VertexShader);
            if (compileShaderFromFile(inputDirectory, inputShaderFileName, entryPoint, outputDirectory, DxShaderType::VertexShader, false, shader) == false)
            {
                return DxObjectId::kInvalidObjectId;
            }
            return pushVertexShaderInternal(shader, inputElementTypeMetaData);
        }

        const DxObjectId& DxShaderPool::pushNonVertexShader(const char* const inputDirectory, const char* const inputShaderFileName, const char* const entryPoint, const DxShaderType shaderType, const char* const outputDirectory)
        {
            DxShader shader(_graphicDevice, shaderType);
            if (compileShaderFromFile(inputDirectory, inputShaderFileName, entryPoint, outputDirectory, shaderType, false, shader) == false)
            {
                return DxObjectId::kInvalidObjectId;
            }
            return pushNonVertexShaderInternal(shader, shaderType);
        }

        const DxObjectId& DxShaderPool::pushVertexShaderInternal(DxShader& shader, const mint::CppHlsl::TypeMetaData* const inputElementTypeMetaData)
        {
            if (createVertexShaderInternal(shader, inputElementTypeMetaData) == true)
            {
                shader.assignIdXXX();
                _vertexShaderArray.push_back(std::move(shader));
                return _vertexShaderArray.back().getId();
            }
            return DxObjectId::kInvalidObjectId;
        }

        const DxObjectId& DxShaderPool::pushNonVertexShaderInternal(DxShader& shader, const DxShaderType shaderType)
        {
            if (createNonVertexShaderInternal(shader, shaderType) == true)
            {
                if (shaderType == DxShaderType::GeometryShader)
                {
                    shader.assignIdXXX();
                    _geometryShaderArray.push_back(std::move(shader));
                    return _geometryShaderArray.back().getId();
                }
                else if (shaderType == DxShaderType::PixelShader)
                {
                    shader.assignIdXXX();
                    _pixelShaderArray.push_back(std::move(shader));
                    return _pixelShaderArray.back().getId();
                }
            }
            return DxObjectId::kInvalidObjectId;
        }

        const bool DxShaderPool::createVertexShaderInternal(DxShader& shader, const mint::CppHlsl::TypeMetaData* const inputElementTypeMetaData)
        {
            if (FAILED(_graphicDevice->getDxDevice()->CreateVertexShader(shader._shaderBlob->GetBufferPointer(), shader._shaderBlob->GetBufferSize(), NULL, reinterpret_cast<ID3D11VertexShader**>(shader._shader.ReleaseAndGetAddressOf()))))
            {
                return false;
            }

            // Input Layer
            if (inputElementTypeMetaData != nullptr)
            {
                shader._inputElementSet._semanticNameArray.clear();
                shader._inputElementSet._inputElementDescriptorArray.clear();

                {
                    const uint32 memberCount = inputElementTypeMetaData->getMemberCount();
                    for (uint32 memberIndex = 0; memberIndex < memberCount; ++memberIndex)
                    {
                        const CppHlsl::TypeMetaData& memberTypeMetaData = inputElementTypeMetaData->getMember(memberIndex);
                        pushInputElement(shader._inputElementSet, *inputElementTypeMetaData, memberTypeMetaData);
                    }
                }
                
                // Input slot 처리
                const uint32 slottedStreamDataCount = inputElementTypeMetaData->getSlottedStreamDataCount();
                for (uint32 slottedStreamDataIndex = 0; slottedStreamDataIndex < slottedStreamDataCount; ++slottedStreamDataIndex)
                {
                    const CppHlsl::TypeMetaData& slottedStreamData = inputElementTypeMetaData->getSlottedStreamData(slottedStreamDataIndex);
                    const uint32 memberCount = slottedStreamData.getMemberCount();
                    for (uint32 memberIndex = 0; memberIndex < memberCount; ++memberIndex)
                    {
                        pushInputElement(shader._inputElementSet, slottedStreamData, slottedStreamData.getMember(memberIndex));
                    }
                }

                if (FAILED(_graphicDevice->getDxDevice()->CreateInputLayout(&shader._inputElementSet._inputElementDescriptorArray[0], static_cast<UINT>(shader._inputElementSet._inputElementDescriptorArray.size()),
                    shader._shaderBlob->GetBufferPointer(), shader._shaderBlob->GetBufferSize(), shader._inputLayout.ReleaseAndGetAddressOf())))
                {
                    MINT_LOG_ERROR("김장원", "VertexShader [[%s]] 의 InputLayout 생성에 실패했습니다. Input 자료형 으로 [[%s]] 을 쓰는게 맞는지 확인해 주세요.", shader._hlslFileName.c_str(), inputElementTypeMetaData->getTypeName().c_str());
                    return false;
                }
            }

            return true;
        }

        void DxShaderPool::pushInputElement(DxInputElementSet& inputElementSet, const mint::CppHlsl::TypeMetaData& outerDataTypeMetaData, const mint::CppHlsl::TypeMetaData& memberTypeMetaData)
        {
            inputElementSet._semanticNameArray.push_back(CppHlsl::Parser::convertDeclarationNameToHlslSemanticName(memberTypeMetaData.getDeclName()));

            D3D11_INPUT_ELEMENT_DESC inputElementDescriptor;
            inputElementDescriptor.SemanticName = inputElementSet._semanticNameArray.back().c_str();
            inputElementDescriptor.SemanticIndex = 0;
            inputElementDescriptor.Format = CppHlsl::Parser::convertCppHlslTypeToDxgiFormat(memberTypeMetaData);
            inputElementDescriptor.InputSlot = memberTypeMetaData.getInputSlot();
            inputElementDescriptor.AlignedByteOffset = memberTypeMetaData.getByteOffset();
            inputElementDescriptor.InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
            inputElementDescriptor.InstanceDataStepRate = outerDataTypeMetaData.getInstanceDataStepRate();
            if (0 < inputElementDescriptor.InstanceDataStepRate)
            {
                inputElementDescriptor.InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_INSTANCE_DATA;
            }
            
            inputElementSet._inputElementDescriptorArray.push_back(inputElementDescriptor);
        }

        const bool DxShaderPool::createNonVertexShaderInternal(DxShader& shader, const DxShaderType shaderType)
        {
            if (shaderType == DxShaderType::GeometryShader)
            {
                if (FAILED(_graphicDevice->getDxDevice()->CreateGeometryShader(shader._shaderBlob->GetBufferPointer(), shader._shaderBlob->GetBufferSize(), NULL, reinterpret_cast<ID3D11GeometryShader**>(shader._shader.ReleaseAndGetAddressOf()))))
                {
                    return false;
                }
            }
            else if (shaderType == DxShaderType::PixelShader)
            {
                if (FAILED(_graphicDevice->getDxDevice()->CreatePixelShader(shader._shaderBlob->GetBufferPointer(), shader._shaderBlob->GetBufferSize(), NULL, reinterpret_cast<ID3D11PixelShader**>(shader._shader.ReleaseAndGetAddressOf()))))
                {
                    return false;
                }
            }
            else
            {
                return false;
            }

            return true;
        }

        const bool DxShaderPool::compileShaderFromFile(const char* const inputDirectory, const char* const inputShaderFileName, const char* const entryPoint, const char* const outputDirectory, const DxShaderType shaderType, const bool forceCompilation, DxShader& inoutShader)
        {
            std::string inputShaderFilePath{ inputDirectory };
            inputShaderFilePath += inputShaderFileName;
            if (mint::FileUtil::exists(inputShaderFilePath.c_str()) == false)
            {
                MINT_LOG_ERROR("김장원", "Input shader file not found : %s", inputShaderFilePath.c_str());
                return false;
            }

            std::string outputShaderFilePath{ inputShaderFileName };
            mint::StringUtil::excludeExtension(outputShaderFilePath);
            if (outputDirectory != nullptr)
            {
                if (mint::FileUtil::exists(outputDirectory) == false)
                {
                    MINT_ASSERT("김장원", mint::FileUtil::createDirectory(outputDirectory) == true, "경로 생성에 실패했습니다!");
                }

                outputShaderFilePath = outputDirectory + outputShaderFilePath;
            }
            else
            {
                outputShaderFilePath = inputDirectory + outputShaderFilePath;
            }
            outputShaderFilePath.append(kCompiledShaderFileExtension);

            return compileShaderFromFile(inputShaderFilePath.c_str(), entryPoint, outputShaderFilePath.c_str(), shaderType, forceCompilation, inoutShader);
        }

        const bool DxShaderPool::compileShaderFromFile(const char* const inputShaderFilePath, const char* const entryPoint, const char* const outputShaderFilePath, const DxShaderType shaderType, const bool forceCompilation, DxShader& inoutShader)
        {
            if (kStringNPos == mint::StringUtil::find(inputShaderFilePath, ".hlsl"))
            {
                return false;
            }

            if (mint::FileUtil::exists(outputShaderFilePath) == false || forceCompilation == true)
            {
                DxShaderCompileParam compileParam;
                compileParam._inputFileName = inputShaderFilePath;
                compileParam._outputFileName = outputShaderFilePath;
                if (compileShaderInternalXXX(shaderType, compileParam, entryPoint, inoutShader._shaderBlob.ReleaseAndGetAddressOf()) == false)
                {
                    return false;
                }
            }
            else
            {
                std::wstring compiledShaderFileNameWide;
                mint::StringUtil::convertStringToWideString(outputShaderFilePath, compiledShaderFileNameWide);
                if (FAILED(D3DReadFileToBlob(compiledShaderFileNameWide.c_str(), inoutShader._shaderBlob.ReleaseAndGetAddressOf())))
                {
                    return false;
                }
            }

            inoutShader._entryPoint = entryPoint;
            inoutShader._hlslFileName = inputShaderFilePath;
            inoutShader._hlslBinaryFileName = outputShaderFilePath;

            return true;
        }

        const bool DxShaderPool::compileShaderInternalXXX(const DxShaderType shaderType, const DxShaderCompileParam& compileParam, const char* const entryPoint, ID3D10Blob** outBlob)
        {
            mint::ScopeStringA<20> version;
            makeShaderVersion(version, shaderType, _shaderVersion);

            const char* content{};
            const char* identifier{};
            uint32 contentLength{};
            mint::TextFileReader textFileReader;
            if (compileParam._inputFileName != nullptr)
            {
                if (textFileReader.open(compileParam._inputFileName) == false)
                {
                    MINT_LOG_ERROR("김장원", "Input file not found : %s", compileParam._inputFileName);
                    return false;
                }

                content = textFileReader.get();
                contentLength = textFileReader.getFileSize();
                identifier = compileParam._inputFileName;
            }
            else
            {
                content = compileParam._shaderTextContent;
                contentLength = mint::StringUtil::strlen(compileParam._shaderTextContent);
                identifier = compileParam._shaderIdentifier;
            }

            const UINT debugFlag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
            if (FAILED(D3DCompile(content, contentLength, identifier, nullptr, _shaderHeaderMemory, entryPoint, version.c_str(), debugFlag, 0, outBlob, _errorMessageBlob.ReleaseAndGetAddressOf())))
            {
                reportCompileError();
                return false;
            }

            if (compileParam._outputFileName != nullptr)
            {
                std::wstring outputFileNameWideString;
                mint::StringUtil::convertStringToWideString(compileParam._outputFileName, outputFileNameWideString);
                if (FAILED(D3DWriteBlobToFile(*outBlob, outputFileNameWideString.c_str(), TRUE)))
                {
                    return false;
                }
            }

            return true;
        }

        void DxShaderPool::recompileAllShaders()
        {
            const uint32 shaderTypeCount = static_cast<uint32>(mint::RenderingBase::DxShaderType::COUNT);
            for (uint32 shaderTypeIndex = 0; shaderTypeIndex < shaderTypeCount; ++shaderTypeIndex)
            {
                const mint::RenderingBase::DxShaderType shaderType = static_cast<mint::RenderingBase::DxShaderType>(shaderTypeIndex);
                const DxObjectId objectId = _boundShaderIdArray[shaderTypeIndex];
                if (objectId.isValid() == true)
                {
                    getShader(shaderType, _boundShaderIdArray[shaderTypeIndex]).unbind();
                }
            }

            const uint32 vertexShaderCount = _vertexShaderArray.size();
            for (uint32 vertexShaderIndex = 0; vertexShaderIndex < vertexShaderCount; ++vertexShaderIndex)
            {
                DxShader& shader = _vertexShaderArray[vertexShaderIndex];
                compileShaderFromFile(shader._hlslFileName.c_str(), shader._entryPoint.c_str(), shader._hlslBinaryFileName.c_str(), shader._shaderType, true, shader);
                createVertexShaderInternal(shader, nullptr);
            }

            const uint32 geometryShaderCount = _geometryShaderArray.size();
            for (uint32 geometryShaderIndex = 0; geometryShaderIndex < geometryShaderCount; ++geometryShaderIndex)
            {
                DxShader& shader = _geometryShaderArray[geometryShaderIndex];
                compileShaderFromFile(shader._hlslFileName.c_str(), shader._entryPoint.c_str(), shader._hlslBinaryFileName.c_str(), shader._shaderType, true, shader);
                createNonVertexShaderInternal(shader, mint::RenderingBase::DxShaderType::GeometryShader);
            }

            const uint32 pixelShaderCount = _pixelShaderArray.size();
            for (uint32 pixelShaderIndex = 0; pixelShaderIndex < pixelShaderCount; ++pixelShaderIndex)
            {
                DxShader& shader = _pixelShaderArray[pixelShaderIndex];
                compileShaderFromFile(shader._hlslFileName.c_str(), shader._entryPoint.c_str(), shader._hlslBinaryFileName.c_str(), shader._shaderType, true, shader);
                createNonVertexShaderInternal(shader, mint::RenderingBase::DxShaderType::PixelShader);
            }

            for (uint32 shaderTypeIndex = 0; shaderTypeIndex < shaderTypeCount; ++shaderTypeIndex)
            {
                const mint::RenderingBase::DxShaderType shaderType = static_cast<mint::RenderingBase::DxShaderType>(shaderTypeIndex);
                const DxObjectId objectId = _boundShaderIdArray[shaderTypeIndex];
                if (objectId.isValid() == true)
                {
                    getShader(shaderType, _boundShaderIdArray[shaderTypeIndex]).bind();
                }
            }
        }

        void DxShaderPool::reportCompileError()
        {
            std::string errorMessages(reinterpret_cast<char*>(_errorMessageBlob->GetBufferPointer()));
            
            const size_t firstNewLinePos = errorMessages.find('\n');
            const size_t secondNewLinePos = errorMessages.find('\n', firstNewLinePos + 1);
            errorMessages = errorMessages.substr(0, secondNewLinePos);

            MINT_LOG_ERROR("김장원", "Shader Compile Error\n\n%s", errorMessages.c_str());
        }

        void DxShaderPool::bindShaderIfNot(const DxShaderType shaderType, const DxObjectId& objectId)
        {
            const uint32 shaderTypeIndex = static_cast<uint32>(shaderType);
            if (_boundShaderIdArray[shaderTypeIndex] != objectId)
            {
                _boundShaderIdArray[shaderTypeIndex] = objectId;

                getShader(shaderType, objectId).bind();
            }
        }

        void DxShaderPool::unbindShader(const DxShaderType shaderType)
        {
            if (shaderType == DxShaderType::COUNT)
            {
                return;
            }

            const uint32 shaderTypeIndex = static_cast<uint32>(shaderType);
            if (false == _boundShaderIdArray[shaderTypeIndex].isValid())
            {
                return;
            }

            getShader(shaderType, _boundShaderIdArray[shaderTypeIndex]).unbind();
            _boundShaderIdArray[shaderTypeIndex] = DxObjectId::kInvalidObjectId;
        }

        const DxShader& DxShaderPool::getShader(const DxShaderType shaderType, const DxObjectId& objectId)
        {
            MINT_ASSERT("김장원", objectId.isObjectType(DxObjectType::Shader) == true, "Invalid parameter - ObjectType !!");

            if (shaderType == DxShaderType::VertexShader)
            {
                const int32 index = mint::binarySearch(_vertexShaderArray, objectId);
                if (0 <= index)
                {
                    return _vertexShaderArray[index];
                }
            }
            else if (shaderType == DxShaderType::GeometryShader)
            {
                const int32 index = mint::binarySearch(_geometryShaderArray, objectId);
                if (0 <= index)
                {
                    return _geometryShaderArray[index];
                }
            }
            else if (shaderType == DxShaderType::PixelShader)
            {
                const int32 index = mint::binarySearch(_pixelShaderArray, objectId);
                if (0 <= index)
                {
                    return _pixelShaderArray[index];
                }
            }
            return DxShader::kNullInstance;
        }
    }
}
