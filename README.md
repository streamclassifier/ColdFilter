## Cold Filter

### Introduction

Approximated stream processing algorithms, such as Count-Min sketch, Space-Saving, etc., support numerous applications in databases, storage systems, networking, and other domains. Unfor- tunately, because of the unbalanced distribution in real data streams, existing algorithms cannot achieve small memory usage, fast pro- cessing speed, and high accuracy at the same time. To address this gap, we propose a generic meta-framework, called Cold Filter (CF), that enables faster and more accurate stream processing.
Di erent from existing  lters, our  lter captures cold items in the  rst stage, and hot items in the second stage. Existing  lters mainly focus on hot items, and often focus on one speci c stream process- ing task. Also, existing  lters require two-direction communication – with frequent exchanges between the two stages being necessary; our  lter on the other hand is one-direction – each item enters one stage at most once. Our  lter can accurately estimate both cold and hot items, giving it a genericity that makes it applicable to a wide variety of stream tasks. To illustrate the bene ts of our  lter, we deploy it on three typical stream tasks and extensive experimental results show speed improvements of up to 4.7 times, and accuracy improvements of up to 51 times.

### About this repo

This repo contains all the algorithms in our experiments, as shown in the following table.

| Task                      | Algorithms                               |
| ------------------------- | ---------------------------------------- |
| Estimating item frequency | cm sketch (count min),  cm-cu sketch (count min sketch with conservative update), A sketch |
| Finding top-k hot items   | cm sketch with heap, cm-cu sketch with heap, space saving |
| Detecting heavy changes   | FlowRadar (invertible IBLT)              |

This repo also contains a small demo to show how to use this algorithms with a small dataset.

### Requirements

- The gather-and-report part of SC use SIMD instructions to achieve high speed, so the cpu must support SSE2 instruction set.
- cmake >= 2.6
- g++ (MSVC is not supported currently.)

### How to build

The project is built upon [cmake](https://cmake.org/). You can use the following commands to build and run.

```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
cd ../
./demo
```
