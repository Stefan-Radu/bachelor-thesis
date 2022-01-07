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

Made a script `./test_flush_reload.sh`. For mutiple batches of 20 runs I got
on average 18 / 20 correct cache hits.

##### Task 3 & 4: Place Secret Data in Kernel Space & try to access it

After placing the secret in kernel space, I tried to access it. The result was
the **expected** Segmentation fault caused by the unhandled SIGSEV signal sent
when we accessed the prohibited memory location.

##### Task 5: Handle Error/Exceptions in C

Exception handling in C is weird, but can be done with signal handlers and `setjmp` & `siglongjmp`

##### Task 6: Out-of-Order Execution by CPU

If we run the code enough times we can see that we got a chach hit on `7`. That means the instructions following the illegal memory access were actually exectued.

##### Task 7: The Basic Meltdown Attack

###### 7.1
Some very rare cache hits but cannot extract any info from that info. most of them are just random.
Seems like the access time for the kernel_address is too long.

###### 7.2
After loading the kernel buffer in the cache the attack is successful. I'm wondering how can I make the attack successful without manually loading the buffer in cache.

![proof](./assets/meltdown_success.png)

### Resource

[Meltdown Paper](https://meltdownattack.com/meltdown.pdf) (1)  

[POC gihub](https://github.com/IAIK/meltdown) (2)  
[main site](https://meltdownattack.com/) (3)  
[Meltdown Attack Lab](https://seedsecuritylabs.org/Labs_20.04/System/Meltdown_Attack/) (4)  
[vulnerable machine](https://seed.nyc3.cdn.digitaloceanspaces.com/SEEDUbuntu-16.04-32bit.zip) (5)  

[Spectre Paper](https://spectreattack.com/spectre.pdf)  
