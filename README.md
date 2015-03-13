# rnif2ki
Orcad to KiCAD netlist converter

Some years ago I needed to use KiCAD as a PCB design software but I wanted to use Orcad capture for schematics (in my company we used to do mostly schematics and outsourcing on PCB design). Some when it came time to design a small PCB, I looked for a OrCAD to KiCAD program......and I had to do it myself :)


This program is able to convert a RINF netlist (generated with OrCAD) to a KiCAD netlist. This command line tool has been written in C. Hereafter the source code and the compiled version of the program:

                                                                                           Download RINF2KI v2

The use is very easy. For exemple, if you want to convert myboard_rinf.net to myboard_kicad.net :

 rinf2ki mypcb_rinf.net mypcb_kicad.net
