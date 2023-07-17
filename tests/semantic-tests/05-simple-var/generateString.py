#!/usr/bin/python3
import random
import string

maxDepth = 5
topLevelStmts = 50
topLevelVars = 5

exprs = [ "term" ]
stmts = [ "assign" ]
terms = [ "str", ]
types = [ "string", ]

sDepth = 0
eDepth = 0

alphanum = string.ascii_letters + string.digits
vars = []

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
        return ""
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
            case "assign":
                return genVar() + ":=" + genExpr() + ";"
        return ""
    finally:
        sDepth -= 1

def genTerm():
    e = random.choice(exprs)
    match random.choice(terms):
        case "int":
            return genInt()
        case "str":
            return genStr()
        case "var":
            return genVar()

def genInt():
    return str(random.randint(1,0xFFFF))

def genStr():
    return '"' + (''.join(random.choices(alphanum, k=random.randint(1,20))))+'"'

def genFreshVar():
    global vars
    varname = random.choice(string.ascii_letters) + (''.join(random.choices(alphanum, k=random.randint(1,20))))
    vars.append(varname)
    return varname

def genVar():
    global vars
    return random.choice(vars)

def genGlobal():
    return "var " + genFreshVar() + " " + random.choice(types) + " := " + genTerm() + ";"


for i in range(topLevelVars):
    print(genGlobal())

for i in range(topLevelStmts):
    print(genStmt())
