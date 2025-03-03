#include "utils.h"
#include <cmath>
#include <sstream>

void char_array_to_uchar(int8_t *in, uint8_t *out, int nsamples)
{
    for (int i = 0; i < nsamples; i++)
    {
        double r = in[i] * 127.0;
        if (r < 0)
            r = 0;
        else if (r > 255)
            r = 255;
        out[i] = floor(r);
    }
}

std::vector<std::string> splitString(std::string input, char del)
{
    std::stringstream stcStream(input);
    std::string seg;
    std::vector<std::string> segs;

    while (std::getline(stcStream, seg, del))
        segs.push_back(seg);

    return segs;
}