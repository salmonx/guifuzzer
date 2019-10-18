import os
import time
import thread

crashes_dir = "c:\\out\\crashes"
hangs_dir = "c:\\out\\hangs"

PASSWORD = "pass"
TIMEOUT = 60


def check_password_in_files(dir = hangs_dir, password):
    files = os.listdir(dir)
    s = []
    for file in files:
        if not os.path.isdir(file):
            f = open(path + '/' + file);
            if password in f.read():
                return True

def check_password_hitted(password=PASSWORD):
    return check_password_in_files(crashes_dir, password) \
        and check_password_in_hangs(hangs_dir, password)


def test_a():
    #time.sleep(1)
    #
    
    return True
    
def test_b():
    print ("this is b")
    
    
    
    time.sleep(1)
    return False


def main():
    ok = 0
    
    testfuncs = [func for func in globals().keys() if func.startswith("test_")]
    for func in testfuncs:
        try:
            if eval(func)():
                ok += 1
                print ( "[Ok] " + func)
            else:
                print ("[Failed] " + func)
        except:
            pass
    
    print ("total: %s ok: %s" % (len(testfuncs), ok))
    

if __name__ == '__main__':
    main()