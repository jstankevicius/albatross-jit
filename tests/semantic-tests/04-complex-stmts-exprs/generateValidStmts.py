#!/usr/bin/python3
import random
import string

maxDepth = 6
topLevelStmts = 10

exprs = [ "+","-","*","%","|","&","!","&&","||","<",">","^",">=","<=","==","<>","term" ]
stmts = [ "while", "repeat", "if", "return" ]

sDepth = 0
eDepth = 0

def genExpr():
    e = random.choice(exprs)
    match e:
        case "term":
            return genTerm()
        case "!":
            return "!" + genTerm()
        case _:
            return genTerm() + e + genTerm()
    return ""

def genStmt():
    global sDepth
    sDepth += 1
    try:
        if sDepth == maxDepth:
            return "return " + genExpr() + ";"

        e = random.choice(stmts)
        match e:
            case "return":
                return "return " + genExpr() + ";"
            case "if":
                return "if (" + genExpr() + ") {" + genStmt() + "}"
            case "while":
                return "while (" + genExpr() + ") {" + genStmt() + "}"
            case "repeat":
                return "repeat (" + genExpr() + ") {" + genStmt() + "}"
        return ""
    finally:
        sDepth -= 1

def genTerm():
    return str(random.randint(0,0xFFFF))

for i in range(topLevelStmts):
    print(genStmt())
