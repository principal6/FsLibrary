#include <stdafx.h>
#include <SimpleRendering\DxShader.h>

#include <Algorithm.hpp>
#include <Container\ScopeString.hpp>
#include <SimpleRendering\DxHelper.h>
#include <SimpleRendering\DxHelper.hpp>
#include <SimpleRendering\GraphicDevice.h>
#include <SimpleRendering\DxShaderHeaderMemory.h>

#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")


namespace fs
{
#pragma region Static function definitions
	template<uint32 BufferSize>
	static void makeShaderVersion(fs::ScopeStringA<BufferSize>& out, const DxShaderType shaderType, const DxShaderVersion shaderVersion)
	{
		out.clear();

		if (shaderType == DxShaderType::VertexShader)
		{
			out.append("vs_");
		}
		else if (shaderType == DxShaderType::PixelShader)
		{
			out.append("ps_");
		}

		if (shaderVersion == DxShaderVersion::v_4_0)
		{
			out.append("4_0");
		}
	}
#pragma endregion


	const DxShader DxShader::kNullInstance(nullptr, DxShaderType::VertexShader);
	DxShader::DxShader(GraphicDevice* const graphicDevice, const DxShaderType shaderType)
		: IDxObject(graphicDevice), _shaderType{ shaderType }
	{
		__noop;
	}

	void DxShader::bind() const noexcept
	{
		if (_shaderType == DxShaderType::VertexShader)
		{
			_graphicDevice->getDxDeviceContext()->VSSetShader(static_cast<ID3D11VertexShader*>(_shader.Get()), nullptr, 0);
			_graphicDevice->getDxDeviceContext()->IASetInputLayout(_inputLayout.Get());
		}
		else if (_shaderType == DxShaderType::PixelShader)
		{
			_graphicDevice->getDxDeviceContext()->PSSetShader(static_cast<ID3D11PixelShader*>(_shader.Get()), nullptr, 0);
		}
	}


	DxShaderPool::DxShaderPool(GraphicDevice* const graphicDevice, DxShaderHeaderMemory* const shaderHeaderMemory)
		: IDxObject(graphicDevice), _shaderHeaderMemory{ shaderHeaderMemory }
	{
		__noop;
	}

	const DxObjectId& DxShaderPool::pushVertexShader(const char* const content, const char* const entryPoint, const DxShaderVersion shaderVersion, fs::IReflective* const inputElementClass)
	{
		DxShader shader(_graphicDevice, DxShaderType::VertexShader);

		fs::ScopeStringA<20> version;
		makeShaderVersion(version, DxShaderType::VertexShader, shaderVersion);
		if (FAILED(D3DCompile(content, strlen(content), nullptr, nullptr, _shaderHeaderMemory, entryPoint, version.c_str(), 0, 0, shader._shaderBlob.ReleaseAndGetAddressOf(), nullptr)))
		{
			return DxObjectId::kInvalidObjectId;
		}

		if (FAILED(_graphicDevice->getDxDevice()->CreateVertexShader(shader._shaderBlob->GetBufferPointer(), shader._shaderBlob->GetBufferSize(), NULL, reinterpret_cast<ID3D11VertexShader**>(shader._shader.ReleaseAndGetAddressOf()))))
		{
			return DxObjectId::kInvalidObjectId;
		}

		// Input Layer
		const uint32 memberCount = inputElementClass->getMemberCount();
		shader._inputElementSet._semanticNameArray.reserve(memberCount);
		shader._inputElementSet._inputElementDescriptorArray.reserve(memberCount);
		for (uint32 memberIndex = 0; memberIndex < memberCount; memberIndex++)
		{
			const fs::ReflectionTypeData& memberType = inputElementClass->getMemberType(memberIndex);
			shader._inputElementSet._semanticNameArray.emplace_back(fs::convertDeclarationNameToHlslSemanticName(memberType.declarationName()));

			D3D11_INPUT_ELEMENT_DESC inputElementDescriptor;
			inputElementDescriptor.SemanticName = shader._inputElementSet._semanticNameArray[memberIndex].c_str();
			inputElementDescriptor.SemanticIndex = 0;
			inputElementDescriptor.Format = fs::convertToDxgiFormat(memberType);
			inputElementDescriptor.InputSlot = 0;
			inputElementDescriptor.AlignedByteOffset = memberType.byteOffset();
			inputElementDescriptor.InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
			inputElementDescriptor.InstanceDataStepRate = 0;
			shader._inputElementSet._inputElementDescriptorArray.emplace_back(inputElementDescriptor);
		}
		if (FAILED(_graphicDevice->getDxDevice()->CreateInputLayout(&shader._inputElementSet._inputElementDescriptorArray[0], static_cast<UINT>(shader._inputElementSet._inputElementDescriptorArray.size()),
			shader._shaderBlob->GetBufferPointer(), shader._shaderBlob->GetBufferSize(), shader._inputLayout.ReleaseAndGetAddressOf())))
		{
			return DxObjectId::kInvalidObjectId;
		}

		shader.assignIdXXX();
		_vertexShaderArray.emplace_back(std::move(shader));
		return _vertexShaderArray.back().getId();
	}

	const DxObjectId& DxShaderPool::pushNonVertexShader(const char* const content, const char* const entryPoint, const DxShaderVersion shaderVersion, const DxShaderType shaderType)
	{
		DxShader shader(_graphicDevice, shaderType);

		fs::ScopeStringA<20> version;
		makeShaderVersion(version, shaderType, shaderVersion);
		if (FAILED(D3DCompile(content, strlen(content), nullptr, nullptr, _shaderHeaderMemory, entryPoint, version.c_str(), 0, 0, shader._shaderBlob.ReleaseAndGetAddressOf(), nullptr)))
		{
			return DxObjectId::kInvalidObjectId;
		}
		
		if (shaderType == DxShaderType::PixelShader)
		{
			if (FAILED(_graphicDevice->getDxDevice()->CreatePixelShader(shader._shaderBlob->GetBufferPointer(), shader._shaderBlob->GetBufferSize(), NULL, reinterpret_cast<ID3D11PixelShader**>(shader._shader.ReleaseAndGetAddressOf()))))
			{
				return DxObjectId::kInvalidObjectId;
			}

			shader.assignIdXXX();
			_pixelShaderArray.emplace_back(std::move(shader));
			return _pixelShaderArray.back().getId();
		}

		return DxObjectId::kInvalidObjectId;
	}

	const DxShader& DxShaderPool::getShader(const DxShaderType shaderType, const DxObjectId& objectId)
	{
		if (shaderType == DxShaderType::VertexShader)
		{
			const uint32 index = fs::binarySearch(_vertexShaderArray, objectId);
			if (index != kUint32Max)
			{
				return _vertexShaderArray[index];
			}
		}
		else if (shaderType == DxShaderType::PixelShader)
		{
			const uint32 index = fs::binarySearch(_pixelShaderArray, objectId);
			if (index != kUint32Max)
			{
				return _pixelShaderArray[index];
			}
		}
		return DxShader::kNullInstance;
	}

}