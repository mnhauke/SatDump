#pragma once

/*
    An arbitrary deframer
*/

#include <vector>
#include <array>
#include <cstdint>

namespace fengyun
{
    namespace waai
    {
        class WAAIDeframer
        {
        private:
            // Main shifter
            uint64_t shifter;
            // Small function to push a bit into the frame
            void pushBit(uint8_t bit);
            // Framing variables
            uint8_t byteBuffer;
            bool writeFrame;
            int wroteBits, outputBits;
            std::vector<uint8_t> frameBuffer;

        public:
            WAAIDeframer();
            std::vector<std::vector<uint8_t>> work(uint8_t *input, int size);
        };
    } // namespace virr
} // namespace fengyun