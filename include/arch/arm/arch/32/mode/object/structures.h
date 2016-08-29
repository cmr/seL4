/*
 * Copyright 2014, General Dynamics C4 Systems
 *
 * This software may be distributed and modified according to the terms of
 * the GNU General Public License version 2. Note that NO WARRANTY is provided.
 * See "LICENSE_GPLv2.txt" for details.
 *
 * @TAG(GD_GPL)
 */

#ifndef __ARCH_OBJECT_STRUCTURES_32_H
#define __ARCH_OBJECT_STRUCTURES_32_H

#include <config.h>
#include <assert.h>
#include <util.h>
#include <api/types.h>
#include <arch/types.h>
#include <arch/object/structures_gen.h>
#include <arch/machine/hardware.h>
#include <arch/machine/registerset.h>

typedef struct arch_tcb {
    /* saved user-level context of thread (72 bytes) */
    user_context_t tcbContext;
#ifdef CONFIG_ARM_HYPERVISOR_SUPPORT
    /* Pointer to associated VCPU. NULL if not associated.
     * tcb->vcpu->tcb == tcb. */
    struct vcpu* vcpu;
#endif
} arch_tcb_t;

enum vm_rights {
    VMNoAccess = 0,
    VMKernelOnly = 1,
    VMReadOnly = 2,
    VMReadWrite = 3
};
typedef word_t vm_rights_t;

#ifdef CONFIG_ARM_HYPERVISOR_SUPPORT
#define PGDE_SIZE_BITS 3
#define PDE_SIZE_BITS  3
#define PTE_SIZE_BITS  3
#define PGD_BITS 2
#define PD_BITS 11
#define PT_BITS 9
#define VCPU_SIZE_BITS 12
/* Generate a vcpu_t pointer from a vcpu block reference */
#define VCPU_PTR(r)       ((struct vcpu *)(r))
#define VCPU_REF(p)       ((unsigned int)(p))

#else /* CONFIG_ARM_HYPERVISOR_SUPPORT */
#define PDE_SIZE_BITS  2
#define PTE_SIZE_BITS  2
#define PD_BITS 12
#define PT_BITS 8
#endif /* CONFIG_ARM_HYPERVISOR_SUPPORT */


#define PDE_PTR(r) ((pde_t *)(r))
#define PDE_REF(p) ((unsigned int)p)

#define PDE_PTR_PTR(r) ((pde_t **)r)

compile_assert(pd_size_bits_sane, (PD_BITS + PDE_SIZE_BITS) == seL4_PageDirBits)
#define PD_PTR(r) ((pde_t *)(r))
#define PD_REF(p) ((unsigned int)p)

/* Page directory entries (PDEs) */
enum pde_type {
    PDEInvalid = 0,
    PDECoarse  = 1,
    PDEMapping = 2
};
typedef word_t pde_type_t;

#define PTE_PTR(r) ((pte_t *)r)
#define PTE_REF(p) ((unsigned int)p)

compile_assert(pt_size_bits_sane, PT_BITS + PTE_SIZE_BITS == seL4_PageTableBits)
#define PT_PTR(r) ((pte_t *)r)
#define PT_REF(p) ((unsigned int)p)


/* LPAE */
#define PGD_SIZE_BITS (PGD_BITS+PGDE_SIZE_BITS)
#define LPAE_PGDE_PTR(r) ((lpae_pde_t *)(r))
#define LPAE_PGDE_REF(p) ((unsigned int)p)
#define LPAE_PGDE_PTR_PTR(r) ((lpae_pde_t **)r)
#define LPAE_PGD_PTR(r) ((lpae_pde_t *)(r))
#define LPAE_PGD_REF(p) ((unsigned int)p)

#define LPAE_PTE_PTR(r) ((lpae_pte_t *)r)
#define LPAE_PTE_REF(p) ((unsigned int)p)

#define LPAE_PT_PTR(r) ((lpae_pte_t *)r)
#define LPAE_PT_REF(p) ((unsigned int)p)

#define WORD_SIZE_BITS 2

struct user_data {
    word_t words[BIT(ARMSmallPageBits) / sizeof(word_t)];
};

typedef struct user_data user_data_t;

enum asidSizeConstants {
#ifdef CONFIG_ARM_SMMU
    asidHighBits = 7,
#else
    asidHighBits = 8,
#endif
    asidLowBits = 10
};

struct asid_pool {
    pde_t* array[BIT(asidLowBits)];
};

typedef struct asid_pool asid_pool_t;

#define ASID_POOL_PTR(r) ((asid_pool_t *)r)
#define ASID_POOL_REF(p) ((unsigned int)p)

#define HW_ASID_SIZE_BITS 1

#define ASID_POOL_INDEX_BITS asidLowBits
compile_assert(asid_pool_size_sane, ASID_POOL_INDEX_BITS + WORD_SIZE_BITS == seL4_ASIDPoolBits)
#define ASID_BITS (asidHighBits+asidLowBits)

#define nASIDPools BIT(asidHighBits)

#define ASID_LOW(a) (a & MASK(asidLowBits))
#define ASID_HIGH(a) ((a >> asidLowBits) & MASK(asidHighBits))

static inline cap_t CONST
cap_small_frame_cap_set_capFMappedASID(cap_t cap, word_t asid)
{
    cap = cap_small_frame_cap_set_capFMappedASIDLow(cap,
                                                    asid & MASK(asidLowBits));
    return cap_small_frame_cap_set_capFMappedASIDHigh(cap,
                                                      (asid >> asidLowBits) & MASK(asidHighBits));
}

static inline word_t CONST
cap_small_frame_cap_get_capFMappedASID(cap_t cap)
{
    return (cap_small_frame_cap_get_capFMappedASIDHigh(cap) << asidLowBits) +
           cap_small_frame_cap_get_capFMappedASIDLow(cap);
}

static inline cap_t CONST
cap_frame_cap_set_capFMappedASID(cap_t cap, word_t asid)
{
    cap = cap_frame_cap_set_capFMappedASIDLow(cap,
                                              asid & MASK(asidLowBits));
    return cap_frame_cap_set_capFMappedASIDHigh(cap,
                                                (asid >> asidLowBits) & MASK(asidHighBits));
}

static inline word_t CONST
cap_frame_cap_get_capFMappedASID(cap_t cap)
{
    return (cap_frame_cap_get_capFMappedASIDHigh(cap) << asidLowBits) +
           cap_frame_cap_get_capFMappedASIDLow(cap);
}

static inline word_t CONST
generic_frame_cap_get_capFMappedASID(cap_t cap)
{
    cap_tag_t ctag;

    ctag = cap_get_capType(cap);

    assert(ctag == cap_small_frame_cap ||
           ctag == cap_frame_cap);

    if (ctag == cap_small_frame_cap) {
        return cap_small_frame_cap_get_capFMappedASID(cap);
    } else {
        return cap_frame_cap_get_capFMappedASID(cap);
    }
}

static inline cap_t CONST
generic_frame_cap_set_capFMappedAddress(cap_t cap, word_t asid, word_t addr)
{
    cap_tag_t ctag;

    ctag = cap_get_capType(cap);
    assert(ctag == cap_small_frame_cap ||
           ctag == cap_frame_cap);

    if (ctag == cap_small_frame_cap) {
        cap = cap_small_frame_cap_set_capFMappedASID(cap, asid);
        cap = cap_small_frame_cap_set_capFMappedAddress(cap, addr);
        return cap;
    } else {
        cap = cap_frame_cap_set_capFMappedASID(cap, asid);
        cap = cap_frame_cap_set_capFMappedAddress(cap, addr);
        return cap;
    }
}

static inline void
generic_frame_cap_ptr_set_capFMappedAddress(cap_t *cap_ptr, word_t asid,
                                            word_t addr)
{
    *cap_ptr = generic_frame_cap_set_capFMappedAddress(*cap_ptr, asid, addr);
}

static inline vm_rights_t CONST
generic_frame_cap_get_capFVMRights(cap_t cap)
{
    cap_tag_t ctag;

    ctag = cap_get_capType(cap);
    assert(ctag == cap_small_frame_cap ||
           ctag == cap_frame_cap);

    switch (ctag) {
    case cap_small_frame_cap:
        return cap_small_frame_cap_get_capFVMRights(cap);

    case cap_frame_cap:
        return cap_frame_cap_get_capFVMRights(cap);

    default:
        return VMNoAccess;
    }
}

static inline word_t CONST
generic_frame_cap_get_capFBasePtr(cap_t cap)
{
    cap_tag_t ctag;

    ctag = cap_get_capType(cap);
    assert(ctag == cap_small_frame_cap ||
           ctag == cap_frame_cap);

    switch (ctag) {
    case cap_small_frame_cap:
        return cap_small_frame_cap_get_capFBasePtr(cap);

    case cap_frame_cap:
        return cap_frame_cap_get_capFBasePtr(cap);

    default:
        return 0;
    }
}

static inline word_t CONST
generic_frame_cap_get_capFSize(cap_t cap)
{
    cap_tag_t ctag;

    ctag = cap_get_capType(cap);
    assert(ctag == cap_small_frame_cap ||
           ctag == cap_frame_cap);

    switch (ctag) {
    case cap_small_frame_cap:
        return ARMSmallPage;

    case cap_frame_cap:
        return cap_frame_cap_get_capFSize(cap);

    default:
        return 0;
    }
}

static inline word_t CONST
generic_frame_cap_get_capFIsMapped(cap_t cap)
{
    return generic_frame_cap_get_capFMappedASID(cap) != 0;
}

static inline word_t CONST
generic_frame_cap_get_capFMappedAddress(cap_t cap)
{
    cap_tag_t ctag;

    ctag = cap_get_capType(cap);
    assert(ctag == cap_small_frame_cap ||
           ctag == cap_frame_cap);

    if (ctag == cap_small_frame_cap) {
        return cap_small_frame_cap_get_capFMappedAddress(cap);
    } else {
        return cap_frame_cap_get_capFMappedAddress(cap);
    }
}

static inline word_t CONST
cap_get_archCapSizeBits(cap_t cap)
{
    cap_tag_t ctag;

    ctag = cap_get_capType(cap);

    switch (ctag) {
    case cap_small_frame_cap:
    case cap_frame_cap:
        return pageBitsForSize(generic_frame_cap_get_capFSize(cap));

    case cap_page_table_cap:
        return seL4_PageTableBits;

    case cap_page_directory_cap:
        return seL4_PageDirBits;

    case cap_asid_pool_cap:
        return seL4_ASIDPoolBits;

    case cap_asid_control_cap:
        return 0;

#ifdef CONFIG_ARM_HYPERVISOR_SUPPORT
    case cap_vcpu_cap:
        return VCPU_SIZE_BITS;
#endif
#ifdef CONFIG_ARM_SMMU
    case cap_io_page_table_cap:
        return seL4_IOPageTableBits;
#endif

    default:
        /* Unreachable, but GCC can't figure that out */
        return 0;
    }
}

static inline bool_t CONST
cap_get_archCapIsPhysical(cap_t cap)
{
    cap_tag_t ctag;

    ctag = cap_get_capType(cap);

    switch (ctag) {

    case cap_small_frame_cap:
        return true;

    case cap_frame_cap:
        return true;

    case cap_page_table_cap:
        return true;

    case cap_page_directory_cap:
        return true;

    case cap_asid_pool_cap:
        return true;

    case cap_asid_control_cap:
        return false;

#ifdef CONFIG_ARM_HYPERVISOR_SUPPORT
    case cap_vcpu_cap:
        return true;
#endif

#ifdef CONFIG_ARM_SMMU
    case cap_io_page_table_cap:
        return true;
#endif

    default:
        /* Unreachable, but GCC can't figure that out */
        return false;
    }
}

static inline void * CONST
cap_get_archCapPtr(cap_t cap)
{
    cap_tag_t ctag;

    ctag = cap_get_capType(cap);

    switch (ctag) {

    case cap_small_frame_cap:
    case cap_frame_cap:
        return (void *)(generic_frame_cap_get_capFBasePtr(cap));

    case cap_page_table_cap:
        return PT_PTR(cap_page_table_cap_get_capPTBasePtr(cap));

    case cap_page_directory_cap:
        return PD_PTR(cap_page_directory_cap_get_capPDBasePtr(cap));

    case cap_asid_pool_cap:
        return ASID_POOL_PTR(cap_asid_pool_cap_get_capASIDPool(cap));

    case cap_asid_control_cap:
        return NULL;

#ifdef CONFIG_ARM_HYPERVISOR_SUPPORT
    case cap_vcpu_cap:
        return VCPU_PTR(cap_vcpu_cap_get_capVCPUPtr(cap));
#endif

#ifdef CONFIG_ARM_SMMU
    case cap_io_page_table_cap:
        return (void *)(cap_io_page_table_cap_get_capIOPTBasePtr(cap));
#endif

    default:
        /* Unreachable, but GCC can't figure that out */
        return NULL;
    }
}

#ifndef CONFIG_ARM_HYPERVISOR_SUPPORT
/* We need to supply different type getters for the bitfield generated PTE type
 * because there is an implicit third type that PTEs can be. If the type bit is
 * set but the reserved bit is not set, the type of the PTE is invalid, not a
 * large PTE.
 */
enum { pte_pte_invalid = 2 };

static inline word_t CONST
pte_get_pteType(pte_t pte)
{
    if (pte_get_pteSize(pte) == pte_pte_small) {
        return pte_pte_small;
    } else if (pte_pte_large_get_reserved(pte) == 1) {
        return pte_pte_large;
    } else {
        return pte_pte_invalid;
    }
}

static inline word_t PURE
pte_ptr_get_pteType(pte_t *pte_ptr)
{
    if (pte_ptr_get_pteSize(pte_ptr) == pte_pte_small) {
        return pte_pte_small;
    } else if (pte_pte_large_ptr_get_reserved(pte_ptr) == 1) {
        return pte_pte_large;
    } else {
        return pte_pte_invalid;
    }
}
#endif /* CONFIG_ARM_HYPERVISOR_SUPPORT */

#endif /* __ARCH_OBJECT_STRUCTURES_32_H */
