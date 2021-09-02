#ifndef AUDIOSTREAM_H
#define AUDIOSTREAM_H

#include <QObject>
#include "globals.h"

namespace graceful
{
class AudioStreamPrivate;
class AudioStream : public QObject
{
    Q_OBJECT
public:
    static AudioStream* getInstance();

Q_SIGNALS:

private:
    explicit AudioStream(QObject *parent = nullptr);
    ~AudioStream();

private:
    static AudioStream*                 gInstance;
    AudioStreamPrivate*                 d_ptr = NULL;

    Q_DISABLE_COPY(AudioStream)
    Q_DECLARE_PRIVATE(AudioStream)
};

}

#endif // AUDIOSTREAM_H
