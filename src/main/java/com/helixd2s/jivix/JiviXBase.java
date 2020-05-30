package com.helixd2s.jivix;

import org.bytedeco.javacpp.*;
import org.bytedeco.javacpp.annotation.*;
import org.bytedeco.javacpp.indexer.*;

// "jniJiviXBase", "JiviX"

// TODO: Wrap CORE Instead Of External Native... (i.e. reduce layers count)
@Platform(include = {
        "./include/GLFW/glfw3.h",
        "./include/vkt3/fw.hpp",
        "./include/JiviX/JiviX.hpp",
        "./jniJiviXCore.h"
}, link = {"vulkan-1", "glfw", "glbinding", "glbinding-aux"}, define = {"ENABLE_OPENGL_INTEROP", "WIN32", "OS_WIN", "VK_ENABLE_BETA_EXTENSIONS", "VK_USE_PLATFORM_WIN32_KHR", "VMA_IMPLEMENTATION", "SHARED_PTR_NAMESPACE std", "UNIQUE_PTR_NAMESPACE std"})
@Name("") //
public class JiviXBase extends Pointer {
    static { Loader.load(); }



    //
    public static native @Name("vkt::initializeGL")
    void initializeGL(@Cast("GLFWglproc(*)(const char*)") long GetProcAddress);

    // Required Linked
    //public static native @Name("vkt::initializeGL")
    //void initializeGL();

    //
    public static native @Name("vkt::createSemaphore")
    void createSemaphore(JiviXCore.Device device, @Cast("VkSemaphore*") long[] vkSemaphore, @Cast("unsigned*") int[] glSemaphore, @Cast("const void*") long pNext);

    //
    public static native @Name("vkt::submitCmd")
    void submitCmd(JiviXCore.Device device, @Cast("VkQueue") long queue, @StdVector @Cast("VkCommandBuffer*") long[] cmds, @Cast("vkh::VkSubmitInfo*") long smbi);

    //
    public static native @Name("vkt::submitCmdAsync")
    void submitCmdAsync(JiviXCore.Device device, @Cast("VkQueue") long queue, @StdVector @Cast("VkCommandBuffer*") long[] cmds, @Cast("vkh::VkSubmitInfo*") long smbi);

    //
    public static native @Name("vkt::createCommandBuffer") @Cast("VkCommandBuffer")
    long createCommandBuffer(JiviXCore.Device device, @Cast("VkCommandPool") long commandPool, byte secondary, byte once);

    //
    public static native @Name("vkt::createCompute") @Cast("VkPipeline")
    long createCompute(JiviXCore.Device device, @StdString String path, @Cast("VkPipelineLayout") long layout, @Cast("VkPipelineCache") long cache, @Cast("uint32_t") int subgroupSize);

    //
    public static native @Name("vkt::createCompute") @Cast("VkPipeline")
    long createCompute(JiviXCore.Device device, @StdVector @Cast("uint32_t*") int[] code, @Cast("VkPipelineLayout") long layout, @Cast("VkPipelineCache") long cache, @Cast("uint32_t") int subgroupSize);


    //
    @Name("vkt::ImageRegion") //
    public static class ImageRegion extends Pointer {
        static { Loader.load(); }

        public ImageRegion() {
            allocate();
        }
        public ImageRegion(Pointer p) {
            super(p);
        }
        public ImageRegion(ImageRegion b) {
            allocate(b);
        }
        public ImageRegion(@SharedPtr JiviXCore.ImageAllocation alloc, @Cast("vkh::VkImageViewCreateInfo*") long info, @Cast("VkImageLayout") int layout) { allocate(alloc, info, layout); }
        public ImageRegion(@SharedPtr JiviXCore.VmaImageAllocation alloc, @Cast("vkh::VkImageViewCreateInfo*") long info, @Cast("VkImageLayout") int layout) { allocate(alloc, info, layout); }

        private native void allocate();
        private native void allocate(ImageRegion b);
        private native void allocate(@SharedPtr JiviXCore.ImageAllocation alloc, @Cast("vkh::VkImageViewCreateInfo*") long info, @Cast("VkImageLayout") int layout);
        private native void allocate(@SharedPtr JiviXCore.VmaImageAllocation alloc, @Cast("vkh::VkImageViewCreateInfo*") long info, @Cast("VkImageLayout") int layout);

        //
        public native int getGLImage();
        public native int getGLMemory();

        //
        public native @Cast("uintptr_t") long getDescriptorPtr();
    }

    @Name("vkt::VectorBase") // TODO: Untyped Vector in Native
    public static class Vector extends Pointer {
        static { Loader.load(); }

        //
        public Vector() { super(); }
        public Vector(Pointer p) {
            super(p);
        }
        public Vector(@SharedPtr JiviXCore.VmaBufferAllocation a, long offset, long size, long stride) { allocate(a, offset, size, stride); }
        public Vector(@SharedPtr JiviXCore.BufferAllocation a, long offset, long size, long stride) { allocate(a, offset, size, stride); }

        //
        protected native void allocate();
        protected native void allocate(@SharedPtr JiviXCore.VmaBufferAllocation a, long offset, long size, long stride);
        protected native void allocate(@SharedPtr JiviXCore.BufferAllocation a, long offset, long size, long stride);

        // Java have NOT support `VkDeviceOrHostAddressKHR` or `VkDeviceOrHostAddressConstKHR`, and become rude var... (DeRMo!)
        public native @Cast("uintptr_t") long deviceAddress();
        public native @Cast("uintptr_t") long address(); // For Pointer Passes
        public native @Cast("uintptr_t") long mapv(long ptr);

        //
        public native long size();
        public native long range();

        //
        public native int getGL();
        public native int getGLBuffer();
        public native int getGLMemory();

        //
        public native @Cast("uintptr_t") long getDescriptorPtr();
    };

    // For JAVA only
    @Name("vkt::Vector<int8_t>") // int8_t default
    public static class ByteVector extends Vector {
        static { Loader.load(); }

        //
        public ByteVector() { super(); }
        public ByteVector(Pointer p) {
            super(p);
        }
        public ByteVector(@SharedPtr JiviXCore.VmaBufferAllocation a, long offset, long size) { super(a, offset, size, 1); }
        public ByteVector(@SharedPtr JiviXCore.BufferAllocation a, long offset, long size) { super(a, offset, size, 1); }

        // Indexer for Data
        public ByteBufferIndexer getIndexer() {
            return new ByteBufferIndexer(this.data().asBuffer());
        }

        //
        @Name("operator=") public native @ByRef ByteVector put(@ByRef ByteVector x);
        @Name("operator[]") public native @ByRef BytePointer at(long n);

        // map buffer data
        public native BytePointer mapped();
        public native BytePointer mapped(long ptr);
        public native BytePointer map();
        public native BytePointer map(long ptr);
        public native BytePointer data();
        public native BytePointer data(long ptr);
    }

    // FOR KOTLIN AND NATIVE!
    @Name("vkt::Vector<uint8_t>") // uint8_t version (C++)
    public static class UByteVector extends Vector {
        static { Loader.load(); }

        //
        public UByteVector(Pointer p) {
            super(p);
        }
        public UByteVector() { super(); }
        public UByteVector(@SharedPtr JiviXCore.VmaBufferAllocation a, long offset, long size) { super(a, offset, size, 1); }
        public UByteVector(@SharedPtr JiviXCore.BufferAllocation a, long offset, long size) { super(a, offset, size, 1); }

        // Indexer for Data
        public UByteBufferIndexer getIndexer() {
            return new UByteBufferIndexer(this.data().asBuffer());
        }

        //
        @Name("operator=") public native @ByRef UByteVector put(@ByRef UByteVector x);
        @Name("operator[]") public native @Cast("uint8_t*") @ByRef BytePointer at(long n);

        // map buffer data
        public native @Cast("uint8_t*")BytePointer mapped();
        public native @Cast("uint8_t*")BytePointer mapped(long ptr);
        public native @Cast("uint8_t*")BytePointer map();
        public native @Cast("uint8_t*")BytePointer map(long ptr);
        public native @Cast("uint8_t*")BytePointer data();
        public native @Cast("uint8_t*")BytePointer data(long ptr);
    }

    // For JAVA only
    @Name("vkt::Vector<int16_t>") // int8_t default
    public static class ShortVector extends Vector {
        static { Loader.load(); }

        //
        public ShortVector(Pointer p) {
            super(p);
        }
        public ShortVector() { super(); }
        public ShortVector(@SharedPtr JiviXCore.VmaBufferAllocation a, long offset, long size) { super(a, offset, size, 2); }
        public ShortVector(@SharedPtr JiviXCore.BufferAllocation a, long offset, long size) { super(a, offset, size, 2); }

        // Indexer for Data
        public ShortBufferIndexer getIndexer() {
            return new ShortBufferIndexer(this.data().asBuffer());
        }

        //
        @Name("operator=") public native @ByRef ShortVector put(@ByRef ShortVector x);
        @Name("operator[]") public native @ByRef ShortPointer at(long n);

        // map buffer data
        public native ShortPointer mapped();
        public native ShortPointer mapped(long ptr);
        public native ShortPointer map();
        public native ShortPointer map(long ptr);
        public native ShortPointer data();
        public native ShortPointer data(long ptr);
    }

    // FOR KOTLIN AND NATIVE!
    @Name("vkt::Vector<uint16_t>") // uint8_t version (C++)
    public static class UShortVector extends Vector {
        static { Loader.load(); }

        //
        public UShortVector(Pointer p) {
            super(p);
        }
        public UShortVector() { super(); }
        public UShortVector(@SharedPtr JiviXCore.VmaBufferAllocation a, long offset, long size) { super(a, offset, size, 2); }
        public UShortVector(@SharedPtr JiviXCore.BufferAllocation a, long offset, long size) { super(a, offset, size, 2); }

        // Indexer for Data
        public UShortBufferIndexer getIndexer() {
            return new UShortBufferIndexer(this.data().asBuffer());
        }

        //
        @Name("operator=") public native @ByRef UShortVector put(@ByRef UShortVector x);
        @Name("operator[]") public native @Cast("uint16_t*") @ByRef ShortPointer at(long n);

        // map buffer data
        public native @Cast("uint16_t*") ShortPointer mapped();
        public native @Cast("uint16_t*") ShortPointer mapped(long ptr);
        public native @Cast("uint16_t*") ShortPointer map();
        public native @Cast("uint16_t*") ShortPointer map(long ptr);
        public native @Cast("uint16_t*") ShortPointer data();
        public native @Cast("uint16_t*") ShortPointer data(long ptr);
    }

    // For JAVA only
    @Name("vkt::Vector<int32_t>") // int8_t default
    public static class IntVector extends Vector {
        static { Loader.load(); }

        //
        public IntVector(Pointer p) {
            super(p);
        }
        public IntVector() { super(); }
        public IntVector(@SharedPtr JiviXCore.VmaBufferAllocation a, long offset, long size) { super(a, offset, size, 4); }
        public IntVector(@SharedPtr JiviXCore.BufferAllocation a, long offset, long size) { super(a, offset, size, 4); }

        // Indexer for Data
        public IntBufferIndexer getIndexer() {
            return new IntBufferIndexer(this.data().asBuffer());
        }

        //
        @Name("operator=") public native @ByRef IntVector put(@ByRef IntVector x);
        @Name("operator[]") public native @ByRef IntPointer at(long n);

        // map buffer data
        public native IntPointer mapped();
        public native IntPointer mapped(long ptr);
        public native IntPointer map();
        public native IntPointer map(long ptr);
        public native IntPointer data();
        public native IntPointer data(long ptr);
    }

    // FOR KOTLIN AND NATIVE!
    @Name("vkt::Vector<uint32_t>") // uint8_t version (C++)
    public static class UIntVector extends Vector {
        static { Loader.load(); }

        //
        public UIntVector(Pointer p) {
            super(p);
        }
        public UIntVector() { super(); }
        public UIntVector(@SharedPtr JiviXCore.VmaBufferAllocation a, long offset, long size) { super(a, offset, size, 4); }
        public UIntVector(@SharedPtr JiviXCore.BufferAllocation a, long offset, long size) { super(a, offset, size, 4); }

        // Indexer for Data
        public UIntBufferIndexer getIndexer() {
            return new UIntBufferIndexer(this.data().asBuffer());
        }

        //
        @Name("operator=") public native @ByRef UIntVector put(@ByRef UIntVector x);
        @Name("operator[]") public native @Cast("uint32_t*") @ByRef IntPointer at(long n);

        // map buffer data
        public native @Cast("uint32_t*") IntPointer mapped();
        public native @Cast("uint32_t*") IntPointer mapped(long ptr);
        public native @Cast("uint32_t*") IntPointer map();
        public native @Cast("uint32_t*") IntPointer map(long ptr);
        public native @Cast("uint32_t*") IntPointer data();
        public native @Cast("uint32_t*") IntPointer data(long ptr);
    }

    // Universal
    @Name("vkt::Vector<float>") // int8_t default
    public static class FloatVector extends Vector {
        static { Loader.load(); }

        //
        public FloatVector(Pointer p) {
            super(p);
        }
        public FloatVector() { super(); }
        public FloatVector(@SharedPtr JiviXCore.VmaBufferAllocation a, long offset, long size) { super(a, offset, size, 4); }
        public FloatVector(@SharedPtr JiviXCore.BufferAllocation a, long offset, long size) { super(a, offset, size, 4); }

        // Indexer for Data
        public FloatBufferIndexer getIndexer() {
            return new FloatBufferIndexer(this.data().asBuffer());
        }

        //
        @Name("operator=") public native @ByRef FloatVector put(@ByRef FloatVector x);
        @Name("operator[]") public native @ByRef FloatPointer at(long n);

        // map buffer data
        public native FloatPointer mapped();
        public native FloatPointer mapped(long ptr);
        public native FloatPointer map();
        public native FloatPointer map(long ptr);
        public native FloatPointer data();
        public native FloatPointer data(long ptr);
    }



    @Name("jvx::Context")
    public static class Context extends Pointer {
        static { Loader.load(); }

        public Context(Pointer p) {
            super(p);
        }
        public Context() {
            allocate();
        }
        public Context(@SharedPtr JiviXCore.Context object) {
            allocate(object);
        }
        public Context(@SharedPtr JiviXCore.Driver object) {
            allocate(object);
        }
        public Context(Driver object) {
            allocate(object);
        }

        private native void allocate();
        private native void allocate(@SharedPtr JiviXCore.Context object);
        private native void allocate(@SharedPtr JiviXCore.Driver object);
        private native void allocate(Driver object);

        public native JiviXCore.Context setModelView(@Cast("glm::mat4x4*") FloatPointer mv);
        public native JiviXCore.Context setPerspective(@Cast("glm::mat4x4*") FloatPointer mv);

        //public native void createDescriptorSet();
        public native @SharedPtr JiviXCore.Context sharedPtr();

        //
        public native @ByRef JiviXBase.ImageRegion getFrameBuffer(int index);
        public native @ByRef JiviXBase.ImageRegion getFlip0Buffer(int index);
        public native @ByRef JiviXBase.ImageRegion getFlip1Buffer(int index);

        //
        public native JiviXCore.Context initialize(int i, int i1);

    }

    @Name("jvx::Thread")
    public static class Thread extends Pointer {
        static { Loader.load(); }

        public Thread(Pointer p) {
            super(p);
        }
        public Thread() {
            allocate();
        }
        public Thread(@SharedPtr JiviXCore.Thread object) {
            allocate(object);
        }

        private native void allocate();
        private native void allocate(@SharedPtr JiviXCore.Thread object);

        // public native void createDescriptorSet();
        public native @SharedPtr JiviXCore.Thread sharedPtr();

        // conflict with vector-based!
        public native void submitCmd(@Cast("VkCommandBuffer") long commandBuf, @Cast("vkh::VkSubmitInfo*") long submitInfoAddress);
        public native void submitCmdAsync(@Cast("VkCommandBuffer") long commandBuf, @Cast("vkh::VkSubmitInfo*") long submitInfoAddress);
    }

    @Name("jvx::Driver")
    public static class Driver extends Pointer {
        static { Loader.load(); }

        public Driver(Pointer p) {
            super(p);
        }
        public Driver() {
            allocate();
        }
        public Driver(@SharedPtr JiviXCore.Driver object) {
            allocate(object);
        }

        private native void allocate();
        private native void allocate(@SharedPtr JiviXCore.Driver object);

        // LWJGL-3 Compatible
        public native long getInstanceCreateInfoAddress();
        public native long getDeviceCreateInfoAddress();

        // Get Handle Value
        public native @Cast("VkPhysicalDevice") long getPhysicalDevice();
        public native @Cast("VkPhysicalDevice") long getPhysicalDevice(int ID);
        public native @Cast("VkDevice") long getDevice();
        public native @Cast("VkQueue")long getQueue();
        public native @Cast("VkFence") long getFence();
        public native @Cast("VkInstance") long getInstance();
        public native @Cast("VkCommandPool") long getCommandPool();
        public native @Cast("VkPipelineCache") long getPipelineCache();
        public native @Cast("VkDescriptorPool") long getDescriptorPool();
        public native @Cast("VkImageView") long getDepthImageView();
        public native @Cast("VkImage") long getDepthImage();
        public native @Cast("VkInstance") long createInstance();
        public native @Cast("VkDevice") long createDevice();
        public native @Cast("VkDevice") long createDevice(@Cast("VkPhysicalDevice") long physicalDeviceHandle);

        // Get Address of Reference... (but needs wrapped as Pointer?)
        public native @Name("getMemoryProperties") @ByRef @Cast("int8_t*") BytePointer _getMemoryProperties();
        public native @Name("getAllocator") @ByRef @Cast("int8_t*") BytePointer _getAllocator();
        //public native @Name("getDispatch")          @ByRef @Cast("int8_t*") BytePointer _getDispatch();

        // Get Address from allocator or properties
        //public long getDispatch()         { return this._getDispatch().address(); };
        public long getAllocator() {
            return this._getAllocator().address();
        }
        public long getMemoryProperties() {
            return this._getMemoryProperties().address();
        }

        //
        public native @SharedPtr JiviXCore.Device getDeviceDispatch();
        public native @SharedPtr JiviXCore.Instance getInstanceDispatch();

        //
        public native long memoryAllocationInfoPtr();
    }

    @Name("jvx::BufferViewSet")
    public static class BufferViewSet extends Pointer {
        static { Loader.load(); }

        public BufferViewSet(Pointer p) {
            super(p);
        }
        public BufferViewSet() {
            allocate();
        }
        public BufferViewSet(Context context) {
            allocate(context);
        }
        public BufferViewSet(@SharedPtr JiviXCore.BufferViewSet object) {
            allocate(object);
        }

        private native void allocate();
        private native void allocate(Context context);
        private native void allocate(@SharedPtr JiviXCore.BufferViewSet object);

        public native @SharedPtr JiviXCore.BufferViewSet sharedPtr();
        public native long getBufferCount();
        public native void createDescriptorSet();
        public native @ByRef UByteVector get(int id);
        public native long pushBufferView(@ByRef UByteVector vector);
        //public native @StdVector @ByRef ByteVector[] getBufferViewList();

        // Value based handle
        public native @Cast("VkDescriptorSet") long getDescriptorSet();
        public native @Cast("VkDescriptorSetLayout") long getDescriptorLayout();

        // Swinka Tramvaya
        //public native @Cast("VkDescriptorSet*") @ByRef LongPointer getDescriptorSet();
        //public native @Cast("VkDescriptorSetLayout*") @ByRef LongPointer getDescriptorLayout();
    }


    @Name("jvx::MeshInput")
    public static class MeshInput extends Pointer {
        static { Loader.load(); }

        public MeshInput(Pointer p) {
            super(p);
        }
        public MeshInput() {
            allocate();
        }
        public MeshInput(Context context) {
            allocate(context);
        }
        public MeshInput(@SharedPtr JiviXCore.MeshInput object) {
            allocate(object);
        }

        private native void allocate();
        private native void allocate(Context context);
        private native void allocate(@SharedPtr JiviXCore.MeshInput object);

        public native @SharedPtr JiviXCore.MeshInput sharedPtr();
        public native JiviXCore.MeshInput createDescriptorSet();
        public native JiviXCore.MeshInput makeQuad();
        public native JiviXCore.MeshInput copyMeta(@Cast("VkCommandBuffer") long commandBuffer);
        public native JiviXCore.MeshInput manifestIndex(@Cast("VkIndexType") int indexType);

        // Use Address from LWJGL-3 structures
        public native JiviXCore.MeshInput addBinding(int bufferID, @Cast("vkh::VkVertexInputBindingDescription*") long bindingAddress);
        public native JiviXCore.MeshInput addAttribute(@Cast("vkh::VkVertexInputAttributeDescription*") long attributeAddress);
        public native JiviXCore.MeshInput setIndexData(int bufferID, @Cast("VkIndexType") int indexType);
        public native JiviXCore.MeshInput setIndexOffset(@Cast("VkDeviceSize") long offset);
        public native JiviXCore.MeshInput setIndexCount(@Cast("VkDeviceSize") long count);
        public native JiviXCore.MeshInput setPrimitiveCount(@Cast("VkDeviceSize") long count);

        // Reference Based
        // public native @Cast("VkDeviceSize&") @ByRef LongPointer getIndexCount(); // Return Reference

        // Value Based
        public native long getIndexCount(); // Return Reference
        public native JiviXCore.MeshInput linkBViewSet(@SharedPtr JiviXCore.BufferViewSet bufferViewSet);
        public native JiviXCore.MeshInput linkBViewSet(@ByRef BufferViewSet bufferViewSet);
    }


    @Name("jvx::MeshBinding")
    public static class MeshBinding extends Pointer {
        static { Loader.load(); }

        public MeshBinding(Pointer p) {
            super(p);
        }
        public MeshBinding() {
            allocate();
        }

        //
        public MeshBinding(Context context, long maxPrimitiveCount, @StdVector long[] perGeometryCount) {
            allocate(context, maxPrimitiveCount, perGeometryCount);
        }

        //
        public MeshBinding(Context context, long maxPrimitiveCount) {
            allocate(context, maxPrimitiveCount);
        }
        public MeshBinding(@SharedPtr JiviXCore.MeshBinding object) {
            allocate(object);
        }

        private native void allocate();
        private native void allocate(Context context, long maxPrimitiveCount, @StdVector long[] perGeometryCount);

        // Not Prefer...
        //public MeshBinding(Context context) { allocate(context); }
        //private native void allocate(Context context);

        private native void allocate(Context context, long maxPrimitiveCount);
        private native void allocate(@SharedPtr JiviXCore.MeshBinding object);

        public native @SharedPtr JiviXCore.MeshBinding sharedPtr();
        public native JiviXCore.MeshBinding setThread(JiviXCore.Thread thread);
        public native JiviXCore.MeshBinding setDriver(JiviXCore.Driver thread);
        public native @ByRef UByteVector getBindingBuffer(long i);
        public native @ByRef UByteVector getIndexBuffer();
        public native int getBindingBufferGL(long i);
        public native int getIndexBufferGL();

        public native JiviXCore.MeshBinding resetGeometry();
        public native JiviXCore.MeshBinding setIndexCount(@Cast("VkDeviceSize") long count);
        public native JiviXCore.MeshBinding setPrimitiveCount(@Cast("VkDeviceSize") long count);
        public native JiviXCore.MeshBinding setTransformData(@Cast("vkt::Vector<glm::mat3x4>*") long address, int stride);
        public native JiviXCore.MeshBinding setTransformData(@Cast("vkt::Vector<glm::mat3x4>*") long address);
        public native JiviXCore.MeshBinding buildGeometry(@Cast("VkCommandBuffer") long cmdbufAddress, @Cast("glm::uvec4*") long meshData);
        public native JiviXCore.MeshBinding addMeshInput(@SharedPtr JiviXCore.MeshInput input, int materialID, long instances);
        public native JiviXCore.MeshBinding addMeshInput(@SharedPtr JiviXCore.MeshInput input, int materialID);
        public native JiviXCore.MeshBinding addMeshInput(@SharedPtr JiviXCore.MeshInput input, @StdVector int[] materialIDs);
        public native JiviXCore.MeshBinding addMeshInput(@ByRef MeshInput input, int materialID, long instances);
        public native JiviXCore.MeshBinding addMeshInput(@ByRef MeshInput input, int materialID);
        public native JiviXCore.MeshBinding addMeshInput(@ByRef MeshInput input, @StdVector int[] materialIDs);
        public native JiviXCore.MeshBinding addRangeInput(long range, int materialID, long instances);
        public native JiviXCore.MeshBinding addRangeInput(long range, int materialID);
        public native JiviXCore.MeshBinding addRangeInput(long range, @StdVector int[] materialIDs);

        //public native @ByRef JiviXCore.MeshBinding addMeshInput(@ByRef MeshInput input, @StdVector int materialIDs[]);
    }


    @Name("jvx::Node")
    public static class Node extends Pointer {
        static { Loader.load(); }

        public Node(Pointer p) {
            super(p);
        }
        public Node() {
            allocate();
        }
        public Node(Context context) {
            allocate(context);
        }
        public Node(@SharedPtr JiviXCore.Node object) {
            allocate(object);
        }

        private native void allocate();
        private native void allocate(Context context);
        private native void allocate(@SharedPtr JiviXCore.Node object);

        public native @SharedPtr JiviXCore.Node sharedPtr();
        public native JiviXCore.Node pushInstance(@Cast("vkh::VsGeometryInstance*") long address);
        public native JiviXCore.Node pushInstance(JiviXCore.VsGeometryInstance geometry);
        public native JiviXCore.Node resetInstances();
        public native long pushMesh(@ByRef MeshBinding binding);
        public native void createDescriptorSet();
    }


    @Name("jvx::Material")
    public static class Material extends Pointer {
        static { Loader.load(); }

        public Material(Pointer p) {
            super(p);
        }
        public Material() {
            allocate();
        }
        public Material(Context context) {
            allocate(context);
        }
        public Material(@SharedPtr JiviXCore.Material object) {
            allocate(object);
        }

        private native void allocate();
        private native void allocate(Context context);
        private native void allocate(@SharedPtr JiviXCore.Material object);

        public native @SharedPtr JiviXCore.Material sharedPtr();
        public native void createDescriptorSet();
        public native long pushMaterial(JiviXCore.MaterialUnit materialAddress);

        public native int pushSampledImage(@Cast("vkh::VkDescriptorImageInfo*") long imageDescAddress);
        public native JiviXCore.Material setRawMaterials(@Cast("vkt::Vector<jvi::MaterialUnit>*") UByteVector rawMaterials, long materialCount);
        public native JiviXCore.Material setGpuMaterials(@Cast("vkt::Vector<jvi::MaterialUnit>*") UByteVector rawMaterials);
        public native JiviXCore.Material resetMaterials();
        public native JiviXCore.Material resetSampledImages();
    }


    @Name("jvx::Renderer")
    public static class Renderer extends Pointer {
        static { Loader.load(); }

        public Renderer(Pointer p) {
            super(p);
        }
        public Renderer() {
            allocate();
        }
        public Renderer(Context context) {
            allocate(context);
        }
        public Renderer(@SharedPtr JiviXCore.Renderer object) {
            allocate(object);
        }

        private native void allocate();
        private native void allocate(Context context);
        private native void allocate(@SharedPtr JiviXCore.Renderer object);

        public native @SharedPtr JiviXCore.Renderer sharedPtr();

        //
        public native JiviXCore.Renderer linkMaterial(@SharedPtr JiviXCore.Material material);
        public native JiviXCore.Renderer linkNode(@SharedPtr JiviXCore.Node node);

        // Deprecated Due sKotlin
        //public native JiviXCore.Renderer linkMaterial(@ByRef Material material);
        //public native JiviXCore.Renderer linkNode(@ByRef Node node);

        //
        public native JiviXCore.Renderer setupCommands(@Cast("VkCommandBuffer") long cmd, byte once, int options);
        public native @Cast("VkCommandBuffer") long refCommandBuffer();



        //public native void createDescriptorSet();
    }


    @Name("jvx::TestClass")
    public static class TestClass extends Pointer {
        static { Loader.load(); }

        public TestClass(Pointer p) {
            super(p);
        }
        public TestClass() {
            allocate();
        }

        private native void allocate();
    }

}
