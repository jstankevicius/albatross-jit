#!/usr/bin/python3
import random
import string

maxDepth = 5
topLevelStmts = 5
topLevelVars = 5
topLevelFuns = 5

exprs = [ "+","-","*","%","|","&","!","&&","||","<",">","^",">=","<=","==","<>","term","call" ]
stmts = [ "while", "repeat", "if", "return", "assign", "else", "otherwise", "call" ]
terms = [ "int", "str", "var" ]
types = [ "int", "string", "char" ]

sDepth = 0
eDepth = 0

alphanum = string.ascii_letters + string.digits

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
            case "call":
                args = ",".join([("" + genExpr()) for x in range(random.randint(1,20)) ])
                return genVar() + "(" + args + ")"
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
            case "else":
                return "if (" + genExpr() + ") {" + genStmt() + "} else {" + genStmt() + "}"
            case "while":
                return "while (" + genExpr() + ") {" + genStmt() + "}"
            case "otherwise":
                return "while (" + genExpr() + ") {" + genStmt() + "} otherwise {" + genStmt() + "}"
            case "repeat":
                return "repeat (" + genExpr() + ") {" + genStmt() + "}"
            case "assign":
                return genVar() + ":=" + genExpr() + ";"
            case "call":
                args = ",".join([("" + genExpr()) for x in range(random.randint(1,20)) ])
                return genVar() + "(" + args + ");"
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
    return str(random.randint(0,0xFFFF))

def genStr():
    return '"' + (''.join(random.choices(alphanum, k=random.randint(1,20))))+'"'

def genVar():
    return random.choice(string.ascii_letters) + (''.join(random.choices(alphanum, k=random.randint(1,20))))

def genGlobal():
    return "var " + genVar() + " " + random.choice(types) + " := " + genExpr() + ";"

def genFun():
    args = ",".join([("" + genVar() + " " + random.choice(types)) for x in range(random.randint(1,20)) ])
    return "fun " + genVar() + " " + random.choice(types) + " ( " + args + ") {" + genStmt() + "}"


for i in range(topLevelVars):
    print(genGlobal())

for i in range(topLevelFuns):
    print(genFun())

for i in range(topLevelStmts):
    print(genStmt())
