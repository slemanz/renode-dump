*** Settings ***
Suite Setup                     Setup
Suite Teardown                  Teardown
Test Setup                      Reset Emulation
Resource                        ${RENODEKEYWORDS}

*** Variables ***
${LED}                          sysbus.gpioPortA.UserLED

*** Keywords ***
Prepare Machine
    Execute Command             mach create "STM32F411"
    Execute Command             machine LoadPlatformDescription @platforms/cpus/stm32f4.repl
    Execute Command             machine LoadPlatformDescription @${CURDIR}/../stm32f411_timer.repl
    Execute Command             sysbus LoadELF @${CURDIR}/../Build/flash.elf
    Create Terminal Tester      sysbus.usart2

*** Test Cases ***
Should Print TIM2 Started
    [Documentation]             Verify firmware boots and TIM2 initializes.
    Prepare Machine
    Start Emulation
    Wait For Line On Uart       TIM2 started    timeout=3

Should Print First Tick After 500ms
    [Documentation]             TIM2 fires every 500ms. First tick message should
    ...                         appear shortly after boot.
    Prepare Machine
    Start Emulation
    Wait For Line On Uart       TIM2 started     timeout=3
    Wait For Line On Uart       [TIM2] tick 1    timeout=3

Should Print Sequential Ticks
    [Documentation]             Verify that tick counter increments: 1, 2, 3.
    ...                         This confirms the ISR fires repeatedly.
    Prepare Machine
    Start Emulation
    Wait For Line On Uart       [TIM2] tick 1    timeout=3
    Wait For Line On Uart       [TIM2] tick 2    timeout=3
    Wait For Line On Uart       [TIM2] tick 3    timeout=3

LED Should Toggle On Each Tick
    [Documentation]             The ISR toggles PA5. After an odd number of ticks
    ...                         the LED should be in opposite state from start.
    Prepare Machine
    ${initial}=                 Execute Command    ${LED} State
    Start Emulation
    # Wait for first tick (LED toggled once)
    Wait For Line On Uart       [TIM2] tick 1    timeout=3
    ${after_1}=                 Execute Command    ${LED} State
    Should Not Be Equal         ${initial}    ${after_1}
    ...                         msg=LED should toggle on tick 1

    # Wait for second tick (LED toggled back)
    Wait For Line On Uart       [TIM2] tick 2    timeout=3
    ${after_2}=                 Execute Command    ${LED} State
    Should Not Be Equal         ${after_1}    ${after_2}
    ...                         msg=LED should toggle again on tick 2

Boot Message Should Precede Timer Ticks
    [Documentation]             "TIM2 started" must appear before any tick.
    Prepare Machine
    Start Emulation
    Wait For Line On Uart       TIM2 started     timeout=3
    Wait For Line On Uart       [TIM2] tick 1    timeout=3
