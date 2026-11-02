
*** Settings ***
Library    SerialLibrary    encoding=ascii

Suite Setup       Connect Serial
Suite Teardown    Disconnect Serial

*** Variables ***
${com}       COM5
${baud}      115200

*** Test Cases ***

String lenght
    Send Time And Check    000120    80X
    Send Time And Check    00120     -1X

Numbers
    Send Time And Check    000120    80X
    Send Time And Check    00A120    -3X

Time > 0
    Send Time And Check    000001    1X
    Send Time And Check    000000    -4X

Hours limits
    Send Time And Check    235959    86399X
    Send Time And Check    245959    -3X

Minutes limits
    Send Time And Check    005959    3599X
    Send Time And Check    006059    -3X

Seconds limits
    Send Time And Check    000059    59X
    Send Time And Check    000060    -3X

Invalid seconds
    Send Time And Check    001059    659X
    Send Time And Check    001067    -3X


*** Keywords ***

Send Time And Check
    [Arguments]    ${time_string}    ${expected}

    Reset Input Buffer
    Write Data    ${time_string}X

    ${read}=    Read Until    terminator=X

    Log To Console    Sent: ${time_string}X
    Log To Console    Received: ${read}

    Should Be Equal As Strings    ${read}    ${expected}


Connect Serial
    Log To Console    Connecting to ${com}

    Add Port    ${com}    baudrate=${baud}    timeout=2
    Port Should Be Open    ${com}

    Sleep    1s

    Reset Input Buffer
    Reset Output Buffer


Disconnect Serial
    Log To Console    Disconnecting serial port
    Delete Port    ${com}
