/****************************************************************
 *                                                              *
 * @copyright  Copyright (c) 2020 SDR-Technologies SAS          *
 * @author     Sylvain AZARIAN - s.azarian@sdr-technologies.fr  *
 * @project    SDR Virtual Machine                              *
 *                                                              *
 * Example code for the SDRVM - feel free to reuse as you like  *
 *                                                              *
 ****************************************************************/
// This example is a simple FM VHF receiver
// it configures the SDR to stream 2 MHz of IQ centered at 144 MHz
// and creates a 12500 Hz wide channel centered at +390 kHz (APRS)
// and sends the IQ to a basic FM demodulaor
// and sends the audio to the Linux Mixer (see control panel)

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

// now create the audio player
var player = new AudioPlayer();
player.configure('FMAudio', 48e3) ;
if( player.start() == false ) {
    print('Could not start AudioPlayer');
    exit();
}

// Configure the radio
// We will use a RTLSDR and ask the RX to send samples 
// to an internal fifo named "input"
var rx = Soapy.makeDevice({'query' : 'driver=rtlsdr' }) ;
rx.setRxSampleRate( 2e6 ) ;
var fifo_from_rx = Queues.create( 'input');

// configure the receiver 
rx.setRxCenterFreq( 144 );
rx.setGain(40); // Gain value

// Create a NBFM demodulator
var fmdemod = new NBFM('');
fmdemod.configure( {'modulation_index': 0.25} );

// Create a single channel DDC
var slice = new DDC('one');
slice.setOutBandwidth(12500); // Will output 12500 kHz
slice.setDemodulator( fmdemod ); // use the demodulator
slice.setAGC(true);
slice.setCenter( 390e3 ) ; 
// engage streaming
if( !fifo_from_rx.ReadFromRx( rx ) ) {
	print('Cannot stream from rx');
	exit();
}

for( ; ; ) {
    var IQBlock = fifo_from_rx.dequeue(true);
    slice.write( IQBlock );
    var audio = slice.read();
    player.play(audio);
}