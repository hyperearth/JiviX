#pragma once

#include <memory>
#include <vector>
#include <thread>
#include <atomic>

// Currently Windows Only Supported
#if (defined(_WIN32) || defined(__MINGW32__) || defined(_MSC_VER_) || defined(__MINGW64__)) 
#include <windows.h> // Fix HMODULE Type Error
#endif

// Default Backend
#if !defined(USE_D3D12) && !defined(USE_VULKAN)
#define USE_VULKAN
#endif

// 
#ifdef USE_VULKAN
#include "./declare/core/vulkan_hpp.inl"
#endif

// 
#ifdef USE_D3D12
#include "./declare/core/d3d12.inl"
#endif

namespace svt {
    namespace core {

        template<class T, class U = uint32_t> class bit_ops { protected: U n = 0u; //friend BitOps<T,U>;
            public: bit_ops<T,U>(const T& v = 0u) : n((U&)v){};
            public: bit_ops<T,U>(const U& n = 0u) : n(n){};
            public: bit_ops<T,U>(T&& v = 0u) : n((U&)v){};
            public: bit_ops<T,U>(U&& n = 0u) : n(n){};
            public: operator U&() { return n; };
            public: operator const U&() const { return n; };
            public: T operator &(const T& o){ return T{n&(U&)o}; };
            public: T operator |(const T& o){ return T{n|(U&)o}; };
            public: T operator ^(const T& o){ return T{n^(U&)o}; };
        };

        template<class S, class B, class U = uint32_t> class flags { protected: U n = 0u;
            public: flags<S,B,U>(const U& f = 0u) { *(U*)(this) = f; };
            public: flags<S,B,U>(const S& f = 0u) { *(U*)(this) = (U&)f; };
            public: flags<S,B,U>(const B& f = 0u) { *(U*)(this) = (U&)f; };
            public: operator U&() { return *(U*)this; };
            public: operator const U&() const { return *(U*)this; };
            public: operator B&() { return *(B*)this; };
            public: operator const B&() const { return *(B*)this; };
        };

        // bitfield with enum helper
        template<class E, class eS>
        class bitfield {
        protected:
            using enum_u = typename std::underlying_type<E>::type;
            union { eS bitmask; enum_u bitmask_u32; }; // bitmask (former FlagBits, replaced by yBitfield), bitfield (former Flags, replaced by yBitfield) types
            using bitfield_t = bitfield<E,eS>;
            friend bitfield_t;

        public:
            constexpr bitfield_t() : bitmask_u32(0u) {}
            bitfield_t(enum_u bitmask_u32) : bitmask_u32(bitmask_u32) {}
            bitfield_t(eS bitmask) : bitmask(bitmask) {}
            bitfield_t(E bitenum) : bitmask_u32(enum_u(bitenum)) {}
            bitfield_t(bitfield_t& a) { bitmask = a; }
            bitfield_t(bitfield_t&& a) { bitmask = std::move(a.bitmask); }

            // typed operator 
            operator eS&() { return bitmask; }
            operator const eS&() const { return bitmask; }
            operator enum_u&() { return bitmask_u32; }
            operator const enum_u&() const { return bitmask_u32; }
            operator const E() const { return (const E&)(bitmask_u32); } // read-only getting base enum

            // and
            bitfield_t operator&(const enum_u& a) const { return bitfield_t(enum_u(*this) & a); }
            bitfield_t operator&(const E& a) const { return (*this) & enum_u(a); }
            bitfield_t operator&(const bitfield_t& a) const { return (*this) & enum_u(a); }
            bitfield_t operator&(const eS& a) const { return (*this) & enum_u(a); }

            // or
            bitfield_t operator|(const enum_u& a) const { return bitfield_t(enum_u(*this) | a); }
            bitfield_t operator|(const E& a) const { return (*this) | enum_u(a); }
            bitfield_t operator|(const bitfield_t& a) const { return (*this) | enum_u(a); }
            bitfield_t operator|(const eS& a) const { return (*this) | enum_u(a); }

            // xor
            bitfield_t operator^(const enum_u& a) const { return bitfield_t(enum_u(*this) ^ a); }
            bitfield_t operator^(const E& a) const { return (*this) ^ enum_u(a); }
            bitfield_t operator^(const bitfield_t& a) const { return (*this) ^ enum_u(a); }
            bitfield_t operator^(const eS& a) const { return (*this) ^ enum_u(a); }
            
            // get access to native bits
            eS& operator->() { return bitmask; };
            const eS& operator->() const { return bitmask; };
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

        // insider handler (TODO: cast from original type i.e. `H`)
        template<class H, class vH = uintptr_t> // first and just handler, second may be class with methods
        class handle_ptr {
            protected:
                union { H _hnd = H(0ull); vH _hni; }; // alternative class/handler
                using handle_ptr_t = handle_ptr<vH,H>;
                friend handle_ptr_t;

            public:
                constexpr handle_ptr_t() : _hnd(0ull) {};
                //constexpr UHandler(const void * a) : _hnd(uintptr_t(a)) {}; // for nullptr arguments
                handle_ptr_t(const void * a) : _hnd(uintptr_t(a)) {}; // include, for nullptr arguments
                handle_ptr_t(const intptr_t& a) : _hnd(uintptr_t(a)) {};
                handle_ptr_t(const uintptr_t& a) : _hnd(a) {};
                handle_ptr_t(const int& a) : _hnd(uintptr_t(a)) {};
                handle_ptr_t(const unsigned& a) : _hnd(a) {};
                handle_ptr_t(handle_ptr_t& a) : _hnd(a._hnd) {};
                handle_ptr_t(handle_ptr_t&& a) : _hnd(std::move(a._hnd)) {};
                handle_ptr_t(const vH& a) : _hni(a) {}; // foreign handler (from C API)
                handle_ptr_t(const H& a) : _hnd(a) {};

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

        template<class T, class R = uint32_t> // T SHOULD BE SHARED_PTR<T_n>
        class handle_ref { 
            protected: std::pair<T,R> pair;
            using handle_ref_t = handle_ref<T,R>; 
            friend T; friend R; 
            friend handle_ref_t;

            // 
            public: handle_ref_t(const T& that, R result = 0u): pair(std::pair<T,R>(that,result)) {};
            public: handle_ref_t(T&& that, R result = 0u): pair(std::pair<T,R>(that,result)) {};
            public: handle_ref_t(const std::pair<T,R>& pair = {}) : pair(pair) {};
            public: handle_ref_t(std::pair<T,R>&& pair = {}) : pair(pair) {};

            // accept another typed pointer
            template<class H = T> handle_ref<H>(H&& object, R result = 0u) : pair(std::pair<T,R>(T(object),result)) {};
            template<class H = T> handle_ref<H>(const H& object, R result = 0u) : pair(std::pair<T,R>(T(object),result)) {};
            template<class H = T> handle_ref<H>(std::pair<H,R>&& pair = {}) : pair(pair) {};
            template<class H = T> handle_ref<H>(const std::pair<H,R>& pair = {}) : pair(pair) {};

            // 
            operator std::pair<T,R>&(){ return pair; };
            operator std::pair<const T,const R>&() const { return pair; };

            // 
            operator const T&() const { return pair.first; };
            operator const R&() const { return pair.second; };
            operator T&(){ return pair.first; };
            operator R&(){ return pair.second; };

            // return ptr reference
            T& ref() { return pair.first; };
            const T& ref() const { return pair.first; };

            // 
            template<class H = T> H cst() {return H(pair.first); };
            template<class H = T> const H cst() const {return H(pair.first); };

            // 
            template<class H = T> handle_ref<H,R>& hnd() {return (handle_ref<H,R>&)(*this); };
            template<class H = T> const handle_ref<H,R>& hnd() const {return (handle_ref<H,R>&)(*this); };

            // tribute handler (bit danger with handler)
            T& operator->() { return that; };
            const T& operator->() const { return that; };
        };

    };

};
