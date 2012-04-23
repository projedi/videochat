#include "converter.h"

void ffinit() {
    avcodec_register_all();
    av_register_all();
    avformat_network_init();
}
