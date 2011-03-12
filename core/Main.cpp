#include "BitWave.h"

int main(int argc, const char **argv)
{
    bittorrent::core::BitWave wave;
    bittorrent::core::BitNetWaveObject net_wave_object;
    bittorrent::core::BitCoreControlObject core_control_object;
    wave.AddWaveObject(&net_wave_object);
    wave.AddWaveObject(&core_control_object);
    wave.Wave();

    return 0;
}
