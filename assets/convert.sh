#!/bin/bash

GRIT="/opt/wonderful/thirdparty/blocksds/core/tools/grit/grit"

$GRIT byte.png -ftB -fh! -gTFF00FF -gt -gB8 -m!
$GRIT key.png -ftB -fh! -gTFF00FF -gt -gB8 -m!
$GRIT door.png -ftB -fh! -gTFF00FF -gt -gB8 -m!
$GRIT box.png -ftB -fh! -gTFF00FF -gt -gB8 -m!

mv *.pal *.img ../nitrofiles/sprite
$GRIT byte-mauve.png -ftB -fh! -gTFF00FF -gt -gB8 -m!
$GRIT byte-saphire.png -ftB -fh! -gTFF00FF -gt -gB8 -m!
$GRIT byte-bone.png -ftB -fh! -gTFF00FF -gt -gB8 -m!
mv *.pal ../nitrofiles/sprite
rm *.img

$GRIT level1.png -ftB -fh! -gTFF00FF -gt -gB8 -mR8 -mLs
$GRIT level2.png -ftB -fh! -gTFF00FF -gt -gB8 -mR8 -mLs
mv *.pal *.img *.map ../nitrofiles/bg

$GRIT level1_col.png -ftB -fh! -gt -gB8 -mRtp -mLf -p!
$GRIT level2_col.png -ftB -fh! -gt -gB8 -mRtp -mLf -p!
mv *.img *.map ../nitrofiles/collision
