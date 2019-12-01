#pragma once

#include <memory>
#include <vector>
#include <thread>
#include <atomic>

// Currently Windows Only Supported
#if (defined(_WIN32) || defined(__MINGW32__) || defined(_MSC_VER_) || defined(__MINGW64__)) 
#include <windows.h> // Fix HMODULE Type Error
#endif

// TODO: Linux Support 
#include <vulkan/vulkan.hpp>

namespace lancer {
    //#define api vk  // C-style
    namespace api { using namespace vk; }; // safer version 
    
    template<class T, class U = uint32_t> class BitOps { protected: U n = 0u; //friend BitOps<T,U>;
        public: BitOps<T,U>(const T& v = 0u) : n((U&)v){};
        public: BitOps<T,U>(const U& n = 0u) : n(n){};
        public: BitOps<T,U>(T&& v = 0u) : n((U&)v){};
        public: BitOps<T,U>(U&& n = 0u) : n(n){};
        public: operator U&() { return n; };
        public: operator const U&() const { return n; };
        public: T operator &(const T& o){ return T{n&(U&)o}; };
        public: T operator |(const T& o){ return T{n|(U&)o}; };
        public: T operator ^(const T& o){ return T{n^(U&)o}; };
    };

    template<class S, class B, class U = uint32_t> class Flags { protected:
        //public: Flags<S,B>(const Flags<S,B,U>& f = 0u) { *(U*)(this) = (U&)f; };
        public: Flags<S,B,U>(const U& f = 0u) { *(U*)(this) = f; };
        public: Flags<S,B,U>(const S& f = 0u) { *(U*)(this) = (U&)f; };
        public: Flags<S,B,U>(const B& f = 0u) { *(U*)(this) = (U&)f; };
        public: operator U&() { return *(U*)this; };
        public: operator const U&() const { return *(U*)this; };
        public: operator B&() { return *(B*)this; };
        public: operator const B&() const { return *(B*)this; };
    };



    // bitfield with enum helper
    template<class E, class eS>
    class Bitfield {
    protected:
        using enum_u = typename std::underlying_type<E>::type;
        union { eS bitmask; enum_u bitmask_u32; }; // bitmask (former FlagBits, replaced by yBitfield), bitfield (former Flags, replaced by yBitfield) types
        using yBitfield = Bitfield<E,eS>;
        
    public:
        friend yBitfield;
        constexpr Bitfield() : bitmask_u32(0u) {}
        Bitfield(enum_u bitmask_u32) : bitmask_u32(bitmask_u32) {}
        Bitfield(eS bitmask) : bitmask(bitmask) {}
        Bitfield(E bitenum) : bitmask_u32(enum_u(bitenum)) {}
        Bitfield(yBitfield& a) { bitmask = a; }
        Bitfield(yBitfield&& a) { bitmask = std::move(a.bitmask); }

        // typed operator 
        operator eS&() { return bitmask; }
        operator const eS&() const { return bitmask; }
        operator enum_u&() { return bitmask_u32; }
        operator const enum_u&() const { return bitmask_u32; }
        operator const E() const { return (const E&)(bitmask_u32); } // read-only getting base enum


        // TODO: bitwise negate operator

        // and
        yBitfield operator&(const enum_u& a) const { return yBitfield(enum_u(*this) & a); }
        yBitfield operator&(const E& a) const { return (*this) & enum_u(a); }
        yBitfield operator&(const yBitfield& a) const { return (*this) & enum_u(a); }
        yBitfield operator&(const eS& a) const { return (*this) & enum_u(a); }

        // or
        yBitfield operator|(const enum_u& a) const { return yBitfield(enum_u(*this) | a); }
        yBitfield operator|(const E& a) const { return (*this) | enum_u(a); }
        yBitfield operator|(const yBitfield& a) const { return (*this) | enum_u(a); }
        yBitfield operator|(const eS& a) const { return (*this) | enum_u(a); }

        // xor
        yBitfield operator^(const enum_u& a) const { return yBitfield(enum_u(*this) ^ a); }
        yBitfield operator^(const E& a) const { return (*this) ^ enum_u(a); }
        yBitfield operator^(const yBitfield& a) const { return (*this) ^ enum_u(a); }
        yBitfield operator^(const eS& a) const { return (*this) ^ enum_u(a); }
        
        // get access to native bits
        eS& operator->() { return bitmask; };
        const eS& operator->() const { return bitmask; };
    };


    template<class Inh, uint32_t S = 0xFFFFFFFFu>
    class StructHeader {
    protected:
        const uint32_t _sType = S;
        uintptr_t _pNext = 0ull;

    public:
        static uint32_t getCType() { return S; } // defend by structure ID
        uint32_t getSType() const { return _sType; }

        using fType = StructHeader<Inh, S>;
        friend fType;

        template<class T>
        T& getNext() const {
            auto cls = (T*)_pNext;
            if (cls->getSType() != cls->getCType()) std::cerr << "got wrong class" << std::endl; // wrong structure secure before API layers
            return *(T*)_pNext;
        }

        template<class T>
        Inh& setNext(const T& c) {
            _pNext = (uintptr_t)(&c); return (Inh&)(*this);
        }
    };

    // Vulkan Handler neutral type 
    struct NHND { uintptr_t _hnd = 0ull; };

    // unsupported at now
    template<class P>
    class Parented {
        protected:
        public:
    };

    // insider handler (TODO: cast from original type i.e. `H`)
    template<class H, class vH = uintptr_t> // first and just handler, second may be class with methods
    class UHandler {
        protected:
            union { H _hnd = H(0ull); vH _hni; }; // alternative class/handler
            using UHandlerT = UHandler<vH,H>;

        public:
            friend UHandlerT;
            constexpr UHandler() : _hnd(0ull) {};
            //constexpr UHandler(const void * a) : _hnd(uintptr_t(a)) {}; // for nullptr arguments
            UHandler(const void * a) : _hnd(uintptr_t(a)) {}; // include, for nullptr arguments
            UHandler(const intptr_t& a) : _hnd(uintptr_t(a)) {};
            UHandler(const uintptr_t& a) : _hnd(a) {};
            UHandler(const int& a) : _hnd(uintptr_t(a)) {};
            UHandler(const unsigned& a) : _hnd(a) {};
            UHandler(UHandlerT& a) : _hnd(a._hnd) {};
            UHandler(UHandlerT&& a) : _hnd(std::move(a._hnd)) {};
            UHandler(const vH& a) : _hni(a) {}; // foreign handler (from C API)
            UHandler(const H& a) : _hnd(a) {};

            // Casting Handlers of API
            operator const vH&() const {return _hni;}; // return original API handler (if need)
            operator vH&() {return _hni;};
            operator const H&() const {return _hnd;}; // return original API handler (if need)
            operator H&() {return _hnd;};
            operator const void*&() const {return (const void*&)(_hnd);}; // return original API handler (if need)
            operator void*&() {return (void*&)(_hnd);};
            operator const uintptr_t&() const {return (uintptr_t&)_hnd;}; // return pointer value
            operator uintptr_t&() {return (uintptr_t&)_hnd;};
            operator const intptr_t&() const {return (intptr_t&)_hnd;}; // return pointer value
            operator intptr_t&() {return (intptr_t&)_hnd;};

            // use another reference
            template<class T = vH> T& ref() {return (T&)(*this); };
            template<class T = vH> const T& ref() const {return (const T&)(*this); };

            // use secondary class methods
            vH& operator->() { return _hni; };
            const vH& operator->() const { return _hni; };
    };

    // boolean 32-bit capable for C++
    class bool32_t { // TODO: support operators
        protected: uint32_t _b = 0u;
        public: 
            friend bool32_t;
            constexpr bool32_t(): _b(0u) {}
            bool32_t(const bool& a): _b(a?1u:0u) {}
            bool32_t(const uint32_t& a): _b(a&1u) {} // make bitmasked
            bool32_t(const bool32_t& a): _b(a._b) {}
            bool32_t(bool32_t& a): _b(a._b) {}
            bool32_t(bool32_t&& a): _b(std::move(a._b)) {}

            // type conversion operators
            operator bool() const { return bool(_b&1u); };
            //operator bool&() { return (bool&)(_b); } // I suppose it not possible
            operator uint32_t() const { return _b&1u; };
            operator uint32_t&() { return _b; } // return uint32 link
    };


    template<class T, class R> // T SHOULD BE SHARED_PTR<T_n>
    class ReturnHandleRef { protected: T that = nullptr; R result = 0u; friend T; friend R;//T& that; R result = RESULT::SUCCESS;
        public: ReturnHandleRef(const T& that, R result = 0u): that(std::atomic_load(that)), result(result) {};
        public: ReturnHandleRef(T&& that, R result = 0u): that(std::move(that)), result(result) {}; // Minimum 1 argument for AVOID 'AMBIGUOUS' STATE

        // accept another typed pointer
        template<class H = T> ReturnHandleRef(H&& object, R result = 0u) : that((T)(std::move(object))) {};
        template<class H = T> ReturnHandleRef(const H& object, R result = 0u) : that((T)(std::atomic_load(object))) {};

        // 
        operator std::pair<T&,R&>(){ return std::pair(that,result); };
        operator std::pair<const T&,const R&>() const { return std::pair(that,result); };
        
        // 
        operator const T&() const { return that; };
        operator const R&() const { return result; };
        operator T&(){ return that; };
        operator R&(){ return result; };

        // return ptr reference
        T& ref() { return that; };
        const T& ref() const { return that; };
        
        template<class H = T> H cst() {return H(that); };
        template<class H = T> const H cst() const {return H(that); };

        template<class H = T> ReturnHandleRef<H,R> hnd() {return ReturnHandleRef<H,R>(that); };
        template<class H = T> const ReturnHandleRef<H,R> hnd() const {return ReturnHandleRef<H,R>(that); };

        template<class H = T> ReturnHandleRef<H,R>& hrf() {return (ReturnHandleRef<H,R>&)(*this); };
        template<class H = T> const ReturnHandleRef<H,R>& hrf() const {return (ReturnHandleRef<H,R>&)(*this); };

        // tribute handler (bit danger with handler)
        T& operator->() { return that; };
        const T& operator->() const { return that; };
    };

    class Device_T;
    class Buffer_T;
    class Image_T;
    class DescriptorSet_T;
    class DescriptorSetLayout_T;
    class Instance_T;
    class PipelineLayout_T;
    class RenderPass_T;
    class MemoryAllocation_T;
    class MemoryAllocator_T;
    class PhysicalDevice_T;
    class Sampler_T;
    class GraphicsPipeline_T;

    template<class T = uint8_t> class BufferRegion_T;
    using SamplerMaker = std::shared_ptr<Sampler_T>;
    using BufferMaker = std::shared_ptr<Buffer_T>;
    using DeviceMaker = std::shared_ptr<Device_T>;
    using ImageMaker = std::shared_ptr<Image_T>;
    using DescriptorSetMaker = std::shared_ptr<DescriptorSet_T>;
    using DescriptorSetLayoutMaker = std::shared_ptr<DescriptorSetLayout_T>;
    using InstanceMaker = std::shared_ptr<Instance_T>;
    using PipelineLayoutMaker = std::shared_ptr<PipelineLayout_T>;
    using RenderPassMaker = std::shared_ptr<RenderPass_T>;
    using MemoryAllocation = std::shared_ptr<MemoryAllocation_T>;
    using MemoryAllocator = std::shared_ptr<MemoryAllocator_T>;
    using PhysicalDeviceHelper = std::shared_ptr<PhysicalDevice_T>;
    using GraphicsPipelineMaker = std::shared_ptr<GraphicsPipeline_T>;
    using BufferRegionU8_T = BufferRegion_T<uint8_t>;
    using BufferRegionU8Maker = std::shared_ptr<BufferRegionU8_T>;
    
};
