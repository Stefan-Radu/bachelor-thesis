# Dangers of spectre attacks

### Spectre

Reproducing the Spectre-v1 attack Bounds Check Bypass (BCB).

##### First Steps

These steps are very similar with steps 1 & 2 from the meltdown attack.
i.e. understanding cache hits and reading the cpu-cache side-channel

##### Timing memory acess
``` c
#define CACHE_HIT_THRESHOLD (100)

static int scores[256];
void reloadSideChannelImproved() {
  int i;
  volatile uint8_t *addr;
  register uint64_t time1, time2;
  unsigned int junk = 0;
  for (i = 0; i < 256; i++) {
    addr = &array[i * 4096 + DELTA];
    time1 = __rdtscp(&junk);
    junk = *addr;
    time2 = __rdtscp(&junk) - time1;
    if (time2 <= CACHE_HIT_THRESHOLD && i != 3)
      // this is a cache hit
  } 
}
```
##### Training the CPU branch predictor

The core of the Spectre attack is a feature in modern cpus called `branch prediction` 
exploited in this block of code because of speculative execution.
``` c
unsigned int buffer_size = 10;
uint8_t buffer[10] = {0,1,2,3,4,5,6,7,8,9}; 
uint8_t temp = 0;
char *secret = "Some Secret Value";   

uint8_t restrictedAccess(size_t x)
{
  if (x < buffer_size) {
     return buffer[x];
  } else {
     return 0;
  } 
}

// Train the CPU to take the true branch inside victim().
for (i = 0; i < 100; i++) {
  _mm_clflush(&buffer_size);
  for (z = 0; z < 100; z++) { }
  restrictedAccess(i % 10);  
}
```

##### How does it work
``` c
#define CACHE_HIT_THRESHOLD (100)
#define DELTA 1024
#define PAGE_SIZE 4096

uint8_t array[256*4096];

// where the magic happens
s = restrictedAccess(larger_x);
array[s*PAGE_SIZE + DELTA] += 88;
```

Executing the original spectre experiment we can observe that we can access an out of bounds array
element by training the cpu branch prediction to take the true route. When we give it an input that
would take the false route, throgh speculative exectuion inside the cpu some code of the true branch
is executed as well, and we can observe this by looking through the CPU-Cache side-channel.

If I don't flush `size` from the cache it works less often, because `size` gets cached and there the
opperations run fast enough and the speculative execution does not start.

If I run with `i + 20` (an out of bounds index) in the training section, we will actually train 
the CPU branch predictor to take the `false` branch, which is the opposite of what we want.
The exeriment will not succeed in this particular case.

##### One run only

The attack works, but since the success rate is rather low, it is not reliable for actually reading
data. If I try to read all the secret, not just the first letter, the task is next to impossible,
since it misses most of the bytes of data.

##### Multiple runs

The side channel is not 100% reliable, so to get a usable output we need to run the attack multiple
times and average the results.

I had to lower the threshold and run a lot more times (500 - 1000 times) to get accurate resulte, but it works.

Obs. Running the Spectre attack I observed that it always printed 0. It makes sense because if the attack 
is successful there are actually two cache hits. One on our secret from speculative exectution, and
one from the actual execution, which returns 0 and hits on 0. If we ignore 0, the best result will be
on our secret.

### Checking my own PC

[meltdown-spectre-checker](https://github.com/speed47/spectre-meltdown-checker)
I used this tool and found out (02.02.2022) my machine is vulnerable to 
* CVE-2020-0543:KO (SRBDS) 

A lot of the vulnerabilities appear to be mittigated:

![mitigation](./assets/vulnerability.png)

The CPU on my machine is the `Intel(R) Core(TM) i5-8250U CPU @ 1.60GHz`
The mitigation are only software based since my hardware is vulnerable to the attacks.
##### Observation

On a later run I noticed the following:
* running the binary compiled in the VM on my machine worked
* compiling the code again and running it on my machine does not work anymore

This happens because the mitigations for *Spectre V1* are at the compiler level
[mitigation](https://gcc.gnu.org/onlinedocs/gcc-10.2.0/gcc/Other-Builtins.html#Other-Builtins). Thus the attack can be reproduced on a machine with up to date software, but vulnerable hardware by:
1. using an unpached compiler
2. using a binary compiled with an unpached compiler 
3. turning off the patch in the updated compiler

![proof](./assets/spectre_on_stef_xps.png)

### Conclusion

Even though Meltdown was mittigated Spectre remains an issue on older hardware.
The **only** complete mitigation for this class of attacks is updating the underlying hardware.

### Resource

[Meltdown Paper](https://meltdownattack.com/meltdown.pdf) (1)  
[POC gihub](https://github.com/IAIK/meltdown) (2)  
[main site](https://meltdownattack.com/) (3)  
[Meltdown Attack Lab](https://seedsecuritylabs.org/Labs_20.04/System/Meltdown_Attack/) (4)  
[vulnerable machine](https://seed.nyc3.cdn.digitaloceanspaces.com/SEEDUbuntu-16.04-32bit.zip) (5)  
[Spectre Paper](https://spectreattack.com/spectre.pdf)  (6)  
[check if vulnerable](https://www.cyberciti.biz/faq/check-linux-server-for-spectre-meltdown-vulnerability/) (7)  
