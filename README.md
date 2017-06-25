# JJAT 2

Second iteration of JJAT's engine, now based on GFraMe v?.

**NOTE:** This game uses GFraMe's latest version, which still hasn't been merged
back into its master. To compile this manually, clone
[GFraMe's devel](https://github.com/SirGFM/GFraMe/tree/devel) branch.

## Player Interaction Reasoning

### Backstory

* The game starts on the lab where Swordy and Gunny were created(?)
* Other creatures were created on same lab (for which reason?)
* There should be biological creatures originated elsewhere

### Swordy Interactions

* Swordy can hit any biological creature
* Swordy can reflect **energy** attacks
* However, **physical** attacks can't be reflected
* Swordy should be able disable (and then hit) some enemies with armor

### Gunny Interactions

* Gunny can target any creature from the lab (how to differentiate?)
* Gunny can target any enemy with armor
* Gunny can only target constructed tiles (e.g., within the lab), but not broken
  ones

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

To remap the inputs, each key identifier must be followed by a colon and every
key value (as defined on [gfmInput.h](https://github.com/SirGFM/GFraMe/blob/master/include/GFraMe/gfmInput.h))
as 2 digits hexadecimal values (in Big-Endian). Good luck doing that
manually... Eventually, there will be a menu for automatically doing that.

Note that mapping for gamepads takes an extra digit, which references the
controller port. Ports are assigned sequentially (starting from zero) as soon as
the framework detects that a new controller was connected.

**Important note:** No two actions may have intersecting keys unless they are
mapped exactly to the same keys.

## Example mappings

If the default binding isn't to your liking, you may use one of the following:

**TODO:** Replace ${GAME_BIN} by the final binary's name.

### Synchronous mode, single player

This mapping uses either the first bound gamepad or the keyboard.

**Launch parameters:**

```sh
./${GAME_BIN} -k "SL:6005200b01;SR:6105300d;SJ:5a00214;SA:5b00c;GL:64047;GR:65048;GJ:5d049;GA:5e04a;P:68042;SW:6904344;"
```

**Keyboard:**

* Swordy: WASD/ZQSD
* Gunny: Arrow keys

**Gamepad:**

* Swordy: Left stick and shoulder buttons
* Gunny: Right stick and shoulder buttons

| Action | Bound input |
| ------ | ----------- |
| Move Swordy Left | Gamepad's left stick </br> Gamepad's d-pad </br> 'A' key </br> 'Q' key |
| Move Swordy Right | Gamepad's left stick </br> Gamepad's d-pad </br> 'D' key |
| Swordy's Jump | Gamepad's left shoulder button </br> 'W' key </br> 'Z' key |
| Swordy's Attack | Gamepad's left shoulder trigger </br> 'S' key |
| Move Gunny Left | Gamepad's right stick </br> 'Left arrow' key |
| Move Gunny Right | Gamepad's right stick </br> 'Right arrow' key |
| Gunny's Jump | Gamepad's right shoulder button </br> 'Up arrow' key |
| Gunny's Attack | Gamepad's right shoulder trigger </br> 'Down arrow' key |
| Pause | Gamepad's start button </br> 'ESC' key |
| Switch active character | Gamepad's select/back button </br> 'TAB' key </br> 'Spacebar' |

### Synchronous mode, two-players

This mapping uses the first bound gamepad for Swordy and the second bound
gamepad or the keyboard for Gunny.

**Launch parameters:**

```sh
./${GAME_BIN} -k "SL:600520;SR:610530;SJ:560;SA:570580;GL:60152147;GR:61153148;GJ:56115;GA:57158116;P:68068142;SW:5905914344;"
```

**Keyboard:**

* Gunny: Arrow keys, 'X' key, 'C' key

**Gamepad 1:**

* Swordy: Left stick, d-pad, 'A' button, 'X' button and 'B' button

**Gamepad 2:**

* Gunny: Left stick, d-pad, 'A' button, 'X' button and 'B' button

| Action | Bound input |
| ------ | ----------- |
| Move Swordy Left | First gamepad's left stick </br> First gamepad's d-pad |
| Move Swordy Right | First gamepad's left stick </br> First gamepad's d-pad |
| Swordy's Jump | First gamepad's 'A' button |
| Swordy's Attack | First gamepad's 'X' button </br> First gamepad's 'B' button |
| Move Gunny Left | Second gamepad's left stick </br> Second gamepad's d-pad </br> 'Left arrow' key |
| Move Gunny Right |  Second gamepad's left stick </br> Second gamepad's d-pad </br> 'Right arrow' key |
| Gunny's Jump |  Second gamepad's 'A' button </br> 'X' key |
| Gunny's Attack |  Second gamepad's 'X' button </br> Second gamepad's 'B' button </br> 'C' key |
| Pause | Gamepad's start button </br> 'ESC' key |
| Switch active character | Gamepad's 'Y' button </br> 'TAB' key </br> 'Spacebar' |

### Asynchronous mode, single player

This mapping uses either the first bound gamepad or the keyboard.

**Launch parameters:**

```sh
./${GAME_BIN} -c -k "SL:60052047;SR:61053048;SJ:56015;SA:57058016;P:68042;SW:5904344;"
```

**Keyboard:**

* Active character: Arrow keys, 'X' key, 'C' key, 'Tab' key, 'Spacebar'

**Gamepad:**

* Active character: Left stick, d-pad, 'A' button, 'X' button, 'B' button, 'Y' button

| Action | Bound input |
| ------ | ----------- |
| Move Active character Left | Gamepad's left stick </br> Gamepad's d-pad </br> 'Left arrow' key |
| Move Active character Right | Gamepad's left stick </br> Gamepad's d-pad </br> 'Right arrow' key |
| Active character's Jump | Gamepad's 'A' button </br> 'X' key |
| Active character's Attack | Gamepad's 'X' button </br> Gamepad's 'B' button </br> 'C' key |
| Pause | Gamepad's start button </br> 'ESC' key |
| Switch active character | Gamepad's 'Y' button </br> 'TAB' key </br> 'Spacebar' |

