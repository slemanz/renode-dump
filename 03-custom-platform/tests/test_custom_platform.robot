*** Settings ***
Suite Setup                     Setup
Suite Teardown                  Teardown
Test Setup                      Reset Emulation
Resource                        ${RENODEKEYWORDS}

*** Variables ***
${STATUS_LED}                   sysbus.gpioPortA.StatusLED
${ERROR_LED}                    sysbus.gpioPortB.ErrorLED
${BUTTON}                       sysbus.gpioPortA.UserButton

*** Keywords ***
Prepare Machine
    Execute Command             mach create "STM32F411"
    Execute Command             machine LoadPlatformDescription @platforms/cpus/stm32f4.repl
    Execute Command             machine LoadPlatformDescription @${CURDIR}/../stm32f411_custom.repl
    Execute Command             sysbus LoadELF @${CURDIR}/../Build/flash.elf
    Create Terminal Tester      sysbus.usart2

*** Test Cases ***
Should Boot And Print Ready
    [Documentation]             Verify boot message appears on UART.
    Prepare Machine
    Start Emulation
    Wait For Line On Uart       Platform ready    timeout=3

Status LED Should Blink
    [Documentation]             Status LED (PA5) should toggle within 600ms.
    Prepare Machine
    ${initial}=                 Execute Command    ${STATUS_LED} State
    Execute Command             emulation RunFor "0.6"
    ${after}=                   Execute Command    ${STATUS_LED} State
    Should Not Be Equal         ${initial}    ${after}
    ...                         msg=Status LED should toggle after 600ms

Error LED Should Be Off At Boot
    [Documentation]             Error LED (PB3) must be off when no button is pressed.
    Prepare Machine
    Execute Command             emulation RunFor "0.1"
    ${state}=                   Execute Command    ${ERROR_LED} State
    Should Contain              ${state}    False

Button Press Should Activate Error LED
    [Documentation]             Pressing the button should turn on the error LED
    ...                         and print "Error ON" on UART.
    Prepare Machine
    Start Emulation

    # Wait for boot
    Wait For Line On Uart       Platform ready    timeout=3

    # Press button
    Execute Command             ${BUTTON} Press
    Wait For Line On Uart       Error ON          timeout=3
    ${state}=                   Execute Command    ${ERROR_LED} State
    Should Contain              ${state}    True
    ...                         msg=Error LED should be ON after button press

Button Release Should Deactivate Error LED
    [Documentation]             Releasing the button should turn off the error LED
    ...                         and print "Error OFF" on UART.
    Prepare Machine
    Start Emulation

    Wait For Line On Uart       Platform ready    timeout=3

    # Press and release
    Execute Command             ${BUTTON} Press
    Wait For Line On Uart       Error ON          timeout=3
    Execute Command             ${BUTTON} Release
    Wait For Line On Uart       Error OFF         timeout=3
    ${state}=                   Execute Command    ${ERROR_LED} State
    Should Contain              ${state}    False
    ...                         msg=Error LED should be OFF after button release

Both LEDs Should Operate Independently
    [Documentation]             Status LED should keep blinking even while error LED is active.
    Prepare Machine

    # Boot the system
    Execute Command             emulation RunFor "0.5"

    # Activate error LED
    Execute Command             ${BUTTON} Press
    Execute Command             emulation RunFor "0.1"

    # Verify error LED is on
    ${error_state}=             Execute Command    ${ERROR_LED} State
    Should Contain              ${error_state}    True
    ...                         msg=Error LED should be ON after button press

    # Check status LED still toggles
    ${before}=                  Execute Command    ${STATUS_LED} State
    Execute Command             emulation RunFor "0.6"
    ${after}=                   Execute Command    ${STATUS_LED} State
    Should Not Be Equal         ${before}    ${after}
    ...                         msg=Status LED should keep blinking while error LED is active