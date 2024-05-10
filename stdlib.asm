# Function declaration
FUNC_VAR_22B14C_00076DC0:
    IN
    RET


# Function declaration
FUNC_VAR_22B14C_01435CD4:
    # Function parameter
    POP [0 + RDX]

    PUSH [0 + RDX]
    DUP
    OUT
    RET


# Function declaration
FUNC_VAR_22B14C_0062909C:
    # Function parameter
    POP [0 + RDX]

    PUSH [0 + RDX]
    SQRT
    RET


# Function declaration
FUNC_VAR_22B14C_0013A52700E2108E01151151:
    # Function parameter
    POP [2 + RDX]
    # Function parameter
    POP [1 + RDX]
    # Function parameter
    POP [0 + RDX]


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


# Function declaration
FUNC_VAR_22B14C_0194AD2B:
    SHOW
    PUSH 1
    RET
