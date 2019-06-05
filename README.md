Protocol Buffer messages and codec for YGOpro Core (ocgcore)
------------------------------------------------------------
This repository includes all the tools needed to read and parse messages from the game core API (the memory buffer you receive after calling `get_message`) and translate them to the Protobuf equivalent described thereby; All Protobuf messages are easy to understand, read and parse. Moreover, they are designed to remove redundancies, limitations and liability of handling raw memory.

The codec is the middle-man which translates said messages back and forth between the core: It performs both reading memory buffer from the core API and translate them to Protobuf messages (Encoder) and also take a Protobuf message with a response meant for the core and transforms it back to a memory buffer that the core itself can understand (Decoder).

Motivation
----------
The messages that the core use and pass around are very cumbersome, they dont have any description on what they do or what they are for, the individual sizes (in bytes) of several fields change for reasons unknown, some messages have several smaller parts OR'd together, and some other go as far as adding dirty bits to data meant for other purposes. In other words, they are complete mess to deal with, they are not scalable and they also have several other limitations/problems that are beyond this topic.

With a unified interface managed by the powerful Protobuf library, end-users of this library dont have to worry about lower level stuff such as allocating memory buffers, reading individual bytes and performing tedious loop reads, they also dont have to worry about messages changing in the core, as they will be deal with at the encoder/decoder level, and even in those cases, Protobuf is backward compatible, making changes to the messages will be as easy as adding a new field to the protobuf descriptions.

This library also has another purpose: Serve as a foundation for new technologies related to the ygopro project itself, for example, if someday the core is to be rewritten, these specifications can be reused and end-users would not need to change a single line of code on their side.

Compiling
---------
TODO
