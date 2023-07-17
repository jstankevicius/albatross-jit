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
    global error;
    bug = (random.randint(0,100) == 1)
    if bug:
        error = True
    e = random.choice(exprs)
    match e:
        case "term":
            return genTerm() if not bug else genBug()
        case "!":
            return "!" + (genTerm() if not bug else genBug())
        case _:
            return genTerm() + e + (genTerm() if not bug else genBug())
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

def genBug():
    return '"' + str(random.randint(0,0xFFFF)) + '"'

while not error:
    for i in range(topLevelStmts):
        print(genStmt())
