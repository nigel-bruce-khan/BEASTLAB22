# Assignment 4 Report
Group: 104

#### Part I: Vector Triad

**1)** The `#pragma omp target` directive defines the target region in lines 31-37. When a host thread (thread in CPU) encounters the **target** construct, the target region is executed by a new thread running on the target device, which in our case is the GPU. The function `omp_is_initial_device()` returns true (1) if the current task is executing on the _host_ device; otherwise it returns _false_ (0). `targetDeviceId` is initialized to 1 in the CPU, but when this block is called and print its value in the terminal, we observe that it changed to 0, meaning that this region was executed in another device. 


**2)** 
