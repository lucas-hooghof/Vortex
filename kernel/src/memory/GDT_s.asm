global LoadGDT

LoadGDT:
    LGDT [RDI]

    PUSH 0x08
    LEA RAX, [REL .reload_CS]
    PUSH RAX
    RETFQ

.reload_CS:
    MOV AX,0x10
    MOV DS,AX
    MOV ES,AX
    MOV FS,AX
    MOV GS,AX
    MOV SS,AX

    RET
