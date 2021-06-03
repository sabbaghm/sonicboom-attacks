# SonicBOOM Spectre Attacks
This repository holds all code used to check if SonicBOOM is susceptible to Spectre attacks. These attacks are implemented based on the [boom-attacks](https://github.com/riscv-boom/boom-attacks).

# Further Details
## BOOM Configuration
This is working with the version of SonicBOOM located at [this commit](https://github.com/riscv-boom/riscv-boom/commit/e252e797c21aa3bf860cb9d67c6009ef00c5916f). We tested the *Small* and *Medium* core configurations.

## Test platform
* Xilinx Vivado block designs for FPGA RISC-V SoC running Debian Linux distro from [this repository](https://github.com/eugene-tarassov/vivado-risc-v).
  * OS: Debian Linux 64-bit OS with the kernel v15.11.16
* Target FPGA: Kintex-7 XC7K325T-2FFG900C (on Genesys-2 board)
  * Core frequency = 100MHz  
* Small core machine
  * 1-wide, 3-issue, 16KB D$ (2 MSHRs), 16KB I$, 512KB L2$
* Medium core machine
  * 2-wide, 3-issue, 16KB D$ (2 MSHRs), 16KB I$, 512KB L2$

## Implemented Attacks
The following attacks are implemented in this repo.

* Spectre-v1 or Bounds Check Bypass [1]
    * condBranchMispred.c
* Spectre-v2 or Branch Target Injection [1]
    * indirBranchMispred.c
* Spectre-v5 or Return Stack Buffer Attack [2]
    * returnStackBuffer.c

# Building the tests
To build you need to run `make`

# Running the Tests
This builds linux binaries that run on the SonicBOOM machine specified above.

# References
[1] P. Kocher, D. Genkin, D. Gruss, W. Haas, M. Hamburg, M. Lipp, S. Mangard, T. Prescher, M. Schwarz, and Y. Yarom, “Spectre attacks: Exploiting speculative execution,” ArXiv e-prints, Jan. 2018

[2] E. M. Koruyeh, K. N. Khasawneh, C. Song, N. Abu-Ghazaleh, “Spectre Returns! Speculation Attacks using the Return Stack Buffer,” 12th USENIX Workshop on Offensive Technologies, 2018
