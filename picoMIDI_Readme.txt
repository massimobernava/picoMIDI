

# Fuse high byte:
# 0xcf = 1 1 0 0   1 1 1 1 <-- RSTDISB (External Reset disable)
#        ^ ^ ^ ^   ^ ^ ^------ BODLEVEL0
#        | | | |   | +-------- BODLEVEL1
#        | | | |   + --------- BODLEVEL2
#        | | | +-------------- WDTON (WDT not always on)
#        | | +---------------- SPIEN (allow serial programming)
#        | +------------------ EESAVE (preserve EEPROM over chip erase)
#        +-------------------- DWEN (debugWIRE Enable)
# Fuse low byte:
# 0xef = 1 1 1 0   1 1 1 1
#        ^ ^ \ /   \--+--/
#        | |  |       +------- CKSEL 3..0 (external >8M crystal)
#        | |  +--------------- SUT 1..0 (crystal osc, BOD enabled)
#        | +------------------ CKOUT (1 = diable)
#        +-------------------- CKDEV8 (CLOCK DEV by 8)


###########################################################################
attiny4313at16.name=ATtiny4313 @ 16 MHz  (external crystal; 4.3 V BOD)

attiny4313at16.upload.using=tiny:arduinoisp
attiny4313at16.upload.maximum_size=4096

# Frequency 16.0- MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 65 ms
# Brown-out detection level at VCC=4.3 V
# Preserve EEPROM memory through the Chip Erase cycle
# Serial program downloading (SPI) enabled

attiny4313at16.bootloader.low_fuses=0xFF
attiny4313at16.bootloader.high_fuses=0xCF
attiny4313at16.bootloader.extended_fuses=0xFF
attiny4313at16.bootloader.path=empty
attiny4313at16.bootloader.file=empty4313at16.hex

attiny4313at16.build.mcu=attiny4313
attiny4313at16.build.f_cpu=16000000L
attiny4313at16.build.core=tiny

