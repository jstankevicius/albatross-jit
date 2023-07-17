#!/usr/bin/python3
import random
import string

maxDepth = 6
topLevelStmts = 10
error=False

exprs = [ "+","-","*","%","|","&","!","&&","||","<",">","^",">=","<=","==","<>","term" ]
stmts = [ "while", "repeat", "if", "return" ]

sDepth = 0
eDepth = 0

def genExpr():
    global eDepth
    eDepth += 1

    try:
        if eDepth == maxDepth:
            return genTerm()

        e = random.choice(exprs)
        match e:
            case "term":
                return genTerm()
            case "!":
                return "!" + genExpr()
            case _:
                return genExpr() + e + genExpr()
    finally:
        eDepth -= 1

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
    global error
    bug = (random.randint(0,100) == 1)
    if not error and bug:
        error = True
        return '"' + str(random.randint(0,0xFFFF)) + '"'

    return str(random.randint(0,0xFFFF))

for i in range(topLevelStmts):
    print(genStmt())
