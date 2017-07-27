### Stream Classifier

Approximated stream processing algorithms, such as Count-Min sketch, Space-Saving, etc., support numerous applications in databases, storage systems, networking, and other domains. Unfortunately, because of the unbalanced distribution in real data streams, existing algorithms cannot achieve small memory usage, fast processing speed, and high accuracy at the same time. To address this gap, we propose a generic framework, called stream classifier, that enables faster and more accurate stream processing. Different from existing classifiers, our framework captures cold items in the first stage, and hot items in the second stage. Existing classifiers mainly focus on hot items, and often focus on one specific stream processing task. Also, existing classifiers require two-direction communication -- with frequent exchanges between the two stages being necessary; our framework on the other hand is one-direction -- each item enters one stage at most once. Our framework can accurately estimate both cold and hot items, giving it a genericity that makes it applicable to a wide variety of stream tasks. To illustrate the benefits of our framework, we deploy our framework on three typical stream tasks and extensive experimental results show speed improvements of up to 4.7 times, and accuracy improvements of up to 51 times.

This repo contains all the algorithms in our experiments, and a small demo to show how the usage.

The project is built upon cmake. You can use the following commands to build and run.

```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
cd ../
./demo
```

