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

#include "Falcor.h"
#include "../SharedUtils/RenderingPipeline.h"
#include "Passes/AmbientOcclusionPass.h"
#include "Passes/ShadowPass.h"
#include "Passes/ReflectionPass.h"
#include "Passes/DirectLightingPass.h"
#include "Passes/FinalStagePass.h"
#include "Passes/SVGFPass.h"
#include "Passes/SVGFShadowPass.h"
#include "Passes/ComparePass.h"
#include "Passes/MergePass.h"
#include "../CommonPasses/SimpleGBufferPass.h"
#include "../CommonPasses/SimpleAccumulationPass.h"
#include "../CommonPasses/CopyToOutputPass.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	// Create our rendering pipeline
	RenderingPipeline *pipeline = new RenderingPipeline();
	Falcor::gProfileEnabled = true;

	int idx = 0;
	constexpr bool useAccum = false;
	constexpr bool perf = true;

	pipeline->setPass(idx++, SimpleGBufferPass::create());
	pipeline->setPass(idx++, DirectLightingPass::create("directLightingChannel"));
	if (useAccum) {
		pipeline->setPass(idx++, ReflectionPass::create("reflectionFilter"));
		pipeline->setPass(idx++, SimpleAccumulationPass::create("reflectionFilter"));
	}
	else {
		pipeline->setPass(idx++, ReflectionPass::create("reflectionOut"));
		pipeline->setPass(idx++, SVGFPass::create("reflectionFilter", "reflectionOut"));
	}
	pipeline->setPass(idx++, AmbientOcclusionPass::create("aoChannel"));
	//pipeline->setPass(idx++, AmbientOcclusionPass::create("shadowChannel"));	//When printing AO shadows
	pipeline->setPass(idx++, ShadowPass::create("shadowChannel"));
	pipeline->setPass(idx++, SVGFShadowPass::create("shadowFilter", "shadowChannel", "aoChannel"));
	pipeline->setPass(idx++, FinalStagePass::create(perf ? ResourceManager::kOutputChannel : "finalOutput"));
	if (!perf) {
		pipeline->setPass(idx++, ComparePass::create("compareOutput"));
		pipeline->setPass(idx++, CopyToOutputPass::create());
	}
	
	// Define a set of config / window parameters for our program
	SampleConfig config;
	config.windowDesc.title = "Hybrid Rendering";
	config.windowDesc.resizableWindow = true;

	// Start our program!
	RenderingPipeline::run(pipeline, config);
}
