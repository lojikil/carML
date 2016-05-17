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
    print """if(cur == '{0}') {{
    state = {1}{2};
}} else {{
    state = LIDENT0;
}}
break;""".format(t, sym, idx)

print """
case {0}{1}:
if(iswhite(cur)) {{
    return {2};
}} else if(cur == ';') {{
    ungetc(cur, fdin);
    return {2};
}} else {{
    state = LIDENT0;
}}
""".format(sym, idx, returntype)
