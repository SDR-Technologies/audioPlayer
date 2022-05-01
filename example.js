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
