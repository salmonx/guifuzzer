

import idautils
import idaapi

def main():
    
    functions = {}
    refs = ""
    
    save_path = "c:\\fuzz\\data\\refs_notepad.txt"
    
    def locate(frm):
        for addr, func in functions.items():
            if frm > func['start_ea'] and frm < func['end_ea']:
                return addr
        return None
    
    for addr in Functions():
        func = idaapi.get_func(addr)        
        functions[addr] =  {'start_ea': func.startEA, 'end_ea': func.endEA}
        
    for item in Names():
        (addr, name) = item
        if addr in functions:
            functions[addr].update({'name': name})
   
    for item in Names():
        (to_addr, to_name) = item 
        for ref in XrefsTo(to_addr, 0):
            #if not ref.iscode: continue
            call_addr = ref.frm
            from_addr = locate(call_addr)
            if not from_addr: continue
            from_name = functions[from_addr]['name']
            
            refs += "%08x\t%08x\t%08x\t%50s\t%s\n" % (call_addr, from_addr, to_addr, from_name, to_name)
            
    print (refs)
    open(save_path, "w").write(refs)
            
            
if __name__ == '__main__':
    main()  