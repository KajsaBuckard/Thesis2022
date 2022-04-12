//http://cwyman.org/code/dxrTutors/tutors/Tutor4/tutorial04.md.html

//#include "../../Falcor/Framework/Source/Data/HostDeviceSharedMacros.h"

#include "../../Falcor/Framework/Source/Data/HostDeviceSharedMacros.h"

// Include and import common Falcor utilities and data structures
__import Raytracing;
__import ShaderCommon;
__import Shading;                      // Shading functions, etc
__import Lights;                       // Light structures for our current scene

// A separate file with some simple utility functions: getPerpendicularVector(), initRand(), nextRand()
#include "../../HybridRenderingPipeline/Data/commonUtils.hlsli"
#include "halton.hlsli"

//#include "../../HybridRenderingPipeline/Data/commonUtils.hlsli"

[shader("raygeneration")]
void GBufferRayGen()
{
	// Convert our ray index into a ray direction in world space. 
	float2 currenPixelLocation = DispatchRaysIndex() + float2(0.5f, 0.5f);
	float2 pixelCenter = currenPixelLocation / DispatchRaysDimensions();
	float2 ndc = float2(2, -2) * pixelCenter + float2(-1, 1);
	float3 rayDir = normalize(ndc.x * gCamera.cameraU +
		ndc.y * gCamera.cameraV +
		gCamera.cameraW);

	// Initialize a ray structure for our ray tracer
	RayDesc ray = { gCamera.posW, 0.0f, rayDir, 1e+38f };

	// Initialize our ray payload (a per-ray, user-definable structure).
	SimplePayload payload = { false };

	// Trace our ray
	TraceRay(gRtScene, RAY_FLAG_NONE, 0xFF, 0, 1, 0, ray, payload);
}

// A dummy payload for this simple ray; never used
struct SimplePayload {
	bool dummyValue;
};

// Our miss shader's variables
cbuffer MissShaderCB {
	float3  gBgColor;
};

// The output textures.  See bindings in C++ code.
//     -> gMatDif is visible in the miss shader
//     -> All textures are visible in the hit shaders
RWTexture2D<float4> gWsPos, gWsNorm, gMatDif, gMatSpec, gMatExtra;

[shader("miss")]
void PrimaryMiss(inout SimplePayload)
{
	gMatDif[DispatchRaysIndex()] = float4(gBgColor, 1.0f);
}

[shader("anyhit")]
void PrimaryAnyHit(inout SimpleRayPayload, BuiltinIntersectionAttribs attribs)
{
	if (alphaTestFails(attribs)) IgnoreHit();
}

[shader("closesthit")]
void PrimaryClosestHit(inout SimpleRayPayload, BuiltinIntersectionAttribs attribs)
{
	// Which pixel spawned our ray?
	uint2 idx = DispatchRaysIndex();

	// Run helper function to compute important data at the current hit point
	ShadingData shadeData = getShadingData(PrimitiveIndex(), attribs);

	// Save out our G-buffer values to the specified output textures
	gWsPos[idx] = float4(shadeData.posW, 1.f);
	gWsNorm[idx] = float4(shadeData.N, length(shadeData.posW - gCamera.posW));
	gMatDif[idx] = float4(shadeData.diffuse, shadeData.opacity);
	gMatSpec[idx] = float4(shadeData.specular, shadeData.linearRoughness);
	gMatExtra[idx] = float4(shadeData.IoR,
		shadeData.doubleSidedMaterial ? 1.f : 0.f,
		0.f, 0.f);
}