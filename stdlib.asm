FUNC_VAR_22B14C_00076DC0:
    IN
    RET

FUNC_VAR_22B14C_01435CD4:
    POP [0 + RDX]       # Number to print

    PUSH [0 + RDX]
    DUP
    OUT
    RET

FUNC_VAR_22B14C_0062909C:
    POP [0 + RDX]       # Number to get sqrt from

    PUSH [0 + RDX]
    SQRT
    RET

FUNC_VAR_22B14C_0013A52700E2108E01151151:
    POP [2 + RDX]       # X
    POP [1 + RDX]       # Y
    POP [0 + RDX]       # Symbol

    PUSH [1 + RDX]
    PUSH 50
    MUL
    PUSH [0 + RDX]
    ADD

    POP RAX

    PUSH [2 + RDX]
    PUSH 1000
    DIV

    POP [RAX]

    PUSH 1
    RET

FUNC_VAR_22B14C_0194AD2B:
    SHOW
    PUSH 1
    RET
