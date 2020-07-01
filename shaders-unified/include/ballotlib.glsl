#ifndef _BALLOTLIB_H
#define _BALLOTLIB_H

#include "../include/mathlib.hlsli"

// for constant maners
#ifndef Wave_Size
    #if (defined(AMD_PLATFORM))
        #define bqtype_t uint64_t
        #define bqtype2 u64float2
        #define Wave_Size 64u
        #define bqualf highp
        bqtype_t extblt(in uint4 blt){return pack64(blt.xy);};
        bqtype2 extbl2(in uint4 blt){return bqtype2(pack64(blt.xy),pack64(blt.zw));};
        bool bltinv(in bqtype_t a){return subgroupInverseBallot(uint4(unpack32(a),0u.xx));};
    #else
        #define bqtype_t uint
        #define bqtype2 u32float2
        #define Wave_Size 32u
        #define bqualf lowp
        bqtype_t extblt(in uint4 blt){return blt.x;};
        bqtype2 extbl2(in uint4 blt){return blt.xy;};
        bool bltinv(in bqtype_t a){return subgroupInverseBallot(uint4(a,0u.xxx));};
        //bool bltinv(in bqtype_t a) { const int l = int(gl_SubgroupInvocationID); return bool(bitfieldExtract(a,l,1)); };
    #endif
#endif

// Z-swizzle for 
#define Ni 2u

#ifdef IS_RAY_SHADER
#define WID gl_LaunchIDNV[Ni].x // shuld be uniform (scalar)
#else
#define WID gl_GlobalInvocationID[Ni].x // shuld be uniform (scalar)
#endif

#define Wave_Size_RT gl_SubgroupSize
#define Wave_Count_RT gl_NumSubgroups

#ifndef OUR_INVOC_TERM
    #define Launch_Idx gl_GlobalInvocationID
    #define Local_Idx gl_LocalInvocationIndex
    #define Wave_Idx gl_SubgroupID
    #define Lane_Idx gl_SubgroupInvocationID
#endif

// 
#define uint_ballot uint4
#define readLane subgroupBroadcast
#define readFLane subgroupBroadcastFirst
#define electedInvoc subgroupElect

// subgroup barriers
//#define LGROUP_BARRIER subgroupBarrier();//memoryBarrier(),subgroupBarrier();
#define LGROUP_BARRIER subgroupBarrier();
#define IFALL(b) [[flatten]]if(subgroupAll(b))
#define IFANY(b)            if(subgroupAny(b))

const uint UONE = 1u;
//lowp uint2 bPrefixSum(in bool val) { return uint2(subgroupAdd(uint(val)), subgroupExclusiveAdd(uint(val))); };
//lowp uint2 bPrefixSum() { return uint2(subgroupAdd(UONE), subgroupExclusiveAdd(UONE)); };

lowp uint2 bPrefixSum() {
    const bqualf uint4 ballot = subgroupBallot(true);
    return uint2(subgroupBallotBitCount(ballot), subgroupBallotExclusiveBitCount(ballot));
    //return uint2(subgroupAdd(UONE), subgroupExclusiveAdd(UONE)); 
};

lowp uint bSum() {
    return subgroupBallotBitCount(subgroupBallot(true));
};

#define initAtomicSubgroupIncFunction(mem, fname, by, T)\
T fname() {\
    const lowp uint2 pfx = bPrefixSum();\
    T gadd = 0; [[flatten]] if (subgroupElect()) {gadd = atomicAdd(mem, T(pfx.x) * T(by));}; gadd = readFLane(gadd);\
    return T(pfx.y) * T(by) + gadd;\
};

#define initAtomicSubgroupIncFunctionTarget(mem, fname, by, T)\
T fname(in  uint WHERE) {\
    const lowp uint2 pfx = bPrefixSum();\
    T gadd = 0; [[flatten]] if (subgroupElect()) {gadd = atomicAdd(mem, T(pfx.x) * T(by));}; gadd = readFLane(gadd);\
    return T(pfx.y) * T(by) + gadd;\
};

#define initAtomicSubgroupIncFunctionTargetBinarity(mem, fname, by, T)\
T fname(in  uint WHERE) {\
    const lowp uint2 pfx = bPrefixSum();\
    T gadd = 0; [[flatten]] if (subgroupElect()) {gadd = atomicAdd(mem[WID], T(pfx.x) * T(by));}; gadd = readFLane(gadd);\
    return T(pfx.y) * T(by) + gadd;\
};

// statically multiplied
#define initSubgroupIncFunctionTarget(mem, fname, by, T)\
T fname(in  uint WHERE) {\
    const lowp uint2 pfx = bPrefixSum();\
    T gadd = 0; [[flatten]] if (subgroupElect()) {gadd = atomicAdd(mem, T(pfx.x) * T(by), gl_ScopeWorkgroup, gl_StorageSemanticsShared, gl_SemanticsRelaxed);}; gadd = readFLane(gadd);\
    return T(pfx.y) * T(by) + gadd;\
};

// statically multiplied
#define initSubgroupIncReducedFunctionTarget(mem, fname, by, T)\
void fname(in  uint WHERE) {\
    const lowp uint pfx = bSum();\
    if (subgroupElect()) {atomicAdd(mem, T(pfx.x) * T(by), gl_ScopeWorkgroup, gl_StorageSemanticsShared, gl_SemanticsRelaxed);};\
};



bqualf uint4 sgrblt(in bool k) { return subgroupBallot(k); };

//#ifdef ENABLE_TURING_INSTRUCTION_SET // better only for Turing GPU's
#if (defined(AMD_PLATFORM))
highp uint4 sgrblt(in bool2 k) { return interleave64x2(u32x4_t(
#else
highp uint4 sgrblt(in bool2 k) { return interleave32x2(u32x4_t(
#endif
    subgroupBallot(k[0]).x,
    subgroupBallot(k[1]).x,
    0u.xx
));};
uint4 sgrblt(in bool4 k) { return interleave32x4(u32x4_t(
    subgroupBallot(k[0]).x,
    subgroupBallot(k[1]).x,
    subgroupBallot(k[2]).x,
    subgroupBallot(k[3]).x
));};
//#endif

#ifdef ENABLE_TURING_INSTRUCTION_SET
bqualf uint4 sgrprt(in lowp uint k) { return subgroupPartitionNV(k); };
highp uint4 sgrprt(in lowp uint2 k) { return interleave32x2(u32x4_t(
    subgroupPartitionNV(k[0]).x,
    subgroupPartitionNV(k[1]).x,
    0u.xx
));};
uint4 sgrprt(in lowp uint4 k) { return interleave32x4(u32x4_t(
    subgroupPartitionNV(k[0]).x,
    subgroupPartitionNV(k[1]).x,
    subgroupPartitionNV(k[2]).x,
    subgroupPartitionNV(k[3]).x
));};
#endif

// 64-bit/64-lane bitfield mask only supported 
#ifdef ENABLE_TURING_INSTRUCTION_SET
highp uint4 genLt2Mask(in uint N) {
    const lowp uint cthread = ((Lane_Idx << 1u) + N);
    const uint64_t tmask = (1ul << uint64_t(cthread))-1ul;
    return uint4(unpack32(tmask),0u.xx);
};
#endif

bqualf uint4 genLtMask(){ return gl_SubgroupLtMask; };


#endif
