#include "infrared2_reader.h"

#define WIDTH 5236
#define HEIGHT (1354 * 2)

namespace goes
{
    namespace gvar
    {
        InfraredReader2::InfraredReader2()
        {
            imageBuffer1 = new unsigned short[HEIGHT * WIDTH];
            imageBuffer2 = new unsigned short[HEIGHT * WIDTH];
            imageLineBuffer = new unsigned short[5252 * 4];
            goodLines = new bool[HEIGHT];
        }

        InfraredReader2::~InfraredReader2()
        {
            delete[] imageBuffer1;
            delete[] imageBuffer2;
            delete[] imageLineBuffer;
            delete[] goodLines;
        }

        void InfraredReader2::startNewFullDisk()
        {
            std::fill(&imageBuffer1[0], &imageBuffer1[HEIGHT * WIDTH], 0);
            std::fill(&imageBuffer2[0], &imageBuffer2[HEIGHT * WIDTH], 0);
            std::fill(&goodLines[0], &goodLines[HEIGHT], false);
        }

        void InfraredReader2::pushFrame(uint8_t *data, int counter, int word_cnt)
        {
            // Offset to start reading from
            int pos = 0;

            // Convert to 10 bits values
            for (int i = 0; i < 5252 * 4; i += 4)
            {
                imageLineBuffer[i] = (data[pos + 0] << 2) | (data[pos + 1] >> 6);
                imageLineBuffer[i + 1] = ((data[pos + 1] % 64) << 4) | (data[pos + 2] >> 4);
                imageLineBuffer[i + 2] = ((data[pos + 2] % 16) << 6) | (data[pos + 3] >> 2);
                imageLineBuffer[i + 3] = ((data[pos + 3] % 4) << 8) | data[pos + 4];
                pos += 5;
            }

            for (int i = 0; i < WIDTH; i++)
            {
                imageBuffer1[((counter * 2 + 0) * WIDTH) + i] = imageLineBuffer[16 + word_cnt * 0 + i] << 6;
                imageBuffer1[((counter * 2 + 1) * WIDTH) + i] = imageLineBuffer[16 + word_cnt * 1 + i] << 6;
                imageBuffer2[((counter * 2 + 0) * WIDTH) + i] = imageLineBuffer[16 + word_cnt * 2 + i] << 6;
                imageBuffer2[((counter * 2 + 1) * WIDTH) + i] = imageLineBuffer[16 + word_cnt * 2 + i] << 6; // The last channel in there is only repeated once... Go figure.
            }

            goodLines[counter * 2 + 0] = true;
            goodLines[counter * 2 + 1] = true;
        }

        cimg_library::CImg<unsigned short> InfraredReader2::getImage1()
        {
            // Fill missing lines by averaging above and below line
            for (int y = 1; y < HEIGHT - 2; y++)
            {
                bool &current = goodLines[y];
                if (!current)
                {
                    for (int i = 0; i < WIDTH; i++)
                    {
                        unsigned short &above = imageBuffer1[((y - 1) * WIDTH) + i];
                        unsigned short &below = imageBuffer1[((y + 2) * WIDTH) + i];
                        imageBuffer1[(y * WIDTH) + i] = (above + below) / 2;
                    }
                }
            }

            return cimg_library::CImg<unsigned short>(&imageBuffer1[0], WIDTH, HEIGHT);
        }

        cimg_library::CImg<unsigned short> InfraredReader2::getImage2()
        {
            // Fill missing lines by averaging above and below line
            for (int y = 1; y < HEIGHT - 2; y++)
            {
                bool &current = goodLines[y];
                if (!current)
                {
                    for (int i = 0; i < WIDTH; i++)
                    {
                        unsigned short &above = imageBuffer2[((y - 1) * WIDTH) + i];
                        unsigned short &below = imageBuffer2[((y + 2) * WIDTH) + i];
                        imageBuffer2[(y * WIDTH) + i] = (above + below) / 2;
                    }
                }
            }

            return cimg_library::CImg<unsigned short>(&imageBuffer2[0], WIDTH, HEIGHT);
        }
    }
}