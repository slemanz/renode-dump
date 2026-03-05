*** Settings ***
Suite Setup                     Setup
Suite Teardown                  Teardown
Test Setup                      Reset Emulation
Resource                        ${RENODEKEYWORDS}

*** Variables ***
${RESC_FILE}                    ${CURDIR}/../blinky.repl
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

LED Should Be Off Initially
    [Documentation]             Verify that the LED starts in the off state after boot.
    Prepare Machine
    ${state}=                   Execute Command    ${LED} State
    Should Contain              ${state}    False
    ...                         msg=LED should be off after boot (before any toggle)

LED Should Change State After Running
    [Documentation]             Verify that the LED toggles after running the simulation.
    ...                         The firmware toggles PA5 every 500ms using SysTick, so
    ...                         after 600ms the LED must have changed state at least once.
    Prepare Machine
    ${initial}=                 Execute Command    ${LED} State
    Execute Command             emulation RunFor "0.6"
    ${final}=                   Execute Command    ${LED} State
    Should Not Be Equal         ${initial}    ${final}
    ...                         msg=LED state should have changed after 600ms of simulation

LED Should Toggle Multiple Times
    [Documentation]             Verify that the LED toggles back and forth.
    ...                         After ~1.1s (2 toggle periods + margin), it should be back
    ...                         to the initial state.
    Prepare Machine
    ${initial}=                 Execute Command    ${LED} State
    # After ~600ms -> first toggle (state changed)
    Execute Command             emulation RunFor "0.6"
    ${after_first}=             Execute Command    ${LED} State
    Should Not Be Equal         ${initial}    ${after_first}
    ...                         msg=LED should toggle after first 600ms
    # After another ~600ms -> second toggle (state restored)
    Execute Command             emulation RunFor "0.6"
    ${after_second}=            Execute Command    ${LED} State
    Should Not Be Equal         ${after_first}    ${after_second}
    ...                         msg=LED should toggle again after another 600ms