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