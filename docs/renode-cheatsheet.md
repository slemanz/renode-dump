# Renode Cheatsheet

Quick reference for daily use. Covers monitor commands, CLI shortcuts, and
common patterns.

## Starting Renode

```bash
# Interactive (with GUI)
renode script.resc

# Headless (no GUI, for CI)
renode --disable-xwt -e "include @script.resc; start; sleep 5; quit"

# Run Robot Framework tests
renode-test tests/test_file.robot

# Run tests with output in a specific directory
renode-test tests/test_file.robot --results-dir tests/results
```

## Machine Management

```bash
# Create / select / list machines
mach create "name"                      # create a new machine
mach create                             # create with default name
mach set "name"                         # switch to a machine
mach set 0                              # switch by index
mach list                               # show all machines

# Load platform and firmware
machine LoadPlatformDescription @path/to/file.repl
sysbus LoadELF @path/to/firmware.elf
sysbus LoadBinary @path/to/firmware.bin 0x08000000

# Reset
machine Reset                           # reload firmware, restart CPU
runMacro $reset                         # execute the reset macro
```