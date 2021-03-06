/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DCommandList_DEFINED
#define GrD3DCommandList_DEFINED

#include "include/gpu/GrTypes.h"
#include "include/gpu/d3d/GrD3DTypes.h"
#include "include/private/SkColorData.h"
#include "src/gpu/GrManagedResource.h"
#include "src/gpu/d3d/GrD3DConstantRingBuffer.h"

#include <memory>

class GrD3DGpu;
class GrD3DBuffer;
class GrD3DConstantRingBuffer;
class GrD3DPipelineState;
class GrD3DRenderTarget;
class GrD3DRootSignature;
class GrD3DTextureResource;

class GrScissorState;

class GrD3DCommandList {
public:
    virtual ~GrD3DCommandList() {
        this->releaseResources();
    }

    enum class SubmitResult {
        kNoWork,
        kSuccess,
        kFailure,
    };
    SubmitResult submit(ID3D12CommandQueue* queue);

    bool close();
    void reset();

    ////////////////////////////////////////////////////////////////////////////
    // GraphicsCommandList commands
    ////////////////////////////////////////////////////////////////////////////

    // For the moment we only support Transition barriers
    // All barriers should reference subresources of managedResource
    void resourceBarrier(sk_sp<GrManagedResource> managedResource,
                         int numBarriers,
                         D3D12_RESOURCE_TRANSITION_BARRIER* barriers);

    // Helper method that calls copyTextureRegion multiple times, once for each subresource
    void copyBufferToTexture(GrD3DBuffer* srcBuffer,
                             GrD3DTextureResource* dstTexture,
                             uint32_t subresourceCount,
                             D3D12_PLACED_SUBRESOURCE_FOOTPRINT* bufferFootprints,
                             int left, int top);
    void copyTextureRegion(sk_sp<GrManagedResource> dst,
                           const D3D12_TEXTURE_COPY_LOCATION* dstLocation,
                           UINT dstX, UINT dstY,
                           sk_sp<GrManagedResource> src,
                           const D3D12_TEXTURE_COPY_LOCATION* srcLocation,
                           const D3D12_BOX* srcBox);
    void copyBufferToBuffer(sk_sp<GrManagedResource> dst,
                            ID3D12Resource* dstBuffer, uint64_t dstOffset,
                            sk_sp<GrManagedResource> src,
                            ID3D12Resource* srcBuffer, uint64_t srcOffset,
                            uint64_t numBytes);

    // Add ref-counted resource that will be tracked and released when this command buffer finishes
    // execution
    void addResource(sk_sp<GrManagedResource> resource) {
        SkASSERT(resource);
        resource->notifyQueuedForWorkOnGpu();
        fTrackedResources.push_back(std::move(resource));
    }

    // Add ref-counted resource that will be tracked and released when this command buffer finishes
    // execution. When it is released, it will signal that the resource can be recycled for reuse.
    void addRecycledResource(sk_sp<GrRecycledResource> resource) {
        resource->notifyQueuedForWorkOnGpu();
        fTrackedRecycledResources.push_back(std::move(resource));
    }

    void releaseResources();

    bool hasWork() const { return fHasWork; }

private:
    static const int kInitialTrackedResourcesCount = 32;

protected:
    GrD3DCommandList(gr_cp<ID3D12CommandAllocator> allocator,
                     gr_cp<ID3D12GraphicsCommandList> commandList);

    void addingWork();
    virtual void onReset() {}

    void submitResourceBarriers();

    gr_cp<ID3D12GraphicsCommandList> fCommandList;

    SkSTArray<kInitialTrackedResourcesCount, sk_sp<GrManagedResource>> fTrackedResources;
    SkSTArray<kInitialTrackedResourcesCount, sk_sp<GrRecycledResource>> fTrackedRecycledResources;

    // When we create a command list it starts in an active recording state
    SkDEBUGCODE(bool fIsActive = true;)
    bool fHasWork = false;

private:
    gr_cp<ID3D12CommandAllocator> fAllocator;

    SkSTArray<4, D3D12_RESOURCE_BARRIER> fResourceBarriers;
};

class GrD3DDirectCommandList : public GrD3DCommandList {
public:
    static std::unique_ptr<GrD3DDirectCommandList> Make(ID3D12Device* device);

    ~GrD3DDirectCommandList() override = default;

    void setPipelineState(sk_sp<GrD3DPipelineState> pipelineState);

    void setCurrentConstantBuffer(const sk_sp<GrD3DConstantRingBuffer>& constantBuffer);

    void setStencilRef(unsigned int stencilRef);
    void setBlendFactor(const float blendFactor[4]);
    void setPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primitiveTopology);
    void setScissorRects(unsigned int numRects, const D3D12_RECT* rects);
    void setViewports(unsigned int numViewports, const D3D12_VIEWPORT* viewports);
    void setGraphicsRootSignature(const sk_sp<GrD3DRootSignature>& rootSignature);
    void setVertexBuffers(unsigned int startSlot,
                          const GrD3DBuffer* vertexBuffer, size_t vertexStride,
                          const GrD3DBuffer* instanceBuffer, size_t instanceStride);
    void setIndexBuffer(const GrD3DBuffer* indexBuffer);
    void drawInstanced(unsigned int vertexCount, unsigned int instanceCount,
                       unsigned int startVertex, unsigned int startInstance);
    void drawIndexedInstanced(unsigned int indexCount, unsigned int instanceCount,
                              unsigned int startIndex, unsigned int baseVertex,
                              unsigned int startInstance);

    void clearRenderTargetView(GrD3DRenderTarget* renderTarget, const SkPMColor4f& color,
                               const GrScissorState& scissor);
    void setRenderTarget(GrD3DRenderTarget* renderTarget);

    void setGraphicsRootConstantBufferView(unsigned int rootParameterIndex,
                                           D3D12_GPU_VIRTUAL_ADDRESS bufferLocation);
    void setGraphicsRootDescriptorTable(unsigned int rootParameterIndex,
                                        D3D12_GPU_DESCRIPTOR_HANDLE bufferLocation);
    void setDescriptorHeaps(sk_sp<GrRecycledResource> srvCrvHeapResource,
                            ID3D12DescriptorHeap* srvDescriptorHeap,
                            sk_sp<GrRecycledResource> samplerHeapResource,
                            ID3D12DescriptorHeap* samplerDescriptorHeap);

private:
    GrD3DDirectCommandList(gr_cp<ID3D12CommandAllocator> allocator,
                           gr_cp<ID3D12GraphicsCommandList> commandList);

    void onReset() override;

    const GrD3DRootSignature* fCurrentRootSignature;
    const GrD3DBuffer* fCurrentVertexBuffer;
    size_t fCurrentVertexStride;
    const GrD3DBuffer* fCurrentInstanceBuffer;
    size_t fCurrentInstanceStride;
    const GrD3DBuffer* fCurrentIndexBuffer;

    GrD3DConstantRingBuffer* fCurrentConstantRingBuffer;
    GrD3DConstantRingBuffer::SubmitData fConstantRingBufferSubmitData;

    ID3D12DescriptorHeap* fCurrentSRVCRVDescriptorHeap;
    ID3D12DescriptorHeap* fCurrentSamplerDescriptorHeap;
};

class GrD3DCopyCommandList : public GrD3DCommandList {
public:
    static std::unique_ptr<GrD3DCopyCommandList> Make(ID3D12Device* device);

private:
    GrD3DCopyCommandList(gr_cp<ID3D12CommandAllocator> allocator,
                         gr_cp<ID3D12GraphicsCommandList> commandList);
};
#endif
