#reference: https://qiita.com/yura/items/e5c6e9527215e0e0524a

import serial

from synthesizer import Player, Synthesizer, Waveform
import numpy as np
import scipy

PORT = 'COM5'




# WYGBRO
#freqs = [554.366, 587.33, 739.988, 659.256, 830.61, 932.328]
freqs = [554.366, 739.988, 587.33, 830.61, 659.256, 932.328]

synth = Synthesizer(
    osc1_waveform=Waveform.triangle, osc1_volume=1.0,
    use_osc2=False, osc2_waveform=Waveform.triangle, osc2_volume=0.3, osc2_freq_transpose=1.0,
)

player = Player()
player.open_stream()

print('start CuYu2')

ser = serial.Serial(PORT, 115200)
while True:
    line = ser.readline()
    print(line, end=' ')
    face_data = int(line.split()[1])
    #face_data = 26
    chord = []
    for i in range(6):
        if 1 & (face_data >> (5 - i)):
            chord.append(freqs[i])
    print(chord)
    if chord:
        wave = synth.generate_chord(chord, 0.01)
        # ローパスフィルタのパラメータ
        nyquist_freq = 44100 / 2.0
        cutoff_freq = 3000.0
        cutoff = cutoff_freq / nyquist_freq
        # ローパスフィルタを適用
        n, wn = scipy.signal.buttord(cutoff , cutoff / 0.7, 6.0, 40.0)
        b, a = scipy.signal.butter(n, wn)
        wave = scipy.signal.filtfilt(b, a, wave)
        
        # play audio
        player.play_wave(wave)

#ser.close()
