#include "viterbi27.h"
#include "common/utils.h"

namespace viterbi
{
    Viterbi27::Viterbi27(int frame_size, std::vector<int> polys, int ber_test_size) : cc_decoder_in(frame_size, 7, 2, polys, 0, -1, CC_STREAMING, false),
                                                                                      cc_encoder_in(ber_test_size / 2, 7, 2, polys, 0, CC_STREAMING, false),
                                                                                      d_ber(0),
                                                                                      d_ber_test_size(ber_test_size),
                                                                                      d_frame_size(frame_size)
    {
        hard_buffer = new uint8_t[frame_size * 2];
        buffer_deco = new uint8_t[frame_size];
        buffer_enco = new uint8_t[ber_test_size];

        bitc = 0;
        bytec = 0;
        shifter = 0;
    }

    Viterbi27::~Viterbi27()
    {
        delete[] hard_buffer;
        delete[] buffer_deco;
        delete[] buffer_enco;
    }

    void Viterbi27::work(int8_t *in, uint8_t *out)
    {
        // Convert to hard symbols
        char_array_to_uchar(in, hard_buffer, d_frame_size * 2);

        // Decode
        cc_decoder_in.generic_work(hard_buffer, buffer_deco);

        // Repack to bytes
        bitc = 0;
        bytec = 0;
        for (int i = 0; i < d_frame_size; i++)
        {
            shifter = shifter << 1 | buffer_deco[i];
            bitc++;

            if (bitc == 8)
            {
                out[bytec++] = shifter;
                bitc = 0;
            }
        }

        // Compute BER
        cc_encoder_in.generic_work(buffer_deco, buffer_enco);

        float errors = 0;
        for (int i = 0; i < d_ber_test_size; i++)
            errors += (hard_buffer[i] > 0) != (buffer_enco[i] > 0);

        d_ber = (errors / ((float)d_ber_test_size * 2.0f)) * 4.0f;
    }

    float Viterbi27::ber()
    {
        return d_ber;
    }
}