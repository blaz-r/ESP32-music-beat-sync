# ESP32-music-beat-sync
A small experiment using esp32, max9814 microphone, neopixels and arduinoFFT to sync ledstrip with music.

## About project

Entire project was written for ESP32, but I think it should work on any other arduino board with some if any tweaks.
I'm using max9814 microphone (sound sensor if you will). I'm also using RGBW ledstrip, but it should work with any neopixel with some tweaks.
For processing the sound I'm using arduinoFFT library https://github.com/kosme/arduinoFFT . I didn't really plan on making this public but since I didn't find much good info that I'd wanted I'm sharing this code for some help. It's mostly just an example and as description says, an experiment.

Beat detection is quite good in most cases, but not always perfect. That's mostly because I'm no professional when it comes to sound processing. Nonetheless it works quite well with most songs. As of now it's just a white light kind of strobe, so it's obvious when it's not working. If you change it to work by by colors and not just strobe, the small misses probably won't be as noticable.

The limits for detection are self adjsusting, which is not done in special or complicated way, just some basic calcualtions, so it works if you reduce the volume, increase it, and it adjusts to different songs. If the change is really big, it takes a bit for it to start working as expected.

## Important

After a lot of experimenting, I've discovered that for this to work better you'll need to disable AGC (automatic gain control) on microphone. Before I did that, magnitudes and freuqencies were fluctuating way more than they do with AGC disabled. In order to do that you just short two pins, as stated in [official documentation] (https://datasheets.maximintegrated.com/en/ds/MAX9814.pdf), TH and MICBIAS pin. Here's a picture of where to short on usual board:

![short guide](
https://github.com/blaz-r/ESP32-music-beat-sync/blob/main/max9814Short.jpeg)

Now you might not want to do this, but in my case, this really made the sync like it should be. It works almsot flawlessly, there's hardly any missed beats.

## Some details

The cirucit itself is simple, just connect ledstrip to microcontroller as usual, same with max9814 mic. You'll only really need to connect power and output from mic, since the limits are self adjsuting it should work without any real gain control, in my case AGC made it worse.

For FFT, 256 samples seem to be working best for me, but you could try increasing, just mind that FFT algo needs that number to be power of 2. Regarding samplingFrequency; I played around a bit, and it doesn't have much effect as long as it's big enough. You could also try playing around with windowing, I did some experimenting and blackman works quite well, but I got best results without any windowing at all, since the calcualtion is also faster.

Music is analyzed every 300 ms, so in should work for music up to 200BPM, one reason why it's not just analyzing all the time is that FFT isn't exactly super fast. You can miss a beat if you keep calcualting every loop and just happen to spend that time of peak calcualting not capturing sound. Limiting interval of analyzing really improved beat detection.

The math behind setting limits is all just empirical, it was just a quick attempt setting up some percentages and tweaking them until it was working quite well. You could of course try to change those settings, but current ones are working really well for my setup. You can simply change percentages by changing defines at the beginning of the code.

As stated before it's mostly an experiment, so the code isn't too clean and all, but I hope it's helpful :). If you have any advice, I'd be welcome to hear it.