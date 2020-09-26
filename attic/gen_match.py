for x in ["8","16","32","64"]:
    print """if(!strncmp(typespec->value, "U{0}", {1})) {{
    strncat(dst, "uint{0}", {2});
}}
""".format(x, len(x) + 1, len(x) + 4)

for x in ["8","16","32","64"]:
    print """if(!strncmp(typespec->value, "I{0}", {1})) {{
    strncat(dst, "int{0}", {2});
}}
""".format(x, len(x) + 1, len(x) + 4)
