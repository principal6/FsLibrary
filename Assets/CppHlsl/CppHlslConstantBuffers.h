#pragma once


#ifndef FS_CPP_HLSL_CONSTANT_BUFFERS_H
#define FS_CPP_HLSL_CONSTANT_BUFFERS_H


#include <FsRenderingBase/Include/Language/CppHlslTypes.h>


namespace fs
{
	namespace RenderingBase
	{
		struct CB_Transforms CPP_HLSL_REGISTER_INDEX(0)
		{
			float4x4			_cbProjectionMatrix;
		};
	}
}


#endif // !FS_CPP_HLSL_CONSTANT_BUFFERS_H
