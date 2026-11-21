#include "vpx/vpx_codec.h"
static const char* const cfg = "--target=generic-gnu --enable-vp8-decoder --disable-vp8-encoder --disable-vp9";
const char *vpx_codec_build_config(void) { return cfg; }
