*** Settings ***
Suite Setup                     Setup
Suite Teardown                  Teardown
Test Setup                      Reset Emulation
Resource                        $(RENODEKEYWORDS)

*** Variables ***
${LED}                          sysbus.gpioPortA.UserLED

*** Keywords ***
Prepare Machine
    Execute Command             mach create "STM32F411"
    Execute Command             machine LoadPlatformDescription @platforms/cpus/stm32f4.repl
    Execute Command             machine LoadPlatformDescription @${CURDIR}/../stm32f411_uart.repl
    Execute Command             sysbus LoadELF @${CURDIR}/../Build/flash.elf
    Create Terminal Tester      sysbus.usart2

*** Test Cases ***
Should Print Boot Message
    [Documentation]             Verify that the firmware prints the boot message on UART2.
    ...                         This is the most basic UART test — confirms that the UART
    ...                         peripheral is properly configured and printf redirection works.
    Prepare Machine
    Start Emulation
    Wait For Line On Uart       Boot OK    timeout=5

Should Print Toggle Messages
    [Documentation]             Verify that the firmware prints periodic toggle messages.
    ...                         The firmware prints "[N] LED toggled" every 1000ms.
    ...                         We wait for the first two messages to confirm timing works.
    Prepare Machine

    Start Emulation
    Wait For Line On Uart       LED toggled    timeout=3
    Wait For Line On Uart       LED toggled    timeout=3

Should Print Sequential Counter
    [Documentation]             Verify that the counter increments across messages.
    ...                         First message should contain [1], second should contain [2].
    Prepare Machine

    Start Emulation
    Wait For Line On Uart       [1] LED toggled    timeout=3
    Wait For Line On Uart       [2] LED toggled    timeout=3
    Wait For Line On Uart       [3] LED toggled    timeout=3

LED Should Toggle With UART Messages
    [Documentation]             Verify that LED state changes match UART messages.
    ...                         After the boot message, the LED should toggle on each
    ...                         "LED toggled" message.
    Prepare Machine

    # Get initial LED state
    ${initial}=                 Execute Command    ${LED} State

    Start Emulation

    # Wait for first toggle
    Wait For Line On Uart       [1] LED toggled    timeout=3
    ${after_first}=             Execute Command    ${LED} State
    Should Not Be Equal         ${initial}    ${after_first}
    ...                         msg=LED should have toggled on first message

    # Wait for second toggle
    Wait For Line On Uart       [2] LED toggled    timeout=3
    ${after_second}=            Execute Command    ${LED} State
    Should Not Be Equal         ${after_first}    ${after_second}
    ...                         msg=LED should have toggled again on second message

Boot Message Should Appear Before Toggle
    [Documentation]             Verify message ordering: "Boot OK" must appear before
    ...                         any toggle message. This validates the init sequence.
    Prepare Machine

    Start Emulation
    Wait For Line On Uart       Boot OK            timeout=3
    Wait For Line On Uart       [1] LED toggled    timeout=3