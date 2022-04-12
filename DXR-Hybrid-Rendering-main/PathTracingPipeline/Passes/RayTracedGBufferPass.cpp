/**********************************************************************************************************************
# Copyright (c) 2018, NVIDIA CORPORATION. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
# following conditions are met:
#  * Redistributions of code must retain the copyright notice, this list of conditions and the following disclaimer.
#  * Neither the name of NVIDIA CORPORATION nor the names of its contributors may be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT
# SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
# OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**********************************************************************************************************************/

//http://cwyman.org/code/dxrTutors/tutors/Tutor4/tutorial04.md.html
#include "RayTracedGBufferPass.h"

namespace {
	// Where are our shaders located?
	const char* rtGBufferShader = "rtGBuffer.hlsl";
	//const char* kGbufFragShader = "CommonPasses\\gBuffer.ps.hlsl";
};

bool RayTracedGBufferPass::initialize(RenderContext::SharedPtr pRenderContext,
	ResourceManager::SharedPtr pResManager)
{
	// Stash a copy of our resource manager; tell it what shared texture
	//     resources we need for this pass.
	mpResManager = pResManager;
	mpResManager->requestTextureResources({ "WorldPosition", "WorldNormal",
										  "MaterialDiffuse", "MaterialSpecRough",
										  "MaterialExtraParams" });

	// Create our wrapper for our DirectX Raytracing launch.
	mpRays = RayLaunch::create("rtGBuffer.hlsl", "GBufferRayGen");
	mpRays->addMissShader("rtGBuffer.hlsl", "PrimaryMiss");
	mpRays->addHitShader("rtGBuffer.hlsl", "PrimaryClosestHit", "PrimaryAnyHit");

	// Compile our shaders and pass in our scene
	mpRays->compileRayProgram();
	mpRays->setScene(mpScene);
	return true;
}

void RayTracedGBufferPass::execute(RenderContext::SharedPtr pRenderContext)
{
	// Color used to clear our G-buffer 
	vec4 black = vec4(0, 0, 0, 0);

	// Load our textures; ask the resource manager to clear them first
	//    Note:  'auto' type used for brevity; actually Texture::SharedPtr
	auto wsPos = mpResManager->getClearedTexture("WorldPosition", black);
	auto wsNorm = mpResManager->getClearedTexture("WorldNormal", black);
	auto matDif = mpResManager->getClearedTexture("MaterialDiffuse", black);
	auto matSpec = mpResManager->getClearedTexture("MaterialSpecRough", black);
	auto matExtra = mpResManager->getClearedTexture("MaterialExtraParams", black);
	auto matEmit = mpResManager->getClearedTexture("Emissive", black);

	// Pass our background color down to miss shader #0
	auto missVars = mpRays->getMissVars(0);
	missVars["MissShaderCB"]["gBgColor"] = mBgColor;  // Color for background
	missVars["gMatDif"] = matDif;                     // Where to store bg color

	// Pass down variables for our hit group #0
	for (auto pVars : mpRays->getHitVars(0))
	{
		pVars["gWsPos"] = wsPos;
		pVars["gWsNorm"] = wsNorm;
		pVars["gMatDif"] = matDif;
		pVars["gMatSpec"] = matSpec;
		pVars["gMatExtra"] = matExtra;
	}

	// Launch our ray tracing
	for (size_t i = 0; i < 2; i++)
	{
		mpRays->execute(pRenderContext, mpResManager->getScreenSize());
	}
	
}