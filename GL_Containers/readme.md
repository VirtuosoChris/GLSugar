## GLSugar Containers

Promoted from KBH app development to GLSugar library.  Used to have an STL-like interface and memory management for various sprite and particle managers.

These data structures were written on the assumption that the template parameter "type" for the container is just a plain old data struct.  No RAII / constructors / destructors / etc are handled here.

We currently have two containers available:

## GPUVector
A contiguous buffer of memory.  Similar performance benefits and hazards to an std::vector.  You can .reserve() a chunk of memory, .clear(), .push_back(), and read and write.

## GPUDeque
Contiguous buffers of memory, broken into "pages".  Can grow or shrink on either end.  The main benefit of this one is that you can avoid extremely expensive reallocations as a vector grows massive in your game loop
(eg extreme carnage causing a massive spawn of blood particles) or alternatively having to reserve a huge chunk of memory you won't always need.

## Work in progress:
- More operations on vector
- Persistent mapped interface
