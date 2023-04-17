# ToyGB

This is a gameboy emulator. Not the most practical one, but it works.

I did this because I wanted to explore a bit of electronics and low-level computer science, the Gameboy is a really nice, albeit quirky machine that’s perfect to dig into those subjects.

It was also a way to explore the new C++ coroutines in C++20 (so this requires a compiler that supports them, like GCC 10+). So if you need an example, there’s some boilerplate coroutine code in `include/util/component.hpp` and `src/util/component.cpp`

Currently, the emulator works really well on original Gameboy (DMG) games, and can run GBC games with some issues. I made it with some accent on emulation accuracy, it passes most blargg’s tests, plus some MooneyeGB ones.

I also intended this as some kind of "reference", not that I’m particularly competent here, but coroutines, despite their performance cost, make the code much, much more straightforward than most other emulators, and I tried to document as much as I could, so this may be a fine reference for other new emulator developers for some unintuitive behaviours. Note that there might be errors and inaccuracies, especially within GBC behaviour.
