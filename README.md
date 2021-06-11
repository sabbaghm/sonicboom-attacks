# SonicBOOM Spectre Attacks
This repository holds all code used to check if SonicBOOM is susceptible to Spectre attacks. These attacks are implemented based on the [boom-attacks](https://github.com/riscv-boom/boom-attacks).

## BOOM Configuration
This is working with the version of SonicBOOM located at [this commit](https://github.com/riscv-boom/riscv-boom/commit/e252e797c21aa3bf860cb9d67c6009ef00c5916f). We tested *Small* and *Medium* core configurations.

## Test platform
* Xilinx Vivado block designs for FPGA RISC-V SoC running Debian Linux distro from [this repository](https://github.com/eugene-tarassov/vivado-risc-v).
  * OS: Debian Linux 64-bit OS with the kernel v15.11.16 with gcc v10.2.1
* Target FPGA: Kintex-7 XC7K325T-2FFG900C (on Genesys-2 board)
  * Core frequency = 100MHz  
* Small core machine
  * 1-wide, 3-issue, 16KB D$ (2 MSHRs), 16KB I$, 512KB L2$
* Medium core machine
  * 2-wide, 3-issue, 16KB D$ (2 MSHRs), 16KB I$, 512KB L2$

## Implemented Attacks
The following attacks are implemented in this repository:

* Spectre-v1 or Bounds Check Bypass [1]
    * condBranchMispred.c
* Spectre-v2 or Branch Target Injection [1]
    * indirBranchMispred.c
* Spectre-v5 or Return Stack Buffer Attack [2]
    * returnStackBuffer.c

# Building the tests
To build binaries and generate assembly codes you need to run `make all`. Note, `make bin` only creates binaries.

### Build for Verilator simulation
To build binaries and generate assembly codes for the Verilator simulation you need to run `make allrv`. Verilator elevates the CHISEL/Verilog codes to C++ to perform a "software" RTL simulation. We used Verilator v4.034 as part of the [*Chipyard*](https://chipyard.readthedocs.io/en/latest/Chipyard-Basics/Initial-Repo-Setup.html#initial-repository-setup) framework. Note, `make binrv` only creates binaries.

# Running the Tests
Running `make all` (`make allrv`) builds linux binaries that run on the SonicBOOM machine. Binaries are located in the `bin` (`binrv`) folder and the assembly programs are located in the `dump` (`dumprv`) folder.

# Citation
If you use SonicBOOM attacks in your published work, please cite it as:

```
@article{sabbaghsboomattacks,
  title={Secure Speculative Execution via RISC-V Open Hardware Design},
  author={Sabbagh, Majid and Fei, Yunsi and Kaeli, David},
  booktitle={Fifth Workshop on Computer Architecture Research with RISC-V},
  year={2021},
  month={June}
}
```

# References
[1] P. Kocher, D. Genkin, D. Gruss, W. Haas, M. Hamburg, M. Lipp, S. Mangard, T. Prescher, M. Schwarz, and Y. Yarom, “Spectre attacks: Exploiting speculative execution,” ArXiv e-prints, Jan. 2018

[2] E. M. Koruyeh, K. N. Khasawneh, C. Song, N. Abu-Ghazaleh, “Spectre Returns! Speculation Attacks using the Return Stack Buffer,” 12th USENIX Workshop on Offensive Technologies, 2018