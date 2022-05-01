/****************************************************************
 *                                                              *
 * @copyright  Copyright (c) 2020 SDR-Technologies SAS          *
 * @author     Sylvain AZARIAN - s.azarian@sdr-technologies.fr  *
 * @project    SDR Virtual Machine                              *
 *                                                              *
 * Code propriete exclusive de la société SDR-Technologies SAS  *
 *                                                              *
 ****************************************************************/


#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H
#include <thread>
#include "vmplugins.h"
#include "vmtypes.h"
#include "ConsumerProducer.h"
#include <pulse/pulseaudio.h>
#include <pulse/simple.h>


typedef struct {
    TYPECPX *audioSamples ;
    int frameLength ;
    int sampleRate ;
} AudioFrame ;

typedef ConsumerProducerQueue<AudioFrame *> AudioQueue ;

#define CHANNEL_NAME_LENGTH (128)
#define AUDIO_QUEUE_LENGTH (3)
#define DEFAULT_AUDIO_SAMPLERATE (44100)

typedef struct  {
    bool terminate ;
    bool running ;
    uint32_t audioRate ;
    char channelName[CHANNEL_NAME_LENGTH] ;
    AudioQueue *queue ;
} AudioPlayerThreadParams ;

class AudioPlayer : public IJSClass
{
public:

    AudioPlayer() = default;

    const char* Name() ;
    const char* JSTypeName() ;
    AudioPlayer* allocNewInstance(ISDRVirtualMachineEnv *host) ;
    void deleteInstance( IJSClass *instance ) ;
    void declareMethods( ISDRVirtualMachineEnv *host ) ;

    void init();

    bool isRunning();
    void stop();
    bool start();
    bool configure( char *channel_name, int sample_rate );
    void push( CpxBlock *b );
    int getQueueSize();

private:
    AudioPlayerThreadParams params ;
    std::thread *runner ;
};

#endif // AUDIOPLAYER_H
