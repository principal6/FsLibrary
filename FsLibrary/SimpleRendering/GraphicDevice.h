﻿#pragma once


#ifndef FS_GRAPHIC_DEVICE_H
#define FS_GRAPHIC_DEVICE_H


#include <CommonDefinitions.h>

#include <Container/StaticArray.h>
#include <Container/ScopeString.h>
#include <Container/UniqueString.h>

#include <SimpleRendering/RectangleRenderer.h>
#include <SimpleRendering/DxShaderHeaderMemory.h>
#include <SimpleRendering/DxShader.h>
#include <SimpleRendering/DxBuffer.h>
#include <SimpleRendering/CppHlslStructs.h>
#include <SimpleRendering/CppHlslCbuffers.h>
#include <SimpleRendering/TriangleBuffer.h>

#include <Language/CppHlsl.h>

#include <Math/Float4x4.h>


namespace fs
{
	namespace Window
	{
		class IWindow;
	}

	namespace SimpleRendering
	{
		using Microsoft::WRL::ComPtr;


		class GraphicDevice final
		{
			friend RectangleRenderer;

		public:
																		GraphicDevice();
																		~GraphicDevice() = default;

		public:
			void														initialize(const fs::Window::IWindow* const window);

		private:
			void														createDxDevice();

		private:
			void														createFontTextureFromMemory();

		public:
			void														beginRendering();
			void														endRendering();

		public:
			FS_INLINE fs::SimpleRendering::RectangleRenderer&			getRectangleRenderer() noexcept { return _rectangleRenderer; }

		public:
			FS_INLINE ID3D11Device*										getDxDevice() noexcept { return _device.Get(); }
			FS_INLINE ID3D11DeviceContext*								getDxDeviceContext() noexcept { return _deviceContext.Get(); }

		private:
			const fs::Window::IWindow*									_window;

		private:
			float														_clearColor[4];

	#pragma region DirectX
		private:
			ComPtr<IDXGISwapChain>										_swapChain;
			ComPtr<ID3D11Device>										_device;
			ComPtr<ID3D11DeviceContext>									_deviceContext;

		private:
			ComPtr<ID3D11Texture2D>										_backBuffer;
			ComPtr<ID3D11RenderTargetView>								_backBufferRtv;

		private:
			static constexpr const char* const							kDefaultVSId{ "VSDefault" };
			static constexpr const char* const							kDefaultPSId{ "PSDefault" };

			DxShaderHeaderMemory										_shaderHeaderMemory;
			DxShaderPool												_shaderPool;
			std::unordered_map<std::string, DxObjectId>					_shaderIdMap;
			DxBufferPool												_bufferPool;

		private:
			ComPtr<ID3D11SamplerState>									_samplerState;
			ComPtr<ID3D11BlendState>									_blendState;

		private:
			static constexpr uint32										kFontTextureWidth		= 16 * kBitsPerByte;
			static constexpr uint32										kFontTextureHeight		= 60;
			static constexpr uint32										kFontTexturePixelCount	= kFontTextureWidth * kFontTextureHeight;
			std::vector<uint8>											_fontTextureRaw;
			ComPtr<ID3D11ShaderResourceView>							_fontTextureSrv;
	#pragma endregion

		private:
			fs::Language::CppHlsl										_cppHlslStructs;
			fs::Language::CppHlsl										_cppHlslCbuffers;

		private:
			SimpleRendering::TriangleBuffer<CppHlsl::VS_INPUT>			_rectangleRendererBuffer;
			fs::SimpleRendering::RectangleRenderer						_rectangleRenderer;
		};
	}
}


#endif // !FS_GRAPHIC_DEVICE_H
