#!/bin/bash

# Generate test files of various sizes
python3 -c "print('A' * 200)" > small.txt
python3 -c "print('B' * 2000)" > big.txt
python3 -c "print('C' * 10000)" > huge.txt
python3 -c "print('D' * 512)" > exact.txt
python3 -c "print('E' * 511)" > justunder.txt
python3 -c "print('F' * 513)" > justover.txt

cp blankfloppy.img robust.img

# Root level files
mcopy -i robust.img small.txt ::SMALL.TXT
mcopy -i robust.img big.txt ::BIG.TXT
mcopy -i robust.img huge.txt ::HUGE.TXT
mcopy -i robust.img exact.txt ::EXACT.TXT
mcopy -i robust.img justunder.txt ::UNDER.TXT
mcopy -i robust.img justover.txt ::OVER.TXT

# Root level deleted files
mcopy -i robust.img small.txt ::DELS.TXT
mcopy -i robust.img big.txt ::DELB.TXT
mcopy -i robust.img huge.txt ::DELH.TXT
mdel -i robust.img ::DELS.TXT
mdel -i robust.img ::DELB.TXT
mdel -i robust.img ::DELH.TXT

# Simple subdirectory
mmd -i robust.img ::SUBDIR
mcopy -i robust.img small.txt ::SUBDIR/SMALL.TXT
mcopy -i robust.img big.txt ::SUBDIR/BIG.TXT
mcopy -i robust.img small.txt ::SUBDIR/DELF.TXT
mdel -i robust.img ::SUBDIR/DELF.TXT

# Directory with extension
mmd -i robust.img ::DIR.EXT
mcopy -i robust.img small.txt ::DIR.EXT/FILE.TXT
mcopy -i robust.img small.txt ::DIR.EXT/DELF.TXT
mdel -i robust.img ::DIR.EXT/DELF.TXT

# Deeply nested directories
mmd -i robust.img ::A
mmd -i robust.img ::A/B
mmd -i robust.img ::A/B/C
mmd -i robust.img ::A/B/C/D
mmd -i robust.img ::A/B/C/D/E
mcopy -i robust.img small.txt ::A/B/C/D/E/DEEP.TXT
mcopy -i robust.img big.txt ::A/B/C/D/E/DEEPB.TXT
mcopy -i robust.img small.txt ::A/B/C/D/E/DELD.TXT
mdel -i robust.img ::A/B/C/D/E/DELD.TXT

# Deleted subdirectory
mmd -i robust.img ::DELDIR
mcopy -i robust.img small.txt ::DELDIR/FILE.TXT
mdel -i robust.img ::DELDIR/FILE.TXT
mrd -i robust.img ::DELDIR

# Empty subdirectory
mmd -i robust.img ::EMPTY

# File with no extension
mcopy -i robust.img small.txt ::NOEXT

mv robust.img images/