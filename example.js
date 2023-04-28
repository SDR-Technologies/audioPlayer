// Init - load the Audio out plugin
if(loadPluginLib('/opt/vmbase/extensions/', 'libaudioPlugin') == false ) {
    print('cannot load lib, stop.');
    exit();
}

// configure the AudioPlayer object from the plugin
if( ImportObject('libaudioPlugin', 'AudioPlayer') == false ) {
    print('Cannot load object');
    exit();
}

// Configure a player at 44100 Hz
var player = new AudioPlayer();
player.configure('test', 44100) ;
if( player.start() == false ) {
    print('Could not start AudioPlayer');
    exit();
}

// Generate a 440 Hz tone (88000 samples at 44100 Hz)
var IQ = DSP.tone( 440, 88000, 44100);
// Send the 440 Hz 10 times to the audio
for( var i=0 ; i < 10 ; i++ ) {
    player.play(IQ);
    print(i);
}
// Wait for the audio output to be empty (finished)
print('now waiting for queue to flush...');
while( player.getQueueSize() > 0 ) {
    sleep(200);
}

// Properly stop audio 
player.stop();
