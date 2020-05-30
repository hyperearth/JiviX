package com.helixd2s.jivix

import org.bytedeco.javacpp.BytePointer
import org.bytedeco.javacpp.FloatPointer
import org.bytedeco.javacpp.IntPointer
import org.bytedeco.javacpp.ShortPointer
import org.bytedeco.javacpp.indexer.*
import org.lwjgl.vulkan.*
import org.lwjgl.vulkan.VK10.VK_IMAGE_LAYOUT_GENERAL
import java.lang.Integer.min

// TODO: Reduce Native Layers count and make more thin
// TODO: Add operable Int, UInt, Long, ULong... referenced types for Kotlin
abstract class JiviX {

    // WHAT IS `core`?
    // Core is UnWrapped Java or Wrapped Native Interface between with Kotlin
    // JavaCPP Have NO ULong, so NEEDS BI-DIRECTIONAL PER-BITS Conversion Between LONG and ULONG! (i.e. ULONG -> LONG -> ULONG WITHOUT any data loss)
    // Here is GLOBAL Methods, Variables...
    companion object {
        fun createSemaphore(device: Device, vkSemaphore: LongArray, glSemaphore: IntArray, pNext: ULong) { // UN-Able to Convert As Pointer, so used Signed
            JiviXBase.createSemaphore(device.core, vkSemaphore, glSemaphore, pNext.toLong())
        }

        fun initializeGL(procAddress: ULong) { JiviXBase.initializeGL(procAddress.toLong()) }
        //fun initializeGL() { JiviXBase.initializeGL() }

        fun submitCmd(device: Device, queue: ULong, cmds: ULongArray, smbi: VkSubmitInfo) {
            JiviXBase.submitCmd(device.core, queue.toLong(), cmds.toLongArray(), smbi.address())
        }

        fun submitCmdAsync(device: Device, queue: ULong, cmds: ULongArray, smbi: VkSubmitInfo) {
            JiviXBase.submitCmdAsync(device.core, queue.toLong(), cmds.toLongArray(), smbi.address())
        }

        fun createCommandBuffer(device: Device, commandPool: ULong, secondary: Byte, once: Byte) : ULong {
            return JiviXBase.createCommandBuffer(device.core, commandPool.toLong(), secondary, once).toULong()
        }

        fun createCompute(device: Device, path: String, layout: ULong = 0UL, cache: ULong = 0UL, subgroupSize: UInt = 0U): ULong {
            return JiviXBase.createCompute(device.core, path, layout.toLong(), cache.toLong(), subgroupSize.toInt()).toULong()
        }

        fun createCompute(device: Device, code: UIntArray, layout: ULong = 0UL, cache: ULong = 0UL, subgroupSize: UInt = 0U): ULong {
            return JiviXBase.createCompute(device.core, code.toIntArray(), layout.toLong(), cache.toLong(), subgroupSize.toInt()).toULong()
        }
    }

    //
    open class Device() {
        open lateinit var core: JiviXCore.Device

        constructor(core: JiviXCore.Device) : this() { this.core = core; }
    }

    //
    open class Instance() {
        open lateinit var core: JiviXCore.Instance

        constructor(core: JiviXCore.Instance) : this() { this.core = core; }
    }

    // MOST HARDCORE!
    open class MaterialUnit() {
        open var core: JiviXCore.MaterialUnit = JiviXCore.MaterialUnit()

        constructor(core: JiviXCore.MaterialUnit) : this() { this.core = core; }

        open var diffuse: FloatArray
            get() { return FloatArray(4).also { FloatPointer(this.core).get(it, 0, 4) } }
            set(value) { FloatPointer(this.core).put(value, 0, min(value.size,4)) }

        open var specular: FloatArray
            get() { return FloatArray(4).also { FloatPointer(this.core).position(16L shr 2).get(it, 0, 4) } }
            set(value) { FloatPointer(this.core).position(16L shr 2).put(value, 0, min(value.size,4)) }

        open var normals: FloatArray
            get() { return FloatArray(4).also { FloatPointer(this.core).position(32L shr 2).get(it, 0, 4) } }
            set(value) { FloatPointer(this.core).position(32L shr 2).put(value, 0, min(value.size,4)) }

        open var emission: FloatArray
            get() { return FloatArray(4).also { FloatPointer(this.core).position(48L shr 2).get(it, 0, 4) } }
            set(value) { FloatPointer(this.core).position(48L shr 2).put(value, 0, min(value.size,4)) }

        open var diffuseTexture: Int
            get() { return IntPointer(this.core).get(16).toInt() }
            set(value) { IntPointer(this.core).put(16L, value.toInt()) }

        open var specularTexture: Int
            get() { return IntPointer(this.core).get(17).toInt() }
            set(value) { IntPointer(this.core).put(17L, value.toInt()) }

        open var normalsTexture: Int
            get() { return IntPointer(this.core).get(18).toInt() }
            set(value) { IntPointer(this.core).put(18L, value.toInt()) }

        open var emissionTexture: Int
            get() { return IntPointer(this.core).get(19).toInt() }
            set(value) { IntPointer(this.core).put(19L, value.toInt()) }
    }

    //
    open class MemoryAllocationInfo() {
        open var core: JiviXCore.MemoryAllocationInfo = JiviXCore.MemoryAllocationInfo()

        constructor(core: JiviXCore.MemoryAllocationInfo) : this() {
            this.core = core; }

        open var glID: Int
            get() { return IntPointer(this.core).get(0).toInt() }
            set(value) { IntPointer(this.core).put(0L, value.toInt()) }
    }

    // TODO: Add Minecraft Matrix4f to FloatArrray[12] Convert!
    open class VsGeometryInstance() {
        open var core: JiviXCore.VsGeometryInstance = JiviXCore.VsGeometryInstance()

        constructor(data: JiviXCore.VsGeometryInstance) : this() { this.core = data }

        // TODO: Reference Version
        //open var transform: FloatPointer? = FloatPointer(1.0F,0.0F,0.0F,0.0F, 0.0F,1.0F,0.0F,0.0F, 0.0F,0.0F,1.0F,0.0F)
        //get ( ) { return this.core.transform(); }

        // ...
        open var transform: FloatArray
            get() { return floatArrayOf(1.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 0.0F).also { FloatPointer(this.core).get(it, 0, 12) } } // UNSAFE!
            set(value) { FloatPointer(this.core).put(value, 0, min(12,value.size)) } // UNSAFE!

        open var mask: UByte
            get() { return BytePointer(this.core).get(51).toUByte() }
            set(value) { BytePointer(this.core).put(51L, value.toByte()) }

        open var flags: UByte
            get() { return BytePointer(this.core).get(55).toUByte() }
            set(value) { BytePointer(this.core).put(55L, value.toByte()) }

        open var instanceId: UInt
            get() { return IntPointer(this.core).get(12).toUInt().and(0xFFFFFFU) }
            set(value) {
                BytePointer(this.core).position(48L).put(ubyteArrayOf(
                    (value shr 0).and(0xFFU).toUByte(),
                    (value shr 8).and(0xFFU).toUByte(),
                    (value shr 16).and(0xFFU).toUByte()
                ).toByteArray(), 0, 3)
            }

        open var instanceOffset: UInt
            get() { return IntPointer(this.core).get(13).toUInt().and(0xFFFFFFU) }
            set(value) {
                BytePointer(this.core).position(52L).put(ubyteArrayOf(
                    (value shr 0).and(0xFFU).toUByte(),
                    (value shr 8).and(0xFFU).toUByte(),
                    (value shr 16).and(0xFFU).toUByte()
                ).toByteArray(), 0, 3)
            }
    }

    // WARNING! ANY STRUCT SHOULD TO BE ALREADY INITIALIZED!
    open class VmaMemoryInfo() {
        open var core: JiviXCore.VmaMemoryInfo = JiviXCore.VmaMemoryInfo()

        constructor(info: JiviXCore.VmaMemoryInfo) : this() { this.core = info }

        open var memUsage: UInt
            set(value) { IntPointer(this.core).put(0, value.toInt()) }
            get() { return IntPointer(this.core).get(0).toUInt(); }

        open var deviceDispatch: Device
            get() { return Device(core.deviceDispatch); }
            set(value) { core.deviceDispatch = value.core; }

        open var instanceDispatch: Instance
            get() { return Instance(core.instanceDispatch); }
            set(value) { core.instanceDispatch = value.core; }
    }


    //
    open class BufferAllocation() {
        open lateinit var core: JiviXCore.BufferAllocation

        //
        constructor(allocation: JiviXCore.BufferAllocation) : this() { this.core = allocation }

        // LWJGL-3 here is now used!
        constructor(bufferCreateInfo: VkBufferCreateInfo, memAlloc: MemoryAllocationInfo) : this() {
            this.core = JiviXCore.BufferAllocation(bufferCreateInfo.address(), memAlloc.core) }
    }

    //
    open class ImageAllocation() {
        open lateinit var core: JiviXCore.ImageAllocation

        //
        constructor(allocation: JiviXCore.ImageAllocation) : this() { this.core = allocation }

        // LWJGL-3 here is now used!
        constructor(imageCreateInfo: VkImageCreateInfo, memAlloc: MemoryAllocationInfo) : this() {
            this.core = JiviXCore.ImageAllocation(imageCreateInfo.address(), memAlloc.core) }
    }


    //
    open class VmaBufferAllocation() {
        open lateinit var core: JiviXCore.VmaBufferAllocation

        //
        constructor(allocation: JiviXCore.VmaBufferAllocation) : this() { this.core = allocation }

        // LWJGL-3 here is now used!
        constructor(vmaAllocator: ULong, bufferCreateInfo: VkBufferCreateInfo, vmaMemInfo: VmaMemoryInfo) : this() {
            this.core = JiviXCore.VmaBufferAllocation(vmaAllocator.toLong(), bufferCreateInfo.address(), vmaMemInfo.core) }

    }

    //
    open class VmaImageAllocation() {
        open lateinit var core: JiviXCore.VmaImageAllocation

        //
        constructor(allocation: JiviXCore.VmaImageAllocation) : this() { this.core = allocation }

        // LWJGL-3 here is now used!
        constructor(vmaAllocator: ULong, imageCreateInfo: VkImageCreateInfo, vmaMemInfo: VmaMemoryInfo) : this() {
            this.core = JiviXCore.VmaImageAllocation(vmaAllocator.toLong(), imageCreateInfo.address(), vmaMemInfo.core) }

    }

    //
    open class ImageRegion() {
        open lateinit var core: JiviXBase.ImageRegion

        //
        constructor(region: JiviXBase.ImageRegion) : this() { this.core = region }

        // JavaCPP Have NO ULong, so NEEDS BI-DIRECTIONAL PER-BITS Conversion Between LONG and ULONG! (i.e. ULONG -> LONG -> ULONG WITHOUT any data loss)
        constructor(allocation: VmaImageAllocation, imageCreateInfo: VkImageViewCreateInfo, layout: UInt = VK_IMAGE_LAYOUT_GENERAL.toUInt()) : this() {
            this.core = JiviXBase.ImageRegion(allocation.core, imageCreateInfo.address(), layout.toInt()) }
        constructor(allocation: ImageAllocation, imageCreateInfo: VkImageViewCreateInfo, layout: UInt = VK_IMAGE_LAYOUT_GENERAL.toUInt()) : this() {
            this.core = JiviXBase.ImageRegion(allocation.core, imageCreateInfo.address(), layout.toInt()) }

        //
        open var descriptor: VkDescriptorImageInfo? = null
            get(){ return VkDescriptorImageInfo.create(this.core.descriptorPtr) }
    }


    //
    open class Vector() {
        open lateinit var core: JiviXBase.Vector

        //constructor(): this() {};
        constructor(vector: JiviXBase.ByteVector) : this() { this.core = vector }

        // JavaCPP Have NO ULong, so NEEDS BI-DIRECTIONAL PER-BITS Conversion Between LONG and ULONG! (i.e. ULONG -> LONG -> ULONG WITHOUT any data loss)
        constructor(allocation: VmaBufferAllocation, offset: ULong, range: ULong, stride: ULong = 1UL) : this() {
            this.core = JiviXBase.Vector(allocation.core, offset.toLong(), range.toLong(), stride.toLong()) }
        constructor(allocation: BufferAllocation, offset: ULong, range: ULong, stride: ULong = 1UL) : this() {
            this.core = JiviXBase.Vector(allocation.core, offset.toLong(), range.toLong(), stride.toLong()) }

        //
        open fun size(): ULong { return this.core.size().toULong(); }
        open fun range(): ULong { return this.core.range().toULong(); }
        open fun address(): ULong { return core.address().toULong(); }
        open fun deviceAddress(): ULong { return core.deviceAddress().toULong(); }
        open fun mapv(offset: ULong = 0UL): ULong { return core.mapv(offset.toLong()).toULong(); } // Used For Wrap with `Pointer{address}` or LWJGL-3 structures

        //
        open var descriptor: VkDescriptorBufferInfo? = null
            get(){ return VkDescriptorBufferInfo.create(this.core.descriptorPtr) }
    }


    //
    open class ByteVector() : Vector() {
        constructor(vector: JiviXBase.ByteVector) : this() { this.core = vector }
        constructor(allocation: VmaBufferAllocation, offset: ULong, range: ULong) : this() {
            this.core = JiviXBase.ByteVector(allocation.core, offset.toLong(), range.toLong()) }

        // TODO: Indexer Wrapper For Kotlin!
        open var indexer: ByteBufferIndexer? = null
            get() { return (core as JiviXBase.ByteVector).indexer }

        // TODO: FULL REFERENCE SUPPORT
        open operator fun get(index: Long): Byte { return (core as JiviXBase.ByteVector).data().get(index) }
        open operator fun set(index: Long, value: Byte) { (core as JiviXBase.ByteVector).data().put(index, value) }

        //
        open fun data(offset: ULong = 0UL): BytePointer { return (core as JiviXBase.ByteVector).data(offset.toLong()) }
        open fun map(offset: ULong = 0UL): BytePointer { return (core as JiviXBase.ByteVector).map(offset.toLong()) }
        open fun mapped(offset: ULong = 0UL): BytePointer { return (core as JiviXBase.ByteVector).mapped(offset.toLong()) }
    }

    // SHOULD TO BE LOSSLESS (Bit In Bit!)
    open class UByteVector() : Vector() {
        constructor(vector: JiviXBase.UByteVector) : this() { this.core = vector }
        constructor(allocation: VmaBufferAllocation, offset: ULong, range: ULong) : this() {
            this.core = JiviXBase.UByteVector(allocation.core, offset.toLong(), range.toLong()) }

        // TODO: Indexer Wrapper For Kotlin!
        open var indexer: UByteBufferIndexer? = null
            get() { return (core as JiviXBase.UByteVector).indexer }

        // TODO: FULL REFERENCE SUPPORT
        open operator fun get(index: Long): UByte { return (core as JiviXBase.UByteVector).data().get(index).toUByte() }
        open operator fun set(index: Long, value: UByte) { (core as JiviXBase.UByteVector).data().put(index, value.toByte()) }

        //
        open fun data(offset: ULong = 0UL): BytePointer { return (core as JiviXBase.UByteVector).data(offset.toLong()) }
        open fun map(offset: ULong = 0UL): BytePointer { return (core as JiviXBase.UByteVector).map(offset.toLong()) }
        open fun mapped(offset: ULong = 0UL): BytePointer { return (core as JiviXBase.UByteVector).mapped(offset.toLong()) }
    }


    //
    open class ShortVector() : Vector() {
        constructor(vector: JiviXBase.ShortVector): this() { this.core = vector }
        constructor(allocation: VmaBufferAllocation, offset: ULong, range: ULong) : this() {
            this.core = JiviXBase.ShortVector(allocation.core, offset.toLong(), range.toLong()) }

        // TODO: Indexer Wrapper For Kotlin!
        open var indexer: ShortBufferIndexer? = null
            get() { return (core as JiviXBase.ShortVector).indexer }

        // TODO: FULL REFERENCE SUPPORT
        open operator fun get(index: Long): Short { return (core as JiviXBase.ShortVector).data().get(index) }
        open operator fun set(index: Long, value: Short) { (core as JiviXBase.ShortVector).data().put(index, value) }

        //
        open fun data(offset: ULong = 0UL): ShortPointer { return (core as JiviXBase.ShortVector).data(offset.toLong()) }
        open fun map(offset: ULong = 0UL): ShortPointer { return (core as JiviXBase.ShortVector).map(offset.toLong()) }
        open fun mapped(offset: ULong = 0UL): ShortPointer { return (core as JiviXBase.ShortVector).mapped(offset.toLong()) }
    }

    // SHOULD TO BE LOSSLESS (Bit In Bit!)
    open class UShortVector() : Vector() {
        constructor(vector: JiviXBase.UShortVector) : this() { this.core = vector }
        constructor(allocation: VmaBufferAllocation, offset: ULong, range: ULong) : this() {
            this.core = JiviXBase.UShortVector(allocation.core, offset.toLong(), range.toLong()) }

        // TODO: Indexer Wrapper For Kotlin!
        open var indexer: UShortBufferIndexer? = null
            get() { return (core as JiviXBase.UShortVector).indexer }

        // TODO: FULL REFERENCE SUPPORT
        open operator fun get(index: Long): UShort { return (core as JiviXBase.UShortVector).data().get(index).toUShort() }
        open operator fun set(index: Long, value: UShort) { (core as JiviXBase.UShortVector).data().put(index, value.toShort()) }

        //
        open fun data(offset: ULong = 0UL): ShortPointer { return (core as JiviXBase.UShortVector).data(offset.toLong()) }
        open fun map(offset: ULong = 0UL): ShortPointer { return (core as JiviXBase.UShortVector).map(offset.toLong()) }
        open fun mapped(offset: ULong = 0UL): ShortPointer { return (core as JiviXBase.UShortVector).mapped(offset.toLong()) }
    }


    // FOR USE:
    // fun Float.Companion.fromBits(bits: Int): Float
    // fun Float.toRawBits(): Int
    open class IntVector() : Vector() {
        constructor(vector: JiviXBase.IntVector): this() { this.core = vector }
        constructor(allocation: VmaBufferAllocation, offset: ULong, range: ULong) : this() {
            this.core = JiviXBase.IntVector(allocation.core, offset.toLong(), range.toLong()) }

        // TODO: Indexer Wrapper For Kotlin!
        open var indexer: IntBufferIndexer? = null
            get() { return (core as JiviXBase.IntVector).indexer }

        // TODO: FULL REFERENCE SUPPORT
        open operator fun get(index: Long): Int { return (core as JiviXBase.IntVector).data().get(index) }
        open operator fun set(index: Long, value: Int) { (core as JiviXBase.IntVector).data().put(index, value) }

        //
        open fun data(offset: ULong = 0UL): IntPointer { return (core as JiviXBase.IntVector).data(offset.toLong()) }
        open fun map(offset: ULong = 0UL): IntPointer { return (core as JiviXBase.IntVector).map(offset.toLong()) }
        open fun mapped(offset: ULong = 0UL): IntPointer { return (core as JiviXBase.IntVector).mapped(offset.toLong()) }
    }

    // SHOULD TO BE LOSSLESS (Bit In Bit!), FOR USE:
    // fun Float.Companion.fromBits(bits: Int): Float
    // fun Float.toRawBits(): Int
    open class UIntVector() : Vector() {
        constructor(vector: JiviXBase.UIntVector) : this() { this.core = vector }
        constructor(allocation: VmaBufferAllocation, offset: ULong, range: ULong) : this() {
            this.core = JiviXBase.UIntVector(allocation.core, offset.toLong(), range.toLong()) }

        // TODO: Indexer Wrapper For Kotlin!
        open var indexer: UIntBufferIndexer? = null
            get() { return (core as JiviXBase.UIntVector).indexer }

        // TODO: FULL REFERENCE SUPPORT
        open operator fun get(index: Long): UInt { return (core as JiviXBase.UIntVector).data().get(index).toUInt() }
        open operator fun set(index: Long, value: UInt) { (core as JiviXBase.UIntVector).data().put(index, value.toInt()) }

        //
        open fun data(offset: ULong = 0UL): IntPointer { return (core as JiviXBase.UIntVector).data(offset.toLong()) }
        open fun map(offset: ULong = 0UL): IntPointer { return (core as JiviXBase.UIntVector).map(offset.toLong()) }
        open fun mapped(offset: ULong = 0UL): IntPointer { return (core as JiviXBase.UIntVector).mapped(offset.toLong()) }
    }

    //
    open class FloatVector() : Vector() {
        constructor(vector: JiviXBase.FloatVector) : this() { this.core = vector }
        constructor(allocation: VmaBufferAllocation, offset: ULong, range: ULong) : this() {
            this.core = JiviXBase.FloatVector(allocation.core, offset.toLong(), range.toLong()) }

        // TODO: Indexer Wrapper For Kotlin!
        open var indexer: FloatBufferIndexer? = null
            get() { return (core as JiviXBase.FloatVector).indexer }

        // TODO: FULL REFERENCE SUPPORT
        open operator fun get(index: Long): Float { return (core as JiviXBase.FloatVector).data().get(index) }
        open operator fun set(index: Long, value: Float) { (core as JiviXBase.FloatVector).data().put(index, value) }

        //
        open fun data(offset: ULong = 0UL): FloatPointer { return (core as JiviXBase.FloatVector).data(offset.toLong()) }
        open fun map(offset: ULong = 0UL): FloatPointer { return (core as JiviXBase.FloatVector).map(offset.toLong()) }
        open fun mapped(offset: ULong = 0UL): FloatPointer { return (core as JiviXBase.FloatVector).mapped(offset.toLong()) }
    }


    //
    //open class Driver() {
    open class Driver {
        open var core: JiviXBase.Driver

        constructor() { this.core = JiviXBase.Driver(); }

        constructor(obj: JiviXBase.Driver) { this.core = obj; }

        constructor(obj: JiviXCore.Driver) { this.core = JiviXBase.Driver(obj); }

        open var allocator: ULong = 0UL
            get() { return this.core._getAllocator().address().toULong(); }

        open var memoryProperties: ULong = 0UL
            get() { return this.core._getMemoryProperties().address().toULong(); }

        open val deviceDispatch: Device //= Device()
            get() { return Device(core.deviceDispatch); }
        //set(value) { core.setDeviceDispatch(value.core); };

        open val instanceDispatch: Instance //= Instance()
            get() { return Instance(core.instanceDispatch); }
        //set(value) { core.setInstanceDispatch(value.core); };

        open fun physicalDevice(idx: UInt): ULong { return this.core.getPhysicalDevice(idx.toInt()).toULong(); }

        open var physicalDevice: ULong = 0UL
            get() { return this.core.physicalDevice.toULong(); }

        open var device: ULong = 0UL
            get() { return this.core.device.toULong(); }

        open var queue: ULong = 0UL
            get(){ return this.core.queue.toULong(); }

        open var fence: ULong = 0UL
            get(){ return this.core.fence.toULong(); }

        open var instance: ULong = 0UL
            get() { // Automatically create when null found
                if (this.core.instance != 0L) {
                    return this.core.instance.toULong();
                } else {
                    return this.core.createInstance().toULong();
                }
            }

        open var commandPool: ULong = 0UL
            get() { return this.core.commandPool.toULong(); }

        open var pipelineCache: ULong = 0UL
            get() { return this.core.pipelineCache.toULong(); }

        open var descriptorPool: ULong = 0UL
            get() { return this.core.descriptorPool.toULong(); }

        open var depthImageView: ULong = 0UL
            get() { return this.core.depthImageView.toULong(); }

        open var depthImage: ULong = 0UL
            get() { return this.core.depthImage.toULong(); }

        open fun createInstance(): ULong { return this.core.createInstance().toULong(); }

        open var instanceClass: VkInstance? = null
            get(){ return VkInstance(this.instance.toLong(), this.instanceCreateInfo); }  // TODO: Null-Safe

        open var physicalDeviceClass: VkPhysicalDevice? = null
            get(){ return VkPhysicalDevice(this.physicalDevice.toLong(), this.instanceClass) } // TODO: Null-Safe

        open fun physicalDeviceClass(idx: UInt): VkPhysicalDevice { // TODO: Null-Safe
            return VkPhysicalDevice(this.physicalDevice(idx).toLong(), this.instanceClass) }

        open fun createDevice(): ULong { return this.core.createDevice().toULong(); }

        open fun createDevice(physicalDeviceHandle: ULong): ULong {
            return this.core.createDevice(physicalDeviceHandle.toLong()).toULong(); }

        open fun createDevice(physicalDevice: VkPhysicalDevice): VkDevice { // TODO: Null-Safe
            return VkDevice(this.createDevice().toLong(), physicalDevice, this.deviceCreateInfo); }

        open var instanceCreateInfo: VkInstanceCreateInfo? = null
            get() { return VkInstanceCreateInfo.createSafe(this.core.instanceCreateInfoAddress) }

        open var deviceCreateInfo: VkDeviceCreateInfo? = null
            get() { return VkDeviceCreateInfo.createSafe(this.core.deviceCreateInfoAddress) }

        open var memoryAllocationInfo: MemoryAllocationInfo? = null
            get() { val ptr = this.core.memoryAllocationInfoPtr()
            return MemoryAllocationInfo(object : JiviXCore.MemoryAllocationInfo() { init { address = ptr } }) }
    }

    //
    open class Context() {
        open lateinit var core: JiviXBase.Context

        constructor(obj: JiviXBase.Context) : this() { this.core = obj; }

        constructor(obj: JiviXCore.Context) : this() { this.core = JiviXBase.Context(obj); }

        constructor(driver: Driver) : this() { this.core = JiviXBase.Context(driver.core); }

        // Core Value
        open fun sharedPtr(): Context { return Context(core.sharedPtr()); }

        open fun initialize(width: UInt, height: UInt): Context {
            return Context(core.initialize(width.toInt(), height.toInt())); }

        //
        open fun frameBuffer(idx: UInt): ImageRegion {
            return ImageRegion(this.core.getFrameBuffer(idx.toInt())); }

        open fun flip0Buffer(idx: UInt): ImageRegion {
            return ImageRegion(this.core.getFlip0Buffer(idx.toInt())); }

        open fun flip1Buffer(idx: UInt): ImageRegion {
            return ImageRegion(this.core.getFlip1Buffer(idx.toInt())); }

        open fun setModelView(mv: FloatArray): Context {
            return Context(this.core.setModelView(FloatPointer(mv.size.toLong()).also{it.put(mv,0,mv.size)})); }

        open fun setPerspective(mv: FloatArray): Context { // WARNING: Needs to be careful with SIZE
            return Context(this.core.setPerspective(FloatPointer(mv.size.toLong()).also{it.put(mv,0,mv.size)})); }

        open fun setModelView(mv: FloatPointer): Context {
            return Context(this.core.setModelView(mv)); }

        open fun setPerspective(mv: FloatPointer): Context { // WARNING: Needs to be careful with SIZE
            return Context(this.core.setPerspective(mv)); }
    }

    //
    open class Thread() {
        open lateinit var core: JiviXBase.Thread

        constructor(obj: JiviXBase.Thread) : this() { this.core = obj; }

        constructor(obj: JiviXCore.Thread) : this() { this.core = JiviXBase.Thread(obj); }

        constructor(driver: Driver) : this() { this.core = JiviXBase.Thread(driver.core); }

        // Core Value
        open fun sharedPtr(): JiviXCore.Thread { return core.sharedPtr(); }
    }

    //
    open class BufferViewSet() {
        open lateinit var core: JiviXBase.BufferViewSet

        constructor(obj: JiviXBase.BufferViewSet) : this() { this.core = obj; }

        constructor(obj: JiviXCore.BufferViewSet) : this() { this.core = JiviXBase.BufferViewSet(obj); }

        constructor(context: Context) : this() { this.core = JiviXBase.BufferViewSet(context.core); }

        // Core Value
        open fun sharedPtr(): JiviXCore.BufferViewSet { return core.sharedPtr(); }

        open var bufferCount: ULong = 0UL
            get() { return this.core.bufferCount.toULong(); }

        //
        open operator fun get(index: Long): JiviXBase.UByteVector? { return core.get(index.toInt()) }  // TODO: Fix Index Type

        //
        open fun pushBufferView(buf: JiviXBase.UByteVector?): Long { return core.pushBufferView(buf) }

        //
        open var descriptorSet: ULong = 0UL
            get() { return core.descriptorSet.toULong(); }

        open var descriptorLayout: ULong = 0UL
            get() { return core.descriptorLayout.toULong(); }
    }

    //
    open class MeshInput() {
        open lateinit var core: JiviXBase.MeshInput

        constructor(obj: JiviXBase.MeshInput) : this() { this.core = obj; }

        constructor(obj: JiviXCore.MeshInput) : this() { this.core = JiviXBase.MeshInput(obj); }

        constructor(context: Context) : this() { this.core = JiviXBase.MeshInput(context.core); }

        // Core Value
        open fun sharedPtr(): JiviXCore.MeshInput { return core.sharedPtr(); }

        open fun addBinding(bufferID: UInt, binding: VkVertexInputBindingDescription): MeshInput {
            return MeshInput(this.core.addBinding(bufferID.toInt(), binding.address())) }

        open fun addAttribute(attribute: VkVertexInputAttributeDescription): MeshInput {
            return MeshInput(this.core.addAttribute(attribute.address())) }

        open fun setIndexData(bufferID: UInt, indexType: UInt): MeshInput {
            return MeshInput(this.core.setIndexData(bufferID.toInt(), indexType.toInt())) }

        open fun setIndexOffset(offset: ULong): MeshInput {
            return MeshInput(this.core.setIndexOffset(offset.toLong())) }

        open fun setPrimitiveCount(count: ULong): MeshInput {
            return MeshInput(this.core.setPrimitiveCount(count.toLong())) }

        open fun setIndexCount(count: ULong): MeshInput {
            return MeshInput(this.core.setIndexCount(count.toLong())) }

        open fun linkBViewSet(vset: BufferViewSet): MeshInput {
            return MeshInput(this.core.linkBViewSet(vset.core)) }

        //open fun getIndexCount(): ULong {
        //    return this.core.getIndexCount().toULong();
        //}

        var indexCount: ULong
            set(value) { this.setIndexCount(value); }
            get() { return this.core.indexCount.toULong(); }
    }

    //
    open class MeshBinding() {
        open lateinit var core: JiviXBase.MeshBinding

        constructor(obj: JiviXBase.MeshBinding) : this() { this.core = obj; }

        constructor(obj: JiviXCore.MeshBinding) : this() { this.core = JiviXBase.MeshBinding(obj); }

        // Core Value
        open fun sharedPtr(): JiviXCore.MeshBinding { return core.sharedPtr(); }

        // Unified Constructor
        constructor(context: Context, maxPrimitiveCount: ULong?, perGeometryCount: ULongArray?) : this() {
            if (maxPrimitiveCount != null) {
                if (perGeometryCount != null) {
                    this.core = JiviXBase.MeshBinding(context.core, maxPrimitiveCount.toLong(), perGeometryCount.toLongArray())
                } else {
                    this.core = JiviXBase.MeshBinding(context.core, maxPrimitiveCount.toLong())
                }
            } else {
                this.core = JiviXBase.MeshBinding(context.core)
            }
        }

        constructor(context: Context) : this() {
            this.core = JiviXBase.MeshBinding(context.core) }

        constructor(context: Context, maxPrimitiveCount: ULong) : this() {
            this.core = JiviXBase.MeshBinding(context.core, maxPrimitiveCount.toLong()) }

        constructor(context: Context, maxPrimitiveCount: ULong, perGeometryCount: ULongArray) : this() {
            this.core = JiviXBase.MeshBinding(context.core, maxPrimitiveCount.toLong(), perGeometryCount.toLongArray()) }

        //
        open fun resetGeometry(): MeshBinding {
            return MeshBinding(this.core.resetGeometry()) }

        //
        open fun bindingBuffer(idx: ULong = 0UL): UByteVector {
            return UByteVector(core.getBindingBuffer(idx.toLong())); }

        open fun bindingBufferGL(idx: ULong = 0UL): UInt {
            return core.getBindingBufferGL(idx.toLong()).toUInt() }

        // Instanced, but same material
        open fun addMeshInput(input: MeshInput, materialID: UInt, instances: ULong = 1UL): MeshBinding {
            return MeshBinding(this.core.addMeshInput(input.core, materialID.toInt(), instances.toLong())) }

        // Instanced, but same material
        open fun addRangeInput(range: ULong, materialID: UInt, instances: ULong = 1UL): MeshBinding {
            return MeshBinding(this.core.addRangeInput(range.toLong(), materialID.toInt(), instances.toLong())) }

        // Material Array
        open fun addMeshInput(input: MeshInput, materialID: UIntArray): MeshBinding {
            return MeshBinding(this.core.addMeshInput(input.core, materialID.toIntArray())) }

        // Material Array
        open fun addRangeInput(range: ULong, materialID: UIntArray): MeshBinding {
            return MeshBinding(this.core.addRangeInput(range.toLong(), materialID.toIntArray())) }

        // TODO: FloatPointer and FloatArray[12] support...
        open fun setTransformData(address: Long, stride: Int = 48): MeshBinding {
            return MeshBinding(this.core.setTransformData(address, stride)) }
    }

    //
    open class Node() {
        open lateinit var core: JiviXBase.Node

        constructor(obj: JiviXBase.Node) : this() { this.core = obj; }

        constructor(obj: JiviXCore.Node) : this() { this.core = JiviXBase.Node(obj); }

        constructor(context: Context) : this() { this.core = JiviXBase.Node(context.core); }

        // Core Value
        open fun sharedPtr(): JiviXCore.Node { return core.sharedPtr(); }

        open fun pushInstance(instance: ULong): Node {
            return Node(core.pushInstance(instance.toLong())); }

        open fun pushInstance(instance: VsGeometryInstance): Node {
            return Node(core.pushInstance(instance.core)); }

        open fun pushMesh(binding: MeshBinding): ULong {
            return core.pushMesh(binding.core).toULong() }

        open fun resetInstances(): Node {
            return Node(core.resetInstances()); }
    }

    //
    open class Renderer() {
        open lateinit var core: JiviXBase.Renderer

        constructor(obj: JiviXBase.Renderer) : this() { this.core = obj; }

        constructor(obj: JiviXCore.Renderer) : this() { this.core = JiviXBase.Renderer(obj); }

        constructor(context: Context) : this() { this.core = JiviXBase.Renderer(context.core); }

        // Core Value
        open fun sharedPtr(): JiviXCore.Renderer { return core.sharedPtr(); }

        //
        open fun linkMaterial(material: Material): Renderer {
            return Renderer(this.core.linkMaterial(material.core.sharedPtr())); }

        open fun linkNode(node: Node): Renderer {
            return Renderer(this.core.linkNode(node.core.sharedPtr())); }

        open fun setupCommands(cmd: ULong = 0UL, once: Byte = 1, options: UInt = 0xFFFFFFFFU): Renderer {
            return Renderer(this.core.setupCommands(cmd.toLong(), once, options.toInt())); }

        open fun refCommandBuffer(): ULong {
            return this.core.refCommandBuffer().toULong() }
    }

    //
    open class Material() {
        open lateinit var core: JiviXBase.Material

        constructor(obj: JiviXBase.Material) : this() { this.core = obj; }

        constructor(obj: JiviXCore.Material) : this() { this.core = JiviXBase.Material(obj); }

        constructor(context: Context) : this() { this.core = JiviXBase.Material(context.core); }

        // Core Value
        open fun sharedPtr(): Material { return Material(core.sharedPtr()); }
        open fun pushMaterial(unit: MaterialUnit): Long { return core.pushMaterial(unit.core) }

        open fun pushSampledImage(imageDescAddress: ULong): Int {
            return core.pushSampledImage(imageDescAddress.toLong()) }

        open fun pushSampledImage(imageDesc: VkDescriptorImageInfo): Int {
            return core.pushSampledImage(imageDesc.address()) }

        open fun setRawMaterials(rawMaterials: UByteVector, materialCount: ULong): Material {
            return Material(this.core.setRawMaterials(rawMaterials.core as JiviXBase.UByteVector?, materialCount.toLong())) }

        open fun setGpuMaterials(rawMaterials: UByteVector): Material {
            return Material(this.core.setGpuMaterials(rawMaterials.core as JiviXBase.UByteVector?)) }

        open fun resetMaterials(): Material {
            return Material(this.core.resetMaterials()) }

        open fun resetSampledImages(): Material {
            return Material(this.core.resetSampledImages()) }

    }

}
