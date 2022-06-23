# ToyGB

This is a gameboy emulator. Not the most practical one, but it works.

I did this because I wanted to, but also as a way to explore the new C++ coroutines in C++20 (so this requires a compiler that supports them, like GCC 10+).

Currently, the emulator works quite well on original Gameboy (DMG) games, and can run GBC games with some issues. I made it with some accent on emulation accuracy, it passes most blargg’s tests, plus some MooneyeGB ones.

I also intended this as some kind of "reference", not that I’m particularly competent here but coroutines, despite their performance cost, make the code much, much more straightforward than most other emulators, and I tried to document as much as I could, so this may be a fine reference for other new emulator developers for some unintuitive behaviours.
