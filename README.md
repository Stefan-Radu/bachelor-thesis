## Bachelor's Thesis - 2022
### Speculative-execution-attacks
###### @University of Bucharest

### Abstract

Modern computers are equiped with features such as _out-of-order
execution_ and _branch prediction_, which are used to reduce CPU idle
time and improve performance. _Meltdown_ and _Spectre_ are two cyber
attacks that exploit microarchitectural side-effects which appear as a result
of such optimization techniques being used. An attacker can read private
data of the vicim at arbitrary locations in memory, without exploiting any
software bug. _Intel_, _AMD_ and _ARM_ were forced to redesign
their CPUs in order to mitigate the risks posed by _Meltdown_ and
_Spectre_. Despite deployed mitigations, in the second half of 2022,
most computers in the world are vulnerable to variations of _Spectre_
attacks, billions of users being at risk. This class of attacks remains a
subject of great interest for researchers in the field of security. In this
work, the technicalities and implications of both attacks will be covered. In
the end, the results of my own experiements will be presented, as a proof of
concept for a _Spectre_ attack. This implementation is different from
the one in the research paper _spectre2019_ and allows an attacker to
read the whole address space of another process.

### Source code

[attacker](./spectre/cross_process_final/attack.c)  
[victim](./spectre/cross_process_final/victim.c)

### Presentation

[slides](./prezentare/Stefan_Radu%20-%20Atacuri%20Speculative.pdf)

### Final Document
