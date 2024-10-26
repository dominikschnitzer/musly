#ifndef MUSLY_DECODERS_NONE_H_
#define MUSLY_DECODERS_NONE_H_

#include "decoder.h"

namespace musly { namespace decoders {

class none : public musly::decoder
{
    MUSLY_DECODER_REGCLASS(none);

public:
    none();

    virtual std::vector<float>
    decodeto_22050hz_mono_float(const std::string& file, float excerpt_length, float excerpt_start);
};

}} // namespace musly::decoders

#endif /* MUSLY_DECODERS_NONE_H_  */
