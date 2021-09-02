#include "audio-stream.h"

using namespace graceful;

AudioStream* AudioStream::gInstance = NULL;

namespace graceful
{
class AudioStreamPrivate
{
public:
    explicit AudioStreamPrivate(AudioStream* d);


public:
    AudioStream* q_ptr = NULL;
};

AudioStreamPrivate::AudioStreamPrivate(AudioStream *d) : q_ptr(d)
{

}
}
// private end!


AudioStream *AudioStream::getInstance()
{
    if (gInstance) {
        gInstance = new AudioStream;
    }

    return gInstance;
}

AudioStream::AudioStream(QObject *parent) : QObject(parent),  d_ptr(new AudioStreamPrivate(this))
{


}

AudioStream::~AudioStream()
{
    delete d_ptr;
}
