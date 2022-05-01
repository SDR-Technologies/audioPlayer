/****************************************************************
 *                                                              *
 * @copyright  Copyright (c) 2020 SDR-Technologies SAS          *
 * @author     Sylvain AZARIAN - s.azarian@sdr-technologies.fr  *
 * @project    SDR Virtual Machine                              *
 *                                                              *
 * Code propriete exclusive de la société SDR-Technologies SAS  *
 *                                                              *
 ****************************************************************/

#include <math.h>
#include <string.h>
#include <samplerate.h>
#include "vmtoolbox.h"
#include "audioplayer.h"
#include "json.hpp"


using json = nlohmann::json;

const char JSTypeNameStr[] = "AudioPlayer" ;



const char* AudioPlayer::Name() {
    return( (const char*) JSTypeNameStr );
}

const char* AudioPlayer::JSTypeName() {
    return( (const char*) JSTypeNameStr );
}

AudioPlayer* AudioPlayer::allocNewInstance(ISDRVirtualMachineEnv *host) {
    (void)host ;
    AudioPlayer* result = new AudioPlayer();
    result->init();
    return( result );
}

// This is called when the allocated instance is no longer required
void AudioPlayer::deleteInstance(IJSClass *instance) {
    if( instance == nullptr )
         return ;

    ((AudioPlayer *)instance)->stop();
    delete instance ;
}

void AudioPlayer::init() {
    memset( &params, 0, sizeof(AudioPlayerThreadParams));
    params.terminate = false ;
    params.running   = false ;
    params.audioRate = DEFAULT_AUDIO_SAMPLERATE ;
    params.queue = new AudioQueue(AUDIO_QUEUE_LENGTH);
}

bool AudioPlayer::configure(char *channel_name, int sample_rate) {
    if( channel_name != nullptr ) {
        snprintf( params.channelName, CHANNEL_NAME_LENGTH, "%s", channel_name );
    }
    if( sample_rate > 0 ) {
        params.audioRate = sample_rate ;
    }
    return( true );
}

bool AudioPlayer::isRunning() {
    if( runner == nullptr )
        return( false );
    return( params.running == true );
}

void AudioPlayer::stop() {
    if( isRunning() == false )
        return ;

    AudioFrame* af = (AudioFrame *)malloc( sizeof(AudioFrame));
    af->audioSamples = nullptr ;
    af->frameLength  = 0 ;
    params.queue->add( af );

    params.terminate = true ;
    if( runner->joinable() ) {
        runner->join();

    }
    delete runner ;
    runner = nullptr ;

    while( !params.queue->isEmpty() ) {
         params.queue->consume( af );
         if( af->audioSamples != nullptr )
             free(af->audioSamples);
         free(af);
    }
}

void audio_thread( AudioPlayerThreadParams *params ) ;
bool AudioPlayer::start() {
    if( (params.running == false) && (params.terminate==false) ) {
         runner = new std::thread( audio_thread, &params );
    }
    return( true );
}


void AudioPlayer::push(CpxBlock *b) {
    if( b == nullptr )
        return ;

    if( b->floatdata == false ) {
        fprintf( stderr, "AudioPlayer accepts FLOAT IQ!\n") ;
        fflush(stderr);
        free( b->data );
        free(b);
        return ;
    }
    if( b->length == 0 ) {
        if( b->data != nullptr )
            free(b->data);
        free(b);
    }

    if( (params.running == false) || (params.terminate==true) ) {
        fprintf( stderr, "start AudioPlayer first !\n") ;
        fflush(stderr);
        if( b->data != nullptr )
            free( b->data );
        free(b);
        return ;
    }


    AudioFrame* af = (AudioFrame *)malloc( sizeof(AudioFrame));
    af->audioSamples = (TYPECPX *)b->data ;
    af->frameLength  = b->length ;
    af->sampleRate = (int)b->samplerate ;
    free(b);
    params.queue->add( af );
}

int AudioPlayer::getQueueSize() {
    return( params.queue->length() );
}

int isrunning_call( void *stack ) ;
int stop_call( void* stack ) ;
int start_call( void* stack ) ;
int configure_call( void *stack );
int push_method( void *stack ) ;
int getQueueSize_method( void *stack );

void AudioPlayer::declareMethods( ISDRVirtualMachineEnv *host ) {
    host->addMethod( (const char *)"isRunning", isrunning_call, false);
    host->addMethod( (const char *)"start", start_call, true);
    host->addMethod( (const char *)"stop", stop_call, false);
    host->addMethod( (const char *)"configure", configure_call, true);
    host->addMethod( (const char *)"play", push_method, true);
    host->addMethod( (const char *)"getQueueSize", getQueueSize_method, true);
}

int isrunning_call( void *stack ) {
    AudioPlayer* p = (AudioPlayer *)vmtools->getObject(stack);
    if( p == nullptr ) {
        vmtools->pushBool( stack, false );
        return(1);
    }
    vmtools->pushBool( stack, p->isRunning() );
    return(1);
}

int stop_call( void* stack ) {
    AudioPlayer* p = (AudioPlayer *)vmtools->getObject(stack);
    if( p == nullptr ) {
        vmtools->pushBool( stack, false );
        return(1);
    }    
    p->stop();
    vmtools->pushBool( stack, true );
    return(1);
}

int getQueueSize_method( void *stack ) {
    AudioPlayer* p = (AudioPlayer *)vmtools->getObject(stack);
    if( p == nullptr ) {
        vmtools->pushInt( stack, -1);
    }  else {
        vmtools->pushInt( stack , p->getQueueSize() );
    }
    return(1);
}

int start_call( void* stack ) {
    AudioPlayer* p = (AudioPlayer *)vmtools->getObject(stack);
    if( p == nullptr ) {
        vmtools->pushBool( stack, false );
        return(1);
    }
    if( p->isRunning() ) {
        vmtools->pushBool( stack, false );
        return(1);
    }

    bool res = p->start();
    vmtools->pushBool( stack, res );
    return(1);
}

int configure_call(void *stack) {
    int n = vmtools->getStackSize( stack );
    AudioPlayer* p = (AudioPlayer *)vmtools->getObject(stack);
    if( p == nullptr ) {
        vmtools->pushBool( stack, false );
        return(1);
    }
    if( p->isRunning() ) {
        vmtools->pushBool( stack, false );
        return(1);
    }
    if( n != 2 ) {
        vmtools->throwException( stack, (char *)"Invalid call to configure. name and audio sample rate required here.");
        return(0);
    }

    const char *channel_name= vmtools->getString( stack, 0 );
    int sample_rate = vmtools->getInt( stack, 1);
    bool configured = p->configure( (char *)channel_name, sample_rate );
    vmtools->pushBool( stack, configured );
    return(1);
}

int push_method( void *stack ) {
    // Check that we have received the valid number of arguments
    int n = vmtools->getStackSize( stack );
    if( n < 1 ) {
        vmtools->throwException( stack, (char *)"Missing argument !");
        return(0);
    }
    // pop the argument
    CpxBlock *iq = vmtools->getIQData( stack, 0);
    // retrieve the "this" pointer for this method
    AudioPlayer* object = (AudioPlayer *)vmtools->getObject(stack);

    // call
    if( object != nullptr ) {
        object->push(iq);
    }
    return(0);
}


void audio_thread(AudioPlayerThreadParams *params ) {
    AudioFrame  *frame ;
    pa_simple *s;
    pa_sample_spec ss;
    pa_buffer_attr attr ;

    int pa_error_v ;

    params->running = true ;

    SRC_STATE* sstate = src_new(SRC_SINC_MEDIUM_QUALITY, 2, &pa_error_v );
    SRC_DATA sdata ;
    float *convert_buffer = nullptr ;
    int convert_buffer_length = 0 ;

    ss.format = PA_SAMPLE_FLOAT32LE;
    ss.channels = 2;
    ss.rate = params->audioRate ;
    attr.maxlength =  (uint32_t) -1;
    attr.tlength   =  (uint32_t) -1;
    attr.prebuf    = (uint32_t) -1 ;
    attr.minreq    = (uint32_t) -1 ;
    attr.fragsize    = (uint32_t) -1 ;
    s = pa_simple_new(NULL,               // Use the default server.
                      "SDRVM",           // Our application's name.
                      PA_STREAM_PLAYBACK,
                      NULL,               // Use the default device.
                      params->channelName,  // Description of our stream.
                      &ss,                // Our sample format.
                      NULL,               // Use default channel map
                      &attr,               // Use default buffering attributes.
                      NULL               // Ignore error code.
                      );

    float *audioSamples = nullptr ;
    int num_stereo_samples = 0 ;
    sdata.end_of_input = 0 ;

    while( !params->terminate ) {
        params->queue->consume(frame);
        if( frame->audioSamples == nullptr ) {
            free(frame);
            continue ;
        }

        audioSamples = (float *)frame->audioSamples ;
        num_stereo_samples = frame->frameLength ;

        if( frame->sampleRate != (int)params->audioRate ) {
            // convertion needed
            double ratio = (double)params->audioRate * 1.0/frame->sampleRate ;

            int out_size = (int)(num_stereo_samples * ratio ) + 1 ;
            if( out_size > convert_buffer_length ) {
                // need a bigger buffer
                if( convert_buffer != nullptr )
                    free(convert_buffer);
                convert_buffer_length = out_size * 2;
                convert_buffer = (float *)malloc( convert_buffer_length * 2 * sizeof(float)  );
                sdata.data_out = convert_buffer ;
                sdata.output_frames = convert_buffer_length ;
            }
            sdata.data_in = (const float *)frame->audioSamples ;
            sdata.input_frames = (long)num_stereo_samples  ;
            sdata.src_ratio = ratio ;
            pa_error_v = src_process( sstate, &sdata );
            if( pa_error_v != 0 ) {
                num_stereo_samples = 0 ;
            } else {
                if( sdata.output_frames_gen > 0 ) {
                    audioSamples = convert_buffer ;
                    num_stereo_samples = sdata.output_frames_gen ;
                } else {
                    num_stereo_samples = 0 ;
                }
            }
        }

        if( num_stereo_samples > 0 ) {
            if( pa_simple_write(s, (const void *)audioSamples, num_stereo_samples * 2 * sizeof(float), &pa_error_v) < 0 ) {
                fprintf( stdout, "%s() error : %d - %s\n", __PRETTY_FUNCTION__, pa_error_v, pa_strerror(pa_error_v));
                fflush( stdout );
            }
        }


        free(frame->audioSamples);
        free(frame);
    }
    pa_simple_drain(s,&pa_error_v) ;
    pa_simple_free(s);

    params->running = false ;
    params->terminate = false ;
}
