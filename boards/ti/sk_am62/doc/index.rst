.. zephyr:board:: sk_am62

Overview
********

The SK-AM62 board configuration is used by Zephyr applications that run on
the TI AM62x platform. The board configuration provides support for:

- ARM Cortex-M4F MCU core and the following features:

   - Nested Vector Interrupt Controller (NVIC)
   - System Tick System Clock (SYSTICK)

- ARM Cortex-A53 core and the following features:

   - General Interrupt Controller (GIC)
   - ARM Generic Timer (arch_timer)
   - On-chip SRAM (oc_sram)
   - UART interfaces (uart0 to uart6)
   - Mailbox interface (mbox0)

The board configuration also enables support for the semihosting debugging console.

See the `TI AM62X Product Page`_ for details.

Hardware
********
The SK-AM62 EVM features the AM62x SoC, which is composed of a quad Cortex-A53
cluster and a single Cortex-M4 core in the MCU domain. Zephyr is ported to run on
the M4F and A53 cores. The following listed hardware specifications are used:

- High-performance ARM Cortex-A53
- Low-power ARM Cortex-M4F
- Memory

   - 256KB of SRAM
   - 2GB of DDR4

- Debug

   - XDS110 based JTAG

Supported Features
==================

.. zephyr:board-supported-hw::

Devices
========
System Clock
------------

This board configuration uses a system clock frequency of 400 MHz.

DDR RAM
-------

The board has 2GB of DDR RAM available. This board configuration
allocates Zephyr 4kB of RAM (only for resource table: 0x9CC00000 to 0x9CC00400).

Serial Port
-----------

This board configuration uses a single serial communication channel with the
MCU domain UART (MCU_UART0).

SD Card
*******

Download TI's official `WIC`_ and flash the WIC file with an etching software
onto an SD-card. This will boot Linux on the A53 application cores of the EVM.
While programming for the M4 core, the A53 cores will then load the zephyr binary on the M4 core using remoteproc.

Programming for M4F Core
************************

The board can use remoteproc, and uses the OpenAMP resource table to accomplish this.

The testing requires the binary to be copied to the SD card to allow the A53 cores to load it while booting using remoteproc.

To test the M4F core, we build the :zephyr:code-sample:`hello_world` sample with the following command.

.. code-block:: console

   # From the root of the Zephyr repository
   west build -p -b sk_am62/am6234/m4 samples/hello_world

This builds the program and the binary is present in the :file:`build/zephyr` directory as
:file:`zephyr.elf`.

We now copy this binary onto the SD card in the :file:`/lib/firmware` directory and name it as
:file:`am62-mcu-m4f0_0-fw`.

.. code-block:: console

   # Mount the SD card at sdcard for example
   sudo mount /dev/sdX sdcard
   # copy the elf to the /lib/firmware directory
   sudo cp --remove-destination zephyr.elf sdcard/lib/firmware/am62-mcu-m4f0_0-fw

The SD card can now be used for booting. The binary will now be loaded onto the M4F core on boot.

To allow the board to boot using the SD card, set the boot pins to the SD Card boot mode. Refer to `EVM Setup Page`_.

After changing the boot mode, the board should go through the boot sequence on powering up.
The binary will run and print Hello world to the MCU_UART0 port.

Programming for A53 Core
************************

Copy the compiled ``zephyr.bin`` to the first FAT partition of the SD card and
plug the SD card into the board. Power it up and stop the u-boot execution at
prompt.

Use U-Boot to load and kick zephyr.bin:

.. code-block:: console

    fatload mmc 1:1 0x82000000 zephyr.bin; go 0x82000000

The Zephyr application should start running on the A53 core.

Debugging
*********

The board is equipped with an XDS110 JTAG debugger. To debug a binary, utilize the ``debug`` build target:

- M4F Core

.. zephyr-app-commands::
   :app: <my_app>
   :board: sk_am62/am6234/m4
   :maybe-skip-config:
   :goals: debug

- A53 Core

.. zephyr-app-commands::
   :app: <my_app>
   :board: sk_am62/am6234/a53
   :maybe-skip-config:
   :goals: debug

.. hint::
   To utilize this feature, you'll need OpenOCD version 0.12 or higher. Due to the possibility of
   older versions being available in package feeds, it's advisable to `build OpenOCD from source`_.

References
**********

AM62x SK EVM TRM:
   https://www.ti.com/lit/ug/spruiv7/spruiv7.pdf

.. _TI AM62X Product Page:
   https://www.ti.com/product/AM625

.. _WIC:
   https://dr-download.ti.com/software-development/software-development-kit-sdk/MD-PvdSyIiioq/10.01.10.04/tisdk-default-image-am62xx-evm-10.01.10.04.rootfs.wic.xz
.. _AM62x SK EVM TRM:
   https://www.ti.com/lit/ug/spruiv7/spruiv7.pdf

.. _EVM Setup Page:
   https://software-dl.ti.com/mcu-plus-sdk/esd/AM62X/08_06_00_18/exports/docs/api_guide_am62x/EVM_SETUP_PAGE.html

.. _build OpenOCD from source:
   https://docs.u-boot.org/en/latest/board/ti/k3.html#building-openocd-from-source
