import random
import time

text = "password"
f = "c:\\out\\cur.input"

def gen_randstr():
    str = ""
    for i in range(8):
        str += text[random.randint(0, len(text)-1)]
    return str
    
    
def main():
    while True:
        str = gen_randstr()
        
        try:
            with open(f) as fp:
                old = fp.read()
                print (old, str)

            with open(f, "w") as fp:
                
                fp.write(str)
                fp.close()
        except Exception as e:
            print (e)
            
        time.sleep(3)
    

if __name__ == "__main__":
    main()