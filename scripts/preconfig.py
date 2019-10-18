import pefile
import sys

inipath = "c:\\fuzz_config\\pe.ini"

def extract_sections(pepath):
    ini = list()
  
    pe = pefile.PE(pepath)
    for section in pe.sections:
        name = bytes.decode(section.Name).strip('\x00')
        
        startaddr = pe.OPTIONAL_HEADER.ImageBase + section.VirtualAddress
        endaddr = startaddr + section.Misc_VirtualSize
        
        ini.append(str(name) + "_start=" + str(hex(startaddr)))
        ini.append(str(name) + "_end=" + str(hex(endaddr)))
      
    open(inipath, "w").write("\n".join(ini));

        
def main():
    if len(sys.argv) != 2:
        print ("Usage %s pefile" % (sys.argv[0]))
        exit(0)
    pepath = sys.argv[1]
    #pepath = "c:\\winafl-master\\build32\\bin\\release\\test.exe"
    extract_sections(pepath)

if __name__ == '__main__':
    main()