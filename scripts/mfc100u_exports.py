s = "ida_mfc100u_exports.txt"

con = open(s).read()
newcon = ""

for line in con.split('\n'):
	
	items = line.split('\t')
	newcon += str(hex(int(items[1], 16) - 0x785f0000)) + "\t" +  str(hex(int(items[1], 16)))   + '\t' + items[0] + "\n"

open("mfc100u_exports.txt", "w").write(newcon)

 
