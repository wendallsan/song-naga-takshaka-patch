#include "daisy_patch.h"
#include "daisysp.h"
#include "SmartKnob.h"
#include "BlockSuperSawOsc.h"
#include "BlockOscillator.h"
#include "BlockMoogLadder.h"
#include "BlockComb.h"

#define SAMPLE_RATE 48000.f
#define KNOBS_COUNT 4

using namespace daisy;
using namespace daisysp;

enum FilterModes { FILTER_MODE_LP, FILTER_MODE_COMB, FILTER_MODES_COUNT };
enum SubOscModes { SUBOSC_TRI_1, SUBOSC_TRI_2, SUBOSC_SQUARE_1, SUBOSC_SQUARE_2, SUBOSC_MODES_COUNT };
enum ControlPages { PAGE_1, PAGE_2, PAGE_3, PAGES_COUNT };
enum Page1Knobs { FILTER_FREQ, FILTER_RES, DRIFT, SHIFT, PAGE_KNOBS_COUNT };
enum Page2Knobs { COARSE_PITCH, FINE_PITCH, SUB_MIX, DRIVE };
enum Page3Knobs { WOBBLE, CUTOFF_CV_ATT, DRIFT_CV_ATT, SHIFT_CV_ATT };
enum WobbleModes { WOBBLE_1, WOBBLE_2, WOBBLE_MODES_COUNT };
enum PageEncoderSettings { FILTER_TYPE, SUBOSC_TYPE, WOBBLE_TYPE };

DaisyPatch hw;
BlockSuperSawOsc superSaw;
BlockOscillator subOsc;
BlockMoogLadder filter;
BlockComb combFilter;
Parameter knobs[ KNOBS_COUNT ];

const size_t sampleBlockSize = 16;
float combFilterBuffer[ 9600 ],
	knobValues[ KNOBS_COUNT ],
	oscFreq,
	filterFreq,
	driftValue,
	shiftValue,
	subOscMix = 0.3f,
	driveValue = 0.1f;
int currentSubOscMode = SUBOSC_TRI_1,
	currentWobbleMode = WOBBLE_1,
	currentFilterMode = FILTER_MODE_LP,
	currentPage = PAGE_1,
	lastPage1Setting,
	lastPage2Setting,
	lastPage3Setting,
	currentSubOscWaveForm = subOsc.WAVE_POLYBLEP_TRI,
	currentSubOscOctave = 1;

float mapFilterFreq( float inputFreq, float oscFreq, bool isComb ){
	return isComb? fmap( inputFreq, 0.01f, fclamp( oscFreq * 16.f, 0.f, 10000.f ), Mapping::EXP ) : 		
		fmap( inputFreq, 20.f, fclamp( oscFreq * 32.f, 0.f, 20000.f ), Mapping::EXP );
}
void filterSignal( float *buff, size_t size ){
	if( currentFilterMode == FILTER_MODE_COMB ) combFilter.Process( buff, size );
	else filter.Process( buff, size );
}
void AudioCallback( AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size ){
	hw.ProcessAnalogControls();
	// DECLARE OUTPUT BUFFER AND ZERO IT OUT
	float output[ size ]; for( size_t i = 0; i < size; i++ ) output[ i ] = 0.f;
	for ( size_t i = 0; i < size; i++ ){		
		float sampleBuffer[ size ],
			subSampleBuffer[ size ];		
		superSaw.Process( sampleBuffer, size );
		subOsc.Process( subSampleBuffer, size );
		// MIX TOGETHER OSC AND SUBOSC BUFFERS
		for( size_t i = 0; i < size; i++ )
			sampleBuffer[i] = sampleBuffer[i] * ( 1.f - subOscMix ) + 
				subSampleBuffer[i] * subOscMix;		
		for( size_t i = 0; i < size; i++ ) // DISTORT THE SIGNAL
			sampleBuffer[ i ] = SoftClip( sampleBuffer[ i ] * ( 1.f + fmap( driveValue, 0.f, 8.f ) ) );
		filterSignal( sampleBuffer, size ); // FILTER THE SIGNAL
		// OUTPUT THE SIGNAL
		for( size_t i = 0; i < size; i++ ){
			out[0][i] = SoftClip( sampleBuffer[i] );
			out[1][i] = 0.f;
			out[2][i] = 0.f;
			out[3][i] = 0.f;
		}
	}
}

int subOscWaveFormLookup( int currentSubOscMode ){
	switch( currentSubOscMode ){
		case SUBOSC_TRI_1:
		case SUBOSC_TRI_2:
			return subOsc.WAVE_POLYBLEP_TRI;
		case SUBOSC_SQUARE_1:
		case SUBOSC_SQUARE_2:
			return subOsc.WAVE_POLYBLEP_SQUARE;
	}
}

int subOscOctaveLookup( int currentSubOscMode ){
	switch( currentSubOscMode ){
		case SUBOSC_TRI_1:
		case SUBOSC_SQUARE_1:
			return 1;
		case SUBOSC_TRI_2:
		case SUBOSC_SQUARE_2:
			return 2;
	}
}

void initDsp(){
	superSaw.Init( SAMPLE_RATE );
	subOsc.Init( SAMPLE_RATE );
	subOsc.SetWaveform( subOsc.WAVE_POLYBLEP_TRI );
	filter.Init( SAMPLE_RATE );
	int combfilterBufferLength = sizeof( combFilterBuffer ) / sizeof( combFilterBuffer[ 0 ] );
	for( int j = 0; j < combfilterBufferLength; j++ ) combFilterBuffer[ j ] = 0.f;
	combFilter.Init( SAMPLE_RATE, combFilterBuffer, combfilterBufferLength );
}

void handleKnobs(){
	for( int i = 0; i < KNOBS_COUNT; i++ ) knobValues[ i ] = 1.0 - knobs[ i ].Process();
}

void initKnobs(){
	knobs[0].Init( hw.controls[ hw.CTRL_1 ],  0.f, 1.f, Parameter::LINEAR );
	knobs[1].Init( hw.controls[ hw.CTRL_2 ],  0.f, 1.f, Parameter::LINEAR );
	knobs[2].Init( hw.controls[ hw.CTRL_3 ],  0.f, 1.f, Parameter::LINEAR );
	knobs[3].Init( hw.controls[ hw.CTRL_4 ],  0.f, 1.f, Parameter::LINEAR );
}

void handleEncoder(){
	int increment = hw.encoder.Increment();
	if( increment == 1 ) pageUp();
	if ( increment == -1 ) pageDown();
    bool encoderPressed = hw.encoder.RisingEdge();
	if( encoderPressed ) changePageSetting();
}
void pageSettingChanged( int whichPage, int newSetting ){
	switch( whichPage ){
		case 0: 
			currentFilterMode = newSetting;
			break;
		case 1:
			currentSubOscMode = newSetting;
			currentSubOscWaveForm = subOscWaveFormLookup( currentSubOscMode );
			currentSubOscOctave = subOscOctaveLookup( currentSubOscMode );
			break;
		case 2:
			currentWobbleMode = newSetting;
			break;
	}
}
void changePageSetting(){
	int newPageSetting;
	switch ( currentPage ){
		case PAGE_1:
			newPageSetting = lastPage1Setting + 1;
			if( newPageSetting >= FILTER_MODES_COUNT ) newPageSetting = 0;
			pageSettingChanged( PAGE_1, newPageSetting );
			break;
		case PAGE_2:
			newPageSetting = lastPage2Setting + 1;
			if( newPageSetting >= SUBOSC_MODES_COUNT ) newPageSetting = 0;
			pageSettingChanged( PAGE_2, newPageSetting );
			break;
		case PAGE_3:
			newPageSetting = lastPage3Setting + 1;
			if( newPageSetting >= WOBBLE_MODES_COUNT ) newPageSetting = 0;
			pageSettingChanged( PAGE_3, newPageSetting );
			break;
	}
}
void updatePageKnobs(){

}
void pageUp(){
	int newCurrentPage = currentPage + 1;
	if( newCurrentPage > PAGES_COUNT ) newCurrentPage = PAGES_COUNT;
	if( newCurrentPage != currentPage ) updatePageKnobs();
	currentPage = newCurrentPage;
}
void pageDown(){
	int newCurrentPage = currentPage - 1;
	if( newCurrentPage < 0 ) newCurrentPage = 0;
	if( newCurrentPage != currentPage ) updatePageKnobs();
	currentPage = newCurrentPage;
}
int main( void ){
	hw.Init();
	hw.SetAudioBlockSize( sampleBlockSize ); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	hw.StartAdc();	
	hw.StartAudio( AudioCallback );
	initKnobs();
	initDsp();
	while( true ){		
		handleKnobs();
		handleEncoder();

		// todo: handle input CV's
		oscFreq = mtof( knobValues[0] );
		filterFreq = mtof( knobValues[1] );
		driftValue = knobValues[2];
		shiftValue = knobValues[3];
		superSaw.SetFreq( oscFreq );
		superSaw.SetDrift( fclamp( driftValue, 0.f, 0.999 ) );
		superSaw.SetShift( fclamp( shiftValue, 0.f, 0.999 ) );
		subOsc.SetFreq( oscFreq / currentSubOscOctave + 1 );
		filter.SetFreq( mapFilterFreq( filterFreq, oscFreq, false ) );		
		combFilter.SetFreq( mapFilterFreq( filterFreq, oscFreq, true ) );

		// TODO: SET FILTER MODE
		// currentFilterMode = filterTypeSmartKnob.GetValue() * FILTER_MODES_COUNT;	
		filter.SetRes( 0.f ); // TODO: SET RESONANCE
		combFilter.SetRevTime( 0.2f ); // TODO: SET COMB FILTER DECAY TIME	
		
		// modeSwitch.Debounce();
		// operationMode = !modeSwitch.Pressed();
		// IF THE OPERATING MODE CHANGED, CHANGE MODE ON THE SMART KNOBS
		// if( operationMode != lastOperationMode ) handleSmartKnobSwitching();
		// lastOperationMode = operationMode;
	}
}