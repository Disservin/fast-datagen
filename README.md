# A data generator for uci chess engines

## Usage

Compile the project using make and just run the executable. Write `stop` if you wish to end the generation.  
I am not sure if control c fucks up the line endings so be careful.

Statistics that are reported are ALWAYS taken from a single thread.

## Modifications

In main.cpp you can specify the limits, as well as the engines name.
In generation.cpp you can modify the format or the adjudication rules.

You can also specify a epd book in the config.

## Chess960

Currently it's not supported as I first need to support it in <https://github.com/Disservin/chess-library>

## Pro's and Con's

Pros:

- You dont have to do anything other than plugging your engine into this and recompiling.
- Cross OS support.
- epd book support
- Multithreaded

Cons:

- A builtin data generator into your engine is in my experience faster, unless I have a bottleneck
  somewhere in my code, please tell me if thats the case.

## Notes

If you specify 30 threads to use, it will spawn 30 engine1's and 30 engine2's, and they will take turns when playing.
