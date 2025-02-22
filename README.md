# Takshaka Patch

## Author

Song Naga

## Description

Takshaka Patch is a series of configurations for the Daisy Seed Patch Eurorack module used for testing the features of the [Takshaka Supersaw Oscillator Desktop Synthesizer](https://github.com/wendallsan/song-naga-takshaka) project.  

### Oscillators Configuration:

- Voltage Controlled Supersaw Oscillator with Voltage control over Frequency, Drift, Shift and SubOscillator Amount controls
- SubOscillator features switchable waveforms and octave levels
<img width="833" alt="image" src="https://github.com/user-attachments/assets/6efb5e30-19dc-417a-a202-b5c3ad5d5284" />

#### Daisy Patch Layout:
cv inputs: freq, fine, drift, shift

controls: 
- knobs: freq, fine, drift, shift
- encoder: sub level, sub wave, sub octave

  outputs:
  - audio output

### Filters Configuration:

- pre-filter Distortion with amount adjustment
- "Growl" Resonant Ladder Low Pass filter with CV controlled Frequency
- "Howl" Comb filter with CV controlled Delay Amount
- filters are in series and their order can be swapped
<img width="828" alt="image" src="https://github.com/user-attachments/assets/d09c0e1b-957e-4fc8-83a9-63a2e01139a5" />

#### Daisy Patch Layout:  

inputs:
-	audio input
-	cv: growl, howl, res, fdbk
-	gate: filter order

controls: 
-	knobs: growl, howl, res, fdbk
-	encoder: venom, filter order

outputs:
-	audio output

