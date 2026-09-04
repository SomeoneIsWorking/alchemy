#ifndef ALCHEMY_IGB_MESH_H
#define ALCHEMY_IGB_MESH_H

#include "igb.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float *pos;          /* nverts * 3 */
    float *nor;          /* nverts * 3, or NULL */
    uint8_t *col;        /* nverts * 4 RGBA, or NULL */
    float *uv;           /* nverts * 2, or NULL */
    int nverts;
    uint32_t *idx;       /* nidx */
    int nidx;
    uint32_t *prim_len;  /* num_prim */
    int num_prim;
    int prim_type;       /* IG_GFX_DRAW enum (3=triangles,4=strip,5=fan) */
    int img_idx;         /* igImage object index for the texture, or -1 */
    float world[16];     /* row-major local->world */
    char *name;
} igb_mesh;

typedef struct {
    igb_mesh *meshes;
    int n_meshes;
} igb_scene;

typedef enum {
    IGB_SCENE_DIAGNOSTIC_LOAD_STARTED = 0,
    IGB_SCENE_DIAGNOSTIC_ROOT_DISCOVERED,
    IGB_SCENE_DIAGNOSTIC_GEOMETRY_DECODED,
} igb_scene_diagnostic_kind;

typedef struct {
    igb_scene_diagnostic_kind kind;
    int object_index;
    const char *object_type;
    int texture_source_index;
    int image_index;
    int vertex_count;
} igb_scene_diagnostic;

typedef void (*igb_scene_diagnostic_fn)(void *context,
                                        const igb_scene_diagnostic *diagnostic);

typedef struct {
    int compose_transforms;
    igb_scene_diagnostic_fn diagnostic;
    void *diagnostic_context;
} igb_scene_options;

/* Walk the root igGroup(s), collect every igGeometry with its resolved
 * vertex/index/primitive buffers, texture image, and optional composed world
 * matrix. Configuration and diagnostics are supplied by the caller; the
 * parser never reads process configuration or writes output directly. */
int igb_scene_load(const igb *f, const igb_scene_options *options, igb_scene *out);
void igb_scene_free(igb_scene *out);

/* Object type / field helpers used by the scene walker. */
int igb_obj_is(const igb *f, int idx, const char *type);
int igb_obj_slot_i32(const igb *f, int idx, uint16_t slot);
const uint8_t *igb_obj_slot_blob(const igb *f, int idx, uint16_t slot, int *out_len);

#ifdef __cplusplus
}
#endif

#endif
