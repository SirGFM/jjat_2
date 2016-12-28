# JJAT 2

Second iteration of JJAT's engine, now based on GFraMe v2.

## Remapping Input

Each in-game action is mapped to a virtual key. The table bellow shows how keys
should be referenced when remapping them through the command line:

| Action | Key Identifier |
| ------ | -------------- |
| Move Swordy Left | SL |
| Move Swordy Right | SR |
| Swordy's Jump | SJ |
| Swordy's Attack | SA |
| Move Gunny Left | GL |
| Move Gunny Right | GR |
| Gunny's Jump | GJ |
| Gunny's Attack | GA |
| Pause | P |
| Switch active character\* | SW |

\* Action only enable when asynchronous mode is active.

To remap the inputs, each key identifier must be folowed by a colon and every
key value (as defined on [gfmInput.h](https://github.com/SirGFM/GFraMe/blob/master/include/GFraMe/gfmInput.h))
as 2 digits hexadecimal values (in Big-Endian). Good luck doing that
manually... Eventually, there will be a menu for automatically doing that.

Note that mapping for gamepads takes an extra digit, which references the
controller port. Ports are assigned sequentially (starting from zero) as soon as
the framework detects that a new controller was connected.

**Important note:** No two actions may have intersecting keys unless they are
mapped exactly to the same keys.

