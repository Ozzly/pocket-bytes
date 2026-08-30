#!/bin/bash

GRIT="/opt/wonderful/thirdparty/blocksds/core/tools/grit/grit"

# Sprites
$GRIT byte.png -ftB -fh! -gTFF00FF -gt -gB8 -m!
$GRIT key.png -ftB -fh! -gTFF00FF -gt -gB8 -m!
$GRIT door.png -ftB -fh! -gTFF00FF -gt -gB8 -m!
$GRIT box.png -ftB -fh! -gTFF00FF -gt -gB8 -m!
$GRIT button.png -ftB -fh! -gTFF00FF -gt -gB8 -m!
$GRIT platform.png -ftB -fh! -gTFF00FF -gt -gB8 -m!
$GRIT return.png -ftB -fh! -gTFF00FF -gt -gB8 -m!
mv *.pal *.img ../nitrofiles/sprite

# Palettes for character sprite 
$GRIT byte-mauve.png -ftB -fh! -gTFF00FF -gt -gB8 -m!
$GRIT byte-saphire.png -ftB -fh! -gTFF00FF -gt -gB8 -m!
$GRIT byte-bone.png -ftB -fh! -gTFF00FF -gt -gB8 -m!
$GRIT byte-pink.png -ftB -fh! -gTFF00FF -gt -gB8 -m!
$GRIT byte-red.png -ftB -fh! -gTFF00FF -gt -gB8 -m!
$GRIT byte-grey.png -ftB -fh! -gTFF00FF -gt -gB8 -m!
$GRIT byte-yellow.png -ftB -fh! -gTFF00FF -gt -gB8 -m!
mv *.pal ../nitrofiles/sprite
rm *.img

$GRIT title-top.png -ftB -fh! -gTFF00FF -gt -gB8 -mR8 -mLs
$GRIT select-mode-singleplayer.png -ftB -fh! -gTFF00FF -gt -gB8 -mR8 -mLs
$GRIT select-mode-multiplayer.png -ftB -fh! -gTFF00FF -gt -gB8 -mR8 -mLs
$GRIT select-mode-options.png -ftB -fh! -gTFF00FF -gt -gB8 -mR8 -mLs
$GRIT player-color-select.png -ftB -fh! -gTFF00FF -gt -gB8 -mR8 -mLs
$GRIT host-client-select.png -ftB -fh! -gTFF00FF -gt -gB8 -mR8 -mLs
$GRIT host-list.png -ftB -fh! -gTFF00FF -gt -gB8 -mR8 -mLs
$GRIT client.png -ftB -fh! -gTFF00FF -gt -gB8 -mR8 -mLs
$GRIT level1.png -ftB -fh! -gTFF00FF -gt -gB8 -mR8 -mLs
$GRIT level2.png -ftB -fh! -gTFF00FF -gt -gB8 -mR8 -mLs
mv *.pal *.img *.map ../nitrofiles/bg

$GRIT level1_col.png -ftB -fh! -gt -gB8 -mRtp -mLf -p!
$GRIT level2_col.png -ftB -fh! -gt -gB8 -mRtp -mLf -p!
$GRIT title-top-col.png -ftB -fh! -gt -gB8 -mRtp -mLf -p!
mv *.img *.map ../nitrofiles/collision

$GRIT default.png -ftB -fh!-gTFF00FF -gt -gB8 -m!
mv *.pal *.img ../nitrofiles/font