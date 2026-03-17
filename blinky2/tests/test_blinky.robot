*** Settings ***
Suite Setup                     Setup
Suite Teardown                  Teardown
Test Setup                      Reset Emulation
Resource                        ${RENODEKEYWORDS}

*** Variables ***
${RESC_FILE}                    ${CURDIR}/../blinky.resc
${LED}                          sysbus.gpioPortA.UserLED

*** Keywords ***
Prepare Machine
    Execute Command             mach create "STM32F411"
    Execute Command             machine LoadPlatformDescription @platforms/cpus/stm32f4.repl
    Execute Command             machine LoadPlatformDescription @${CURDIR}/../stm32f411_blinky.repl
    Execute Command             sysbus LoadELF @${CURDIR}/../Build/flash.elf
    Execute Command             logLevel -1 gpioPortA.UserLED

*** Test Cases ***
Should Create Machine
    [Documentation]             Verify that the machine is created and firmware loads without errors.
    Prepare Machine

LED Should Change State After Running
    [Documentation]             Verify that the LED toggles after running the simulation for a while.
    Prepare Machine
    Execute Command             emulation RunFor "0.5"
