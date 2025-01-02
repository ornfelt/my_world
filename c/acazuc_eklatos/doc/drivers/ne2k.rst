====
ne2k
====


Specifications
==============

`DP8390D/NS32490D NIC Network Interface Controller <https://web.archive.org/web/20010612150713/http://www.national.com/ds/DP/DP8390D.pdf>`_

`RTL8029AS Realtek PCI Full-Duplex Ethernet Controller with built-in SRAM <https://realtek.info/pdf/rtl8029as.pdf>`_


Hardware details
================

The NIC is configured through I/O ports (section 5 of specifications).
It contains one unique block of memory of 32KiB (address from 16KiB to 48KiB) to store packets.
This buffer is described by two ringbuf (one for incoming packets, one for outgoin packets).

.. note::
    Packets inside the buffer are 256-bytes aligned

Transfer of packets payload is done by sequentially reading / writing through the DMA register.

Initialization
==============

The NIC is first reset by reading the RST register.
The MAC address is then read through the 6 PAR register (if no address is set in the NIC, the hardcoded 52:54:00:12:34:56 MAC is used).
RX buffer is then configured by writing ringbuf bounds to registers BBNRY, PSTART, PSTOP, RSAR.
Interrupts for packets reception / emission are then set and interrupt handler is then installed.

Interrupt handler
=================

The interrupt cause is read from register ISR

If PTX or TXE bits are set, a packet has been send by the NIC and more space is available in the TX ring. A signal is sent to the TX waitq to unblock threads waiting for TX availability.

If PRX or RXE bits are set, a packet has been received and the reception handler is called

Packet reception
================

The packet header is read though the DMA engine. It consists of 4 bytes:
 - 1 byte of status (rsr)
 - 1 byte of next packet address
 - 2 bytes of packet length

If the rsr contains an error bit, packet has not been received and no further processing is done

Otherwise, the packet payload is read through the DMA engine and is then sent to the network stack

Packet emission
===============

Packet is DMA into the NIC by setting register RSAR & RBCR and then writing the packet through DMA register.
TX ringbuf size & positions are then set through registers TBCVR & TPSR.
The NIC is then notified of the presence of a packet in the TX ringbuf by writing the TXP bit in the CR register.
