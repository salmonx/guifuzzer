from idautils import *
from idaapi import *
import idc
from idc import *



output = ""

for segea in Segments():
    segname = SegName(segea)
    for funcea in Functions(segea, SegEnd(segea)):
        functionName = GetFunctionName(funcea)
        for (startea, endea) in Chunks(funcea):
            #print functionName, ":", "0x%08x"%(startea), "0x%08x"%(endea)
            output += functionName + ":" + "0x%08x"%(startea) + "-" + "0x%08x"%(endea) + "\n"

            for head in Heads(startea, endea):
                mcodes = "".join(["%02X"%(ord(ch)) for ch in idc.GetManyBytes(head, ItemSize(head))])
                #print "0x%08x"%(head), mcodes, GetDisasm(head)
                output += "0x%08x"%(head) + "\t" +  mcodes + "\t" + GetDisasm(head) + "\n"
                


open("c:\\fuzz\\disasm_notepad.txt", "w").write(output)