/*
 ***********************************************************************************************************************
 *
 *  Copyright (c) 2017-2020 Advanced Micro Devices, Inc. All Rights Reserved.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 *
 **********************************************************************************************************************/
/**
 ***********************************************************************************************************************
 * @file  llpcPatchDescriptorLoad.h
 * @brief LLPC header file: contains declaration of class lgc::PatchDescriptorLoad.
 ***********************************************************************************************************************
 */
#pragma once

#include "llvm/IR/InstVisitor.h"

#include <unordered_set>
#include "llpcPatch.h"
#include "llpcPipelineShaders.h"
#include "llpcPipelineState.h"
#include "llpcSystemValues.h"

namespace lgc
{

// =====================================================================================================================
// Represents the pass of LLVM patching opertions for descriptor load.
class PatchDescriptorLoad:
    public Patch,
    public llvm::InstVisitor<PatchDescriptorLoad>
{
public:
    PatchDescriptorLoad();

    void getAnalysisUsage(llvm::AnalysisUsage& analysisUsage) const override
    {
        analysisUsage.addRequired<PipelineStateWrapper>();
        analysisUsage.addRequired<PipelineShaders>();
        analysisUsage.addPreserved<PipelineShaders>();
    }

    virtual bool runOnModule(llvm::Module& module) override;
    virtual void visitCallInst(llvm::CallInst& callInst);

    // -----------------------------------------------------------------------------------------------------------------

    static char ID;   // ID of this pass

private:
    PatchDescriptorLoad(const PatchDescriptorLoad&) = delete;
    PatchDescriptorLoad& operator=(const PatchDescriptorLoad&) = delete;

    void ProcessDescriptorGetPtr(llvm::CallInst* pDescPtrCall, llvm::StringRef descPtrCallName);
    llvm::Value* GetDescPtrAndStride(ResourceNodeType        resType,
                                     uint32_t                descSet,
                                     uint32_t                binding,
                                     const ResourceNode*     pTopNode,
                                     const ResourceNode*     pNode,
                                     bool                    shadow,
                                     llvm::IRBuilder<>&      builder);
    llvm::Value* GetDescPtr(ResourceNodeType resType,
                            uint32_t                descSet,
                            uint32_t                binding,
                            const ResourceNode*     pTopNode,
                            const ResourceNode*     pNode,
                            bool                    shadow,
                            llvm::IRBuilder<>&      builder);

    void ProcessDescriptorIndex(llvm::CallInst* pCall);
    void ProcessLoadDescFromPtr(llvm::CallInst* pLoadFromPtr);
    llvm::Value* LoadBufferDescriptor(uint32_t            descSet,
                                      uint32_t            binding,
                                      llvm::Value*        pArrayOffset,
                                      llvm::Instruction*  pInsertPoint);

    llvm::Value* BuildInlineBufferDesc(llvm::Value* pDescPtr, llvm::IRBuilder<>& builder);
    llvm::Value* BuildBufferCompactDesc(llvm::Value* pDesc, llvm::Instruction* pInsertPoint);

    // -----------------------------------------------------------------------------------------------------------------

    // Descriptor size
    static const uint32_t  DescriptorSizeResource      = 8 * sizeof(uint32_t);
    static const uint32_t  DescriptorSizeSampler       = 4 * sizeof(uint32_t);
    static const uint32_t  DescriptorSizeSamplerYCbCr  = 8 * sizeof(uint32_t);
    static const uint32_t  DescriptorSizeBuffer        = 4 * sizeof(uint32_t);
    static const uint32_t  DescriptorSizeBufferCompact = 2 * sizeof(uint32_t);

    bool                                m_changed;            // Whether the pass has modified the code
    PipelineSystemValues                m_pipelineSysValues;  // Cache of ShaderValues object per shader
    std::vector<llvm::CallInst*>        m_descLoadCalls;      // List of instructions to load descriptors
    std::unordered_set<llvm::Function*> m_descLoadFuncs;      // Set of descriptor load functions

    PipelineState*                  m_pPipelineState = nullptr;
                                                              // Pipeline state from PipelineStateWrapper pass
};

} // lgc
