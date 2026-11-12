# Changelog

## November 11, 2026

### New

- The HUD now starts on the touch screen, over a darkened jiggy-puzzle background, with the jiggy and Mumbo token totals lined up with the notes and eggs, and the honeycombs and air side by side. If you've already played, your settings keep the HUD where it was: switch it on the Game page.
- The touch-screen HUD takes less work to draw: only the part that changed is redrawn, its text and background are drawn by the GPU, and the in-between frames at 60 fps skip it.
- Screenshots are numbered and never overwrite each other, and the "saved" message clears itself after 10 seconds.
- The HOME prompt now quits with X instead of A, so a stray A press can't close the game.

### Fixed

- The Luma3DS Rosalina menu freezing the console when SELECT was pressed before L and Down.
- Long sounds are less likely to be cut off when many sounds play at once.

## November 2, 2026

### New

- HUD on the bottom screen, if you want it: the counters move to the touch screen and stay up while you play, so the top screen is all game. Turn it on with the HUD setting on the Game page.
- Spanish and German translations, picked with the Language setting on the Mods page.
- Sound works out of the box, the game's reverb included: no more dumping your DSP firmware.
- Remap any button, add up to two buttons on the touch screen, and set each screen's brightness.
- Audio setting: High priority keeps the music on time, Low priority trades a little of that for speed on an Old 3DS (see below), and Off turns the sound off.
- Quitting from the HOME button waits for the game to finish saving.
- The Luma3DS Rosalina menu now works while playing.
- The mods' buttons are always on the touch screen. To use the 3DS's own buttons for a mod instead, map its N64 buttons on the Controls page.
- The frame-rate readout on the bottom screen is off by default; Performance monitor on the Game page turns it on.

### Fixed

- The underwater tint and screen fades not reaching the sides of the wide screen.
- A ROM that fails the check at start is no longer deleted: the game says what's wrong and leaves the file alone.

### Old 3DS / 2DS

- Faster: 20-30 fps, up from 15-25.
- Faster loading with the CIA: it now holds the whole ROM in memory instead of reading it from the SD card.
- Smoother play: wireless and background services pause while the game runs; slight CPU allocation increase.
- Audio starts at Low priority, which gives about 5% more frames, but the music can run a little slow or crackle in busy scenes. Set Audio to High priority on the Game page to keep the music on time.
