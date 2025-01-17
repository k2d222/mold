#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <core/md_str.h>
#include <core/md_vec_math.h>

// Forward declarations
struct md_allocator_i;

typedef struct md_molecule_o md_molecule_o; // Opaque data blob

typedef int32_t                     md_atom_idx_t;
typedef int32_t                     md_residue_idx_t;
typedef int32_t                     md_backbone_idx_t;
typedef int32_t                     md_residue_id_t;
typedef int32_t                     md_chain_idx_t;
typedef int32_t                     md_molecule_idx_t;
typedef uint32_t                    md_secondary_structure_t;
typedef uint8_t                     md_flags_t;
typedef uint8_t                     md_element_t;
typedef uint8_t                     md_ramachandran_type_t;
typedef mat3_t                      md_coordinate_frame_t;

// We are sneaky, we encode the secondary structure as a uint8x4 unorm where the the components encode the fraction of each secondary structure type
enum {
    MD_SECONDARY_STRUCTURE_UNKNOWN = 0,
    MD_SECONDARY_STRUCTURE_COIL    = 0x000000FF,
    MD_SECONDARY_STRUCTURE_HELIX   = 0x0000FF00,
    MD_SECONDARY_STRUCTURE_SHEET   = 0x00FF0000
};

enum {
    MD_RAMACHANDRAN_TYPE_UNKNOWN    = 0,
    MD_RAMACHANDRAN_TYPE_GENERAL    = 1,
    MD_RAMACHANDRAN_TYPE_GLYCINE    = 2,
    MD_RAMACHANDRAN_TYPE_PROLINE    = 3,
    MD_RAMACHANDRAN_TYPE_PREPROL    = 4,
};

// Open ended range of indices (e.g. range(0,4) -> [0,1,2,3])
typedef struct md_range_t {
    int32_t beg;
    int32_t end;
} md_range_t;

// Single bond between two entities represented by the indices
typedef struct md_bond_t {
    md_atom_idx_t idx[2];
} md_bond_t;

typedef struct md_backbone_atoms_t {
    md_atom_idx_t n;
    md_atom_idx_t ca;
    md_atom_idx_t c;
    md_atom_idx_t o;
} md_backbone_atoms_t;

typedef struct md_backbone_angles_t {
    float phi;
    float psi;
} md_backbone_angles_t;

typedef struct md_label_t {
    char    buf[7];
    uint8_t len;

#ifdef __cplusplus
    constexpr operator str_t() { return {buf, len}; }
#endif
} md_label_t;

typedef struct md_molecule_atom_data_t {
    int64_t count;
    // Coordinates
    float* x;
    float* y;
    float* z;
    // Velocities
    float* vx;
    float* vy;
    float* vz;
    // Misc
    float* radius;
    float* mass;
    uint8_t* valence;
    md_element_t* element;
    md_label_t* name;
    md_flags_t* flags;                          // Auxillary bit buffer for flagging individual atoms
    md_residue_idx_t* residue_idx;
    md_chain_idx_t* chain_idx;
} md_molecule_atom_data_t;

typedef struct md_molecule_residue_data_t {
    int64_t count;
    md_label_t* name;
    md_residue_id_t* id;
    md_range_t* atom_range;
    md_range_t* internal_covalent_bond_range;   // Range of covalent bonds within the resuidue
    md_range_t* complete_covalent_bond_range;   // Range of covalent bonds that in anyway is part of the residue
} md_molecule_residue_data_t;

typedef struct md_molecule_chain_data_t {
    int64_t count;
    md_label_t* id;
    md_range_t* residue_range;
    md_range_t* atom_range;
    md_range_t* backbone_range;
} md_molecule_chain_data_t;

typedef struct md_molecule_backbone_data_t {
    int64_t count;
    md_backbone_atoms_t* atoms;
    md_backbone_angles_t* angle;
    md_secondary_structure_t* secondary_structure;
    md_ramachandran_type_t* ramachandran_type;
    md_residue_idx_t* residue_idx;            // Index to the residue which contains the backbone
} md_molecule_backbone_data_t;

typedef struct md_molecule_bond_data_t {
    int64_t count;
    md_bond_t* bond;
} md_molecule_bond_data_t;

typedef struct md_molecule_t {
    md_molecule_o* inst;

    md_coordinate_frame_t   coord_frame;
    md_molecule_atom_data_t atom;
    md_molecule_residue_data_t residue;
    md_molecule_chain_data_t chain;
    md_molecule_backbone_data_t backbone;
    md_molecule_bond_data_t covalent_bond;
    md_molecule_bond_data_t hydrogen_bond;

} md_molecule_t;

#if 0
// This is not really well defined yet since it has no real use case so far.
struct md_macro_molecule {
    int64_t num_molecules;
    md_molecule* molecules;

    struct {
        int64_t             count;
        md_molecule_idx_t*    idx;
        const char**        label;
        float               (*transform)[4][4];
    } instance;
};
#endif


/*

The molecule api is just a convenience API for abstracing the functionality of initializing and freeing molecule data

The reason for providing a distinct function for initializing from file is that some molecule files can
also contain their trajectories, such as PDB files. In such case, the whole file would have to be read and passed, but for
molecule data only the first part of the file is used.

*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct md_molecule_api {
    bool (*init_from_str) (md_molecule_t* mol, str_t string,   struct md_allocator_i* alloc);
    bool (*init_from_file)(md_molecule_t* mol, str_t filename, struct md_allocator_i* alloc);
    bool (*free)(md_molecule_t* mol, struct md_allocator_i* alloc);
} md_molecule_api;

#define MAKE_LABEL(cstr) {cstr"", sizeof(cstr)-1}

static inline md_label_t make_label(str_t str) {
    md_label_t lbl = {0};
    if (str.ptr) {
        const int64_t len = MIN(str.len, (int64_t)sizeof(lbl.buf) - 1);
        for (int64_t i = 0; i < len; ++i) {
            lbl.buf[i] = str.ptr[i];
        }
        lbl.len = (uint8_t)len;
    }
    return lbl;
}

// @NOTE(Robin): This needs to be a pointer as we extract the ptr from the structure and thus cannot be copied as a parameter
static inline str_t label_to_str(const md_label_t* lbl) {
    ASSERT(lbl);
    str_t str = {lbl->buf, lbl->len};
    return str;
}

#ifdef __cplusplus
}
#endif
