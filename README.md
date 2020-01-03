# HDMI capture

## Intro

This is a simple example which illustrates how to capture HDMI video frames on a Realtek RTD1295 to disk using `v4l2`.

The output format is ABGR.

To list the supported formats, try the following commands:
```bash
v4l2-ctl --list-formats-ext
```
or if `ffmpeg` installed
```bash
ffmpeg -f v4l2 -list_formats all -i /dev/video0
```

## Usage

Both `Makefile` and `CMakeLists` are provided (but only the Makefile has been tested).

-   using `Makefile`
```bash
git clone https://github.com/patrickhwood/rtd1295-hdmi-capture
cd rtd1295-hdmi-capture
make
./capture	# defaults to 1920x1080
./capture 1280 720
```


## References

[fswebcam](https://github.com/fsphil/fswebcam)

[CAPTURING A WEBCAM STREAM USING V4L2](https://jwhsmith.net/2014/12/capturing-a-webcam-stream-using-v4l2/)
