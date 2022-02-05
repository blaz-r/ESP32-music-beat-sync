# ESP32-music-beat-sync
A small experiment using esp32, max9814 microphone, neopixels and arduinoFFT to sync ledstrip with music.

## About project

Entire project was written for ESP32, but I think it should work on any other arduino board with some, if any, tweaks. Project is now structured to work with PlatformIO, but you can always use it in arduinoIDE by taking .cpp file and renaming it back to .ino.

I'm using max9814 microphone (sound sensor). I'm also using RGBW ledstrip, but it should work with any neopixel with some tweaks.
For processing the sound I'm using arduinoFFT library https://github.com/kosme/arduinoFFT . Mind that version from "develop" branch is used, since it's faster and uses less RAM!!! I didn't really plan on making this public but since I didn't find much good info that I'd wanted I'm sharing this code for some help. It's mostly just an example and as description says, an experiment.

Beat detection is OK in most cases, but not always perfect. That's mostly because I'm no professional when it comes to sound processing. Nonetheless it works quite well with most songs. As of now it's just a white light kind of strobe, so it's obvious when it's not working. If you change it to work by by colors and not just strobe, the small misses probably won't be as noticeable.

The limits for detection are self adjusting, which is not done in special or complicated way, just some basic averaging and checking magnitude, so it works if you reduce the volume, increase it, and it adjusts to different songs. If the change is really big, it takes a bit for it to start working as expected.

## Important

After a lot of experimenting, I've discovered that for this to work better you'll need to disable AGC (automatic gain control) on microphone. Before I did that, magnitudes and frequencies were fluctuating way more than they do with AGC disabled. In order to do that you just short two pins, as stated in [official documentation](https://datasheets.maximintegrated.com/en/ds/MAX9814.pdf), TH and MICBIAS pin. Here's a picture of where to short on usual board:

![short guide](
https://github.com/blaz-r/ESP32-music-beat-sync/blob/main/max9814Short.jpeg)

Now you might not want to do this, but in my case, this really made the sync like it should be. I'd say that it's first worth to try it without shorting, and then try shorting to see if it has any effect. In any case, you can later remove the short.

## Some details

The circuit itself is simple, just connect ledstrip to microcontroller as usual, same with max9814 mic. You'll only really need to connect power and output from mic, since the limits are self adjusting it should work without any real gain control, in my case AGC made it worse. But if the music is super loud, there might be some problems.

For FFT, 1024 samples seem to be working much much better than anything lower, but you could try experimenting on your own. Just mind that FFT algo needs that number to be power of 2. Regarding samplingFrequency; I played around a bit, and it doesn't have much effect as long as it's big enough. You could try increasing or decreasing it, but it should be about it's calculated value, that is 1 / ( sampling period ). You could also try playing around with windowing, I did some experimenting and blackman works quite well, but I got best results without any windowing at all, since the calculation is also faster.

The code is using ADC1 to read data, since it's a bit faster than analogRead (25kHz vs 16kHz sampling). It's good so you don't get frequency overlapping and this way you can analyse wider spectrum. In this example I'm using channel 4, which is on pin 32. You could change that, but check pinout and change channel according to pins where ADC1 channels are.

You can set the upper limit of music BPM. This is used for time between each beat so it doesn't analyse sound all the time. One reason why it's not just analyzing all the time is that FFT isn't exactly super fast. You can miss a beat if you keep calculating every loop and just happen to spend that time of peak calculating not capturing sound. Limiting interval of analyzing really improved beat detection.

The math behind setting limits is all just empirical, it was just a quick attempt setting up some percentages and limits, then tweaking them until it was working quite well. In latest version, beat is detected by comparing the current magnitude to moving average, this way beat detection is quite simple and more robust. You could of course try to change some parameters or adjust calculations, but current ones are working well for my setup. You can simply change them by changing defines at the beginning of the code.

As stated before it's mostly an experiment, so the code isn't too clean and all, but I hope it's helpful :smile:. If you have any advice, I'd be welcome to hear it.
