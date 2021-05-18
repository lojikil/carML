import sys

if len(sys.argv) != 4:
    print "usage: gen.py [symbol name] [string] [return type]"
    sys.exit(1)

sym = sys.argv[1]
tar = sys.argv[2]
returntype = sys.argv[3]
idx = 0

for t in tar:
    print "case {0}{1}: ".format(sym, idx)
    idx += 1
    print """    if(cur == '{0}') {{
        state = {1}{2};
    }} else if(iswhite(cur) || cur == '\\n' || isbrace(cur)) {{
        ungetc(cur, fdin);
        buf[idx - 1] = '\\0';
        return TIDENT;
    }} else {{
        state = LIDENT0;
    }}
    break;""".format(t, sym, idx)

print """
case {0}{1}:
    if(isident(cur)) {{
        substate = LIDENT0;
    }}else if(iswhite(cur) || cur == '\\n' || isbrace(cur)) {{
        ungetc(cur, fdin);
        return {2};
    }} else {{
        strncpy(buf, "malformed identifier", 512);
        return TERROR;
    }}
    break;
""".format(sym, idx, returntype)
