(Please excuse the general crudeness of this code, as well as the lack
of a proper build system; this project was conceived over six years ago
and remained largely unmodified for the last five years, mostly due to
the fact that I'm the only one using it, and it works pretty well for me.)

What is DSPR?
-------------

A DSP host supporting plugins that implement Winamp or VST APIs.

Properly configured it allows for transparent processing of all of the sound
coming through the user’s computer without requiring any further modification
of any actual applications emitting the audio.

If you ever wanted to pipe all of your audio through a parametric equalizer,
a reverb or an awesome spatial enhancer[1] then this is the droid you are
looking for.

[1] -- http://dallashodgson.info/articles/OpenAmbienceProject/

Installation
------------

To build everything you need `g++`, `wineg++` (usually part of `wine-dev`),
`mingw` and `ladspa-dev`, then you just need to simply use `make`, that is:

    $ make

If you don't need everything that gets built by default (or you don't have
all of the dependencies installed) you can just comment out the parts you
don't want in the makefile.

Your can grab the executables from the `build` directory. There is no
`make install`; you can just copy them to your `/usr/local/bin`.

How to use it
-------------

You can launch the server and load a VST DSP plugin:

    dspr-server-launcher vst --plugin-path <vst_plugin.dll> --load --editor

Or you can load a Winamp DSP plugin instead:

    dspr-server-launcher winamp --plugin-path <winamp_plugin.dll> --load

Or you can load both, in which case the sound will be piped through both:

    dspr-server-launcher vst --plugin-path <vst_plugin.dll> --load   winamp --plugin-path <winamp_plugin.dll> --load

The `dspr-server-launcher` will launch `dspr-server-wine.exe` with SCHED_RR
scheduling, so be sure to `chown root` and `chmod u+s` it.

The arguments are handled git-style, that is `mode --arg-for-mode`, except
multiple modes are accepted, which probably makes it a little confusing.

By default the plugins are not loaded; the `--load` argument forces them
to load. (Originally I envisioned a way to dynamically control loaded plugins
at runtime, hence they are not loaded by default; in the end I didn't need that
functionality.)

The `--editor` launches the settings editor (if any) for the VST plugin.

After launching the DSP server you also need to pipe your system's audio
through it. There are several ways to do that:

   * through JACK with `dspr-jack`,
   * through ALSA with ALSA's LADSPA host[2] using included LADSPA plugin (`dspr-ladspa.so`),
   * use it only with select applications supporting LADSPA plugins (e.g. mplayer)

I've found that the least glitchy way is to give JACK direct access to your
sound card and just pipe everything through it; also ALSA's JACK plugin is
pretty buggy, so it's just best to redirect ALSA to Pulseaudio. So, to reiterate
the setup I'm currently using looks like this:

    ALSA -> Pulseaudio -> JACK -> DSPR -> JACK -> The hardware

You can redirect ALSA to Pulseaudio by pasting this into your `~/.asoundrc`:

    pcm.!default {
        type pulse
        fallback "sysdefault"
    }

    ctl.!default {
        type pulse
        fallback "sysdefault"
    }

Then in your `/etc/pulse/default.pa` add:

    load-module module-jack-sink sink_name="jack_out"
    set-default-sink jack_out

And disable Pulsaudio's automatic device discovery:

    # load-module module-udev-detect
    # load-module module-detect

I've also disabled suspend-on-idle since I've found it to be buggy in the past:

    # load-module module-suspend-on-idle

There is probably a better way to prevent Pulseaudio from touching your soundcard
than to disable its autodetection module altogether; I don't particularly care for
it so it's not a problem for me.

Now you can finally launch everything:

    $ jackd -d alsa --rate 44100 -s --playback=<your device> -o 2 &
    $ dspr-jack &
    $ pulseaudio --disallow-exit --exit-idle-time=-1 &
    $ dspr-server-launcher <...> &

The only application that might *rarely* have problems with this setup is Wine;
everything else works perfect.

Also, for convenience I'd recommend running your DSP plugins under
`Xvfb` and access their configuration panels through a VNC client
of your choice with the help of `x11vnc`.

[2] -- http://alsa.opensrc.org/Ladspa_(plugin)
