# -----------------------------------------------------------------------
# This is an example illustrating how to enumerate imports
# (c) Hex-Rays
#
import idaapi


s = ""

def imp_cb(ea, name, ord):
    global s
    if not name:
        print "%08x: ord#%d" % (ea, ord)
        s += "%08x: ord#%d" % (ea, ord) + "\n"
    else:
        print "%08x: %s (ord#%d)" % (ea, name, ord)
        s += "%08x: %s (ord#%d)" % (ea, name, ord) + "\n"
    # True -> Continue enumeration
    # False -> Stop enumeration
    return True

nimps = idaapi.get_import_module_qty()

print "Found %d import(s)..." % nimps
s += "Found %d import(s)..." % nimps + "\n"

for i in xrange(0, nimps):
    name = idaapi.get_import_module_name(i)
    if name != "mfc100" : continue
   

    print "Walking-> %s" % name
    s += "Walking-> %s" % name + "\n"
    idaapi.enum_import_names(i, imp_cb)

print "All done..."

open("c:\\fuzz\\imports.txt", "w").write(s)
