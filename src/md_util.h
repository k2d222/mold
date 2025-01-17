#pragma once

#include "md_molecule.h"
#include <core/md_str.h>
#include <core/md_vec_math.h>

struct md_allocator_i;

#ifdef __cplusplus
extern "C" {
#endif

// This assumes the string exactly matches the value within the look up table
// The match is case sensitive and expects elements to be formatted with Big first letter and small second letter:
// E.g. H, He, Fe, Na, C
md_element_t md_util_element_lookup(str_t element_str);

str_t md_util_element_symbol(md_element_t element);
str_t md_util_element_name(md_element_t element);

float md_util_element_vdw_radius(md_element_t element);
float md_util_element_covalent_radius(md_element_t element);
float md_util_element_atomic_mass(md_element_t element);
int   md_util_element_max_valence(md_element_t element);
uint32_t md_util_element_cpk_color(md_element_t element);

bool md_util_resname_dna(str_t str);
bool md_util_resname_acidic(str_t str);
bool md_util_resname_basic(str_t str);
bool md_util_resname_neutral(str_t str);
bool md_util_resname_water(str_t str);
bool md_util_resname_hydrophobic(str_t str);
bool md_util_resname_amino_acid(str_t str);

static inline bool md_util_backbone_atoms_valid(md_backbone_atoms_t prot) {
    return (prot.ca != prot.c) && (prot.ca != prot.o) && (prot.c != prot.o);
}

// This operation tries to deduce the element from the atom type/name which usually contains alot of cruft.
// It also tries resolve some ambiguities: Such as CA, is that Carbon Alpha or is it calcium?
// We can resolve that by looking at the residue name and in the case of Carbon Alpha, the residue name should be matched to an amino acid.
bool md_util_element_decode(md_element_t element[], int64_t capacity, const struct md_molecule_t* mol);

// Extracts the indices which make up the backbone from the supplied atom names within corresponding residues
bool md_util_backbone_atoms_extract(md_backbone_atoms_t backbone_atoms[], int64_t capacity, const struct md_molecule_t* mol);

// Computes secondary structures from backbone atoms
// Does not allocate any data, it assumes that secondary_structures has the same length as args->backbone.count
bool md_util_backbone_secondary_structure_compute(md_secondary_structure_t secondary_structures[], int64_t capacity, const struct md_molecule_t* mol);

// Computes backbone angles from backbone atoms
// Does not allocate any data, assumes that backbone_angles has the same length as args->backbone.count
bool md_util_backbone_angles_compute(md_backbone_angles_t backbone_angles[], int64_t capacity, const struct md_molecule_t* mol);

// Classifies the ramachandran type (General / Glycine / Proline / Preproline) from the residue name
bool md_util_backbone_ramachandran_classify(md_ramachandran_type_t ramachandran_types[], int64_t capacity, const struct md_molecule_t* mol);

// the result is an md_array allocated using the supplied allocator, since we cannot determine how many bonds will be formed.
bool md_util_extract_covalent_bonds(struct md_molecule_t* mol, struct md_allocator_i* alloc);
bool md_util_extract_hydrogen_bonds(struct md_molecule_t* mol, struct md_allocator_i* alloc);

// Generates missing data such as covalent bonds, chains, secondary structures, backbone angles etc.
bool md_util_postprocess_molecule(struct md_molecule_t* mol, struct md_allocator_i* alloc);

// Compute a mat3 basis from cell extents a,b,c and cell axis angles alpha, beta, gamma (in degrees)
mat3_t md_util_compute_unit_cell_basis(double a, double b, double c, double alpha, double beta, double gamma);
vec3_t md_util_compute_unit_cell_extent(mat3_t M);

// Applies periodic boundary conditions to coordinates of atoms within molecule
// It ensures that residues are
bool md_util_apply_pbc(struct md_molecule_t* mol, vec3_t pbc_ext);

// Structure Of Array layout version of vec3_t
// This is to simplify the interfaces a bit when dealing with multiple coordinate streams
typedef struct md_vec3_soa_t {
    float* x;
    float* y;
    float* z;
} md_vec3_soa_t;

// Convenience functions to extract vec3_soa streams from molecule
static inline md_vec3_soa_t md_molecule_coord(md_molecule_t* mol) {
    ASSERT(mol);
    md_vec3_soa_t soa = {mol->atom.x, mol->atom.y, mol->atom.z};
    return soa;
}

static inline md_vec3_soa_t md_molecule_vel(md_molecule_t* mol) {
    ASSERT(mol);
    md_vec3_soa_t soa = {mol->atom.vx, mol->atom.vy, mol->atom.vz};
    return soa;
}

// Computes the center of mass for a set of points with a given weight
// x, y, z -> Arrays containing coordinates
// w -> Array of weights (optional): set as NULL to use equal weights
// count -> Length of all arrays
vec3_t md_util_compute_com(const float* x, const float* y, const float* z, const float* w, int64_t count);

// Computes the center of mass for a set of points with a given weight given in periodic boundary conditions
// x, y, z -> Arrays containing coordinates
// w -> Array of weights (optional): set as NULL to use equal weights
// count -> Length of all arrays
// pbc_ext -> Extent of periodic boundary (optional): Set to zero if pbc does not apply in that dimension
vec3_t md_util_compute_com_periodic(const float* x, const float* y, const float* z, const float* w, int64_t count, vec3_t pbc_ext);

// Computes the optimal rotation between two configurations of a set of points with corresponding weights weights
// coords -> coordinate arrays [2] (x0, y0, z0), (x1, y1, z1)
// com -> center of mass [2] (xyz0), (xyz1)
// w -> array of weights (optional): set as NULL to use equal weights
// count -> Length of all arrays (coords + w)
mat3_t md_util_compute_optimal_rotation(const md_vec3_soa_t coord[2], const vec3_t com[2], const float* w, int64_t count);

// Computes the similarity between two sets of points with given weights.
// One of the sets is rotated and translated to match the other set in an optimal fashion before the similarity is computed.
// The rmsd is the root mean squared deviation between the two sets of aligned vectors.
// coords -> coordinate arrays [2] (x0, y0, z0), (x1, y1, z1)
// com -> center of mass [2] (xyz0), (xyz1)
// w -> array of weights (optional): set as NULL to use equal weights
// count -> Length of all arrays (coords + w)
double md_util_compute_rmsd(const md_vec3_soa_t coord[2], const vec3_t com[2], const float* w, int64_t count);

// Perform linear interpolation of supplied coordinates
// dst_coord -> destination arrays (x,y,z)
// src_coord -> source arrays [2] (x0, y0, z0), (x1, y1, z1)
// count -> count of coordinates (this implies that all coordinate arrays must be equal in length)
// pbc_ext -> Extent of periodic boundary (optional) set to zero if should be ignored
// t -> interpolation factor (0..1)
bool md_util_linear_interpolation(md_vec3_soa_t dst_coord, const md_vec3_soa_t src_coord[2], int64_t count, vec3_t pbc_ext, float t);

// Perform cubic interpolation of supplied coordinates
// dst_coord -> destination arrays (x,y,z)
// src_coord -> source arrays [4] (x0, y0, z0), (x1, y1, z1), (x2, y2, z2), (x3, y3, z3)
// count -> count of coordinates (this implies that all coordinate arrays must be equal in length)
// pbc_ext -> Extent of periodic boundary (optional): Set to zero if pbc does not apply in that dimension
// t -> interpolation factor (0..1)
// tension -> tension factor (0..1), 0 is jerky, 0.5 corresponds to catmul rom, 1.0 is silky smooth
bool md_util_cubic_interpolation(md_vec3_soa_t dst_coord, const md_vec3_soa_t src_coord[4], int64_t count, vec3_t pbc_ext, float t, float tension);

#ifdef __cplusplus
}
#endif
