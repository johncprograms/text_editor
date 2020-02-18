# text_editor
My personal text editor. It's a modal editor I wrote to avoid wrist problems, so it's not very user-friendly.
But it works for me, and hey, you might find some interesting code in there.

# credits
* Sean Barret for https://github.com/nothings/stb/blob/master/stb_truetype.h, which is included in the repo, or at least on old version of it.
* Myself, of course!

# logistics
It's Windows-only, AVX2-only. That's because all the machines I'm using meet that spec. 
It mainly uses AVX2 to have reasonably fast software rendering ( ~10 milliseconds end-to-end frame time, on a 4k display ).

It used to use OpenGL, but OpenGL on Windows requires at least half a second of boot delay; particularly in SetPixelFormat to set up the driver's GL context. Terrible.
Perhaps D2D/D3D would fare better, but I doubt it.
So we use our own software rendering ( ~7 cycles per pixel for texture transparency blending ), fed into good ol' GDI. It turns out that's fast enough for realtime animation of such simple data, so I'm sticking with that for now. The OGL crap is still in there; I'll let it sit since that transition was recent.

The build/ folder has the batch files to build debug/ship versions of the program.
It requires a VS2019 Community Edition install to build:
* cd build
* set_vs2019_env.bat
* build_ship.bat ( or build_debug.bat )

The program expects a .config file to live next to the executable, with the same name.
It contains all the keybinds, color options, font options, etc. The parser for it is terrible, but it's a nice dumping ground for machine-specific settings ( e.g. default spaces_per_tab=4 at work, 2 at home ). The build script will copy the master .config next to the built executable, so rename/move/copy it alongside the .exe.

There's some local-machine logging, so the .exe will create a /<exe_name>\_logs/ folder, containing some machine stats, and any instrumentation for that specific run of the program. This all just saved locally; there's no network calls in the program. See ```LOGGER_ENABLED``` for the switch to turn this on/off.

On crash, a full heap dump will get written to that logging directory alongside the .exe. This is so I can copy them from work machines back home, and debug on my own time.

There's also an instrumentation profiler, which dumps it's statistics to that log on program exit. See ```PROF_ENABLED``` for the switch to turn this on/off.

# programming curiosities
* ```CsFromFloat32``` and ```CsToFloat32``` are some half-done, infinite-precision implementations of ftoa/atof. Interesting for the sake of learning why those operations are so crazy slow.
* ```queue_mrmw_t``` and similar srmw, mrsw, srsw versions implement lock-free ringbuffers for multithreading. "mrsw" meaning "multiple-readers, single-writer", and similar for the other versions. In my opinion, using this kind of lock-free message-passing and SetEvent to wakeup threads is _the_ way to do multithreading.
* ```FontLoadGlyphImage``` does something similar to ClearType, with sharpening filters to try to make text glyphs look better.
* ```buf_t``` implements a list of variably-sized spans, which each point into a page in a shared linked-list of variably-sized backing pages. This is the workhorse of fast text editing operations. It gets constant-time ( no amortization ), excluding the O( count of string you're adding/replacing to ) cost, which every system pretty much has to pay for.
* ```content_ptr_t``` is the iterator for the ```buf_t```. It used to just be an absolute uint in \[0, filesize\], but the caching scheme I had to go from absolute uint index to span, was too slow. It turns out that you already need all these CursorMoveR/L style traversal functions anyways, so using a { span_index, offset_into_span } iterator instead of a uint isn't that bad. Well a little bad, because you have to pass all active iterators into the ```buf_t``` modification functions, since the span_index may change. I could make this simpler, but eh, it's not too bad. It's better than fixing up all the active iterators adhoc, post-change, as I used to do when they were just uints.
* ```TxtUpdateScrolling``` has some fun differential equation simulation, to implement smooth scrolling.
* ```fileopener_t``` implements reasonably fast search-for-file. This has a background thread just to un-stall the main thread for UI. I've previously tried doing all the application logic on a thread other than the main one, but the unpredictable latency to post messages across threads was too terrible. On Windows, it seems we're stuck with doing most things in the main thread, and splitting slow stuff out to background threads.
* ```findinfiles_t``` implements reasonably fast search-for-file-contents. This has some fun multithreading, and terrible scheduling at the moment. Yet it's still fast enough, on the order of VS2019's Find-In-Files. I used to memory-map files in this to make it as fast as possible, but I seemed to leak a file handle every 200K files or so. I think it was some Windows bug, due to kicking off dozens of threads to memory-map separate files at the same time. So for now, we read files in the standard ReadFile way, which is good enough for now.
* ```EditControlKeyboard``` is a reasonable example of how I do keybind -> command dispatch, in a medium-quality way.
* Search for ```LOGFILLRATE``` to see the fun AVX2 intrinsics implementing texture blits for the software rendering.
