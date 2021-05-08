# ESP32-music-beat-sync
A small experiment using esp32, max9814 microphone, neopixels and arduinoFFT to sync ledstrip with music.

## About project

Entire project was written for ESP32, but I think it should work on any other arduino board with some if any tweaks.
I'm using max9814 microphone (sound sensor if you will). I'm also using RGBW ledstrip, but it should work with any neopixel with some tweaks.
For processing the sound I'm using arduinoFFT library https://github.com/kosme/arduinoFFT . I didn't really plan on making this public but since I didn't find much good info that I'd wanted I'm sharing this code for some help. It's mostly just an example and as description says, an experiment.

Beat detection isn't as good as I'd want it to be, but that's mostly because I'm no professional when it comes to sound processing. Nonetheless it works quite well with most songs. As of now it's just a white light kind of strobe, so it's obvious when it's not working. If you change it to work by by colors and not just strobe, it should work quite well.

The limits for detection are self adjsusting, which is not done in special or complicated way, just some basic calcualtions, so it works if you reduce the volume, increase it, and it adjusts to different songs. Sometimes it takes a bit for it to start working as expected.

## Some details

The cirucit itself is simple, just connect ledstrip to microcontroler as usual, same with max9814 mic. You'll only really need to connect power and output from mic, since the limits are self adjsuting it should work without any real gain control so it's all right for it to float.

The math behind setting limits is all just empirical, it was just a quick attempt setting up some percentages and it seems to work good enough. You could of course try to change those settings and if you have any advice, I'd be welcome to hear it.
As stated before it's mostly an experiment so the code isn't too clean and all, but I hope it's helpful :).