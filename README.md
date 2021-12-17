# Speculative-execution-attacks

### Starting off

Starting off, my task is to reproduce the Meltdown attack.

I'll use the seedlabs metltdown lab.

##### Task 1: Reading from Cache versus from Memory

On average cache times fall under are between 50 and 200 CPU cycles.

I defined the threshold as 60% of the average times. If the access time is
lower than that, it is most likely a cache hit.

When the difference is the biggest, usually the cache time is below 100 cycles

##### Task 2: Using Cache as a Side Channel

Made a script `./test_flush_reload.sh`. For mutiple batches of  20 runs I got
anywhere from 2 to 20 correct guesses.

##### Task 3: Place Secret Data in Kernel Space




### Resource

[Meltdown Paper](https://meltdownattack.com/meltdown.pdf) (1)

[POC gihub](https://github.com/IAIK/meltdown) (2)
[main site](https://meltdownattack.com/) (3)
[Meltdown Attack Lab](https://seedsecuritylabs.org/Labs_20.04/System/Meltdown_Attack/) (4)
[vulnerable machine](https://seed.nyc3.cdn.digitaloceanspaces.com/SEEDUbuntu-16.04-32bit.zip) (5)

[Spectre Paper](https://spectreattack.com/spectre.pdf)
