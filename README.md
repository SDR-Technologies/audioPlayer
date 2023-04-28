# audioPlugin


## Introduction
This plugin adds a way to send audio frames to PulseAudio

Example code:

``` javascript
var ok = loadPluginLib('/opt/vmbase/extensions/', 'libaudioPlugin');
if( ok == false ) {
    print('cannot load lib, stop.');
    exit();
}


var ok = ImportObject('libaudioPlugin', 'AudioPlayer');
if( ok == false ) {
    print('Cannot load object');
    exit();
}

var player = new AudioPlayer();
player.configure('test', 44100) ;
if( player.start() == false ) {
    print('Could not start AudioPlayer');
    exit();
}


var IQ = DSP.tone( 440, 88000, 44100);
for( var i=0 ; i < 10 ; i++ ) {
    player.play(IQ);
    print(i);
}
print('now waiting for queue to flush...');
while( player.getQueueSize() > 0 ) {
    sleep(200);
}

player.stop();
```

This sends 10 blocks of 88000 samples, waits for the sound queue to be empty and then ends.

## Compilation

This lib depends on the following libs :

* *pulse-simple*
* libsamplerate0*

to install them :  
``` bash
sudo apt-get install libsamplerate0-dev libpulse-dev
``` 

Compilation can be made with QMake or CMake

**Compilation with QMake**

``` bash
qmake
make
```

The resulting shared library (libaudioPlugin) is copied into /opt/vmbase/extensions

**Compilation with CMake**

``` bash
mkdir build
cd build
cmake ../
make
make install
``
The resulting shared library (libaudioPlugin) is copied into /opt/vmbase/extensions
