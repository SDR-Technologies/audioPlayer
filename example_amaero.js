/****************************************************************
 *                                                              *
 * @copyright  Copyright (c) 2020 SDR-Technologies SAS          *
 * @author     Sylvain AZARIAN - s.azarian@sdr-technologies.fr  *
 * @project    SDR Virtual Machine                              *
 *                                                              *
 * Example code for the SDRVM - feel free to reuse as you like  *
 *                                                              *
 ****************************************************************/
// This example is a simple AM VHF receiver
// it is tuned by default on Paris Orly approach frequency
// http://www.pictaero.com/en/airports/airport,fr,lfpo
//
// it configures the SDR to stream 2 MHz of IQ centered at 123 MHz
// and creates a 12500 Hz wide channel centered at +875 kHz
// and sends the IQ to a basic AM demodulaor
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
rx.setRxCenterFreq( 123 );
rx.setGain(40); // Gain value

// Create a AM demodulator - see http://sdrvm.sdrtechnologies.fr/ampmodem/
var amdemod = new AMPModem('');
amdemod.configure( { 'type' : 'AM' }  );

// Create a single channel DDC
var slice = new DDC('one');
slice.setOutBandwidth(12500); // Will output 12500 Hz
slice.setDemodulator( amdemod ); // use the demodulator
slice.setAGC(true);
slice.setCenter( 875e3 ) ; // RX is 123 + 0.875

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