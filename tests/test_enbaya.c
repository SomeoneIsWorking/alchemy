#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "igb_anim.h"
#include "igb_internal.h"

static int closef(float a, float b, float eps) { return fabsf(a - b) < eps; }

/* Oracle: scratch/logs/blob666.bin, the 666-byte Enbaya stream of the
 * Wolverine walk (03_wolverine.IGB object 2159 -> blob 1136). Not committed
 * to git (game asset); the test skips when the data is absent. */
static void test_synthetic_stream(void) {
  /* Authored one-track stream: quaternion w=1, x position=2, then x += 1.
   * Seven initial tags use two bytes; one delta tag and one run tag follow. */
  uint8_t blob[160] = {0};
  blob[4] = 1;
  blob[10] = 0x80;
  blob[11] = 0x3f; /* scale = 1.0, encoded little-endian */
  blob[14] = 0x80;
  blob[15] = 0x3f; /* duration = 1.0 */
  blob[16] = 1;    /* fps */
  blob[0x14] = 2;  /* initial tag bytes */
  blob[0x18] = 2;  /* initial value bytes */
  blob[0x24] = 1;  /* delta tag byte */
  blob[0x38] = 1;  /* run tag byte */
  blob[0x50] = 0x01;
  blob[0x51] = 0x40;
  blob[0x52] = 1;
  blob[0x53] = 2;
  blob[0x54] = 0x40;
  blob[0x56] = 0x10; /* x position channel receives the delta */
  igb_enbaya_anim animation;
  assert(igb_enbaya_decode(blob, sizeof(blob), &animation) == 0);
  assert(animation.track_count == 1 && animation.frame_count == 2);
  assert(closef(animation.poses[0].quat[3], 1.0f, 1e-6f));
  assert(closef(animation.poses[0].pos[0], 2.0f, 1e-6f));
  assert(closef(animation.poses[1].pos[0], 3.0f, 1e-6f));
  igb_enbaya_pose pose;
  igb_enbaya_pose_at(&animation, -1.0f, &pose);
  assert(closef(pose.pos[0], 2.0f, 1e-6f));
  igb_enbaya_pose_at(&animation, 100.0f, &pose);
  assert(closef(pose.pos[0], 2.0f, 1e-6f));
  igb_enbaya_free(&animation);
  assert(animation.poses == NULL && animation.frame_count == 0);
  assert(igb_enbaya_decode(blob, 1, &animation) == -1);
  blob[4] = 0;
  assert(igb_enbaya_decode(blob, sizeof(blob), &animation) == -1);
  puts("Enbaya synthetic: initial pose, delta, clamping, cleanup and invalid input passed");
}

int main(int argc, char **argv) {
  if (argc == 1) {
    test_synthetic_stream();
    return 0;
  }
  assert(argc == 2 && strcmp(argv[1], "--real") == 0);
  const char *path = "scratch/logs/blob666.bin";
  FILE *fp = igb_file_open(path, "rb");
  if (!fp) {
    printf("test_enbaya: SKIP (%s not present)\n", path);
    return 77;
  }
  uint8_t blob[666];
  size_t n = fread(blob, 1, sizeof(blob), fp);
  fclose(fp);
  if (n != sizeof(blob)) {
    fprintf(stderr, "test_enbaya: corrupt corpus (expected 666 bytes, read %zu)\n", n);
    return 1;
  }

  igb_enbaya_anim a;
  int rc = igb_enbaya_decode(blob, sizeof(blob), &a);
  assert(rc == 0);
  assert(a.track_count == 33);
  assert(a.frame_count == 7);
  assert(closef(a.duration, 0.3f, 1e-5f));
  assert(closef(a.interval, 0.05f, 1e-6f));

  /* t0 (root) stays identity through the cycle. */
  for (int f = 0; f < a.frame_count; ++f) {
    const igb_enbaya_pose *p = &a.poses[(size_t)f * a.track_count + 0];
    assert(closef(p->quat[3], 1.0f, 1e-4f));
    assert(closef(p->quat[0], 0.0f, 1e-5f));
    assert(closef(p->quat[1], 0.0f, 1e-5f));
    assert(closef(p->quat[2], 0.0f, 1e-5f));
  }

  /* Reference values from the Python oracle (scratch/logs/dec2.py) with the
   * engine's quaternion normalization applied. */
  const igb_enbaya_pose *f1 = &a.poses[a.track_count + 1];
  assert(closef(f1->quat[0], -0.0360f, 1e-3f));
  assert(closef(f1->quat[1], -0.1279f, 1e-3f));
  assert(closef(f1->quat[2], 0.0720f, 1e-3f));
  assert(closef(f1->pos[0], 0.148f, 1e-2f));
  assert(closef(f1->pos[1], -0.832f, 1e-2f));
  assert(closef(f1->pos[2], 29.740f, 1e-2f));

  const igb_enbaya_pose *f5 = &a.poses[(size_t)5 * a.track_count + 1];
  assert(closef(f5->quat[0], -0.0441f, 1e-3f));
  assert(closef(f5->quat[1], -0.2442f, 1e-3f));
  assert(closef(f5->quat[2], 0.0682f, 1e-3f));
  assert(closef(f5->pos[0], -0.712f, 1e-2f));
  assert(closef(f5->pos[2], 28.716f, 1e-2f));

  /* Step 6 still applies deltas to tracks 9/11/29/30 (quat y/z channels). */
  const igb_enbaya_pose *f6t9 = &a.poses[(size_t)6 * a.track_count + 9];
  assert(closef(f6t9->quat[0], -0.33827f, 1e-3f));
  assert(closef(f6t9->quat[1], -0.43589f, 1e-3f));
  assert(closef(f6t9->quat[2], 0.15477f, 1e-3f));
  assert(closef(f6t9->quat[3], 0.81952f, 1e-3f));
  const igb_enbaya_pose *f6t11 = &a.poses[(size_t)6 * a.track_count + 11];
  assert(closef(f6t11->quat[1], -0.31195f, 1e-3f));
  const igb_enbaya_pose *f6t29 = &a.poses[(size_t)6 * a.track_count + 29];
  assert(closef(f6t29->quat[2], 0.84393f, 1e-3f));
  const igb_enbaya_pose *f6t30 = &a.poses[(size_t)6 * a.track_count + 30];
  assert(closef(f6t30->quat[1], 0.03094f, 1e-3f));

  /* pose_at clamps and samples the right keyframe. */
  igb_enbaya_pose *at = (igb_enbaya_pose *)malloc((size_t)a.track_count * sizeof(*at));
  assert(at);
  igb_enbaya_pose_at(&a, 0.11f, at);
  for (int t = 0; t < a.track_count; ++t) {
    const igb_enbaya_pose *p2 = &a.poses[(size_t)2 * a.track_count + t];
    assert(closef(at[t].quat[0], p2->quat[0], 1e-4f));
    assert(closef(at[t].pos[1], p2->pos[1], 1e-4f));
  }
  free(at);

  igb_enbaya_free(&a);
  printf("test_enbaya: OK\n");
  return 0;
}
