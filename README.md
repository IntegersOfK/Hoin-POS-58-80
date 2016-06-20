# Hoin-POS-58-80
Drivers for the Hoin HOP-E801 point of sale thermal receipt printer with variable paper width 58 or 80 millimeters.

I bought the HOP-E801 thermal receipt printer from http://hoinprinter.com/ but the compiled drivers were built on a 32-bit system so the filter would fail to print with CUPS on my Raspberry Pi 3. The source was available on the little CD they sent with it and seems to be a fork of the ZJ-58 thermal printer drivers (that's probably fine because many thermal reciept printers share the same printing commands). Fork this repo to make your own paper sizes, or maybe you came here just to download the 64-bit version I re-compiled.
