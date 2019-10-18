import unittest
import xmlrunner
import time


class QtInterceptorsTest(unittest.TestCase):

    def tearDown(self):
        print("after test")
    
    def setUp(self):
        print("before test")

    @classmethod
    def tearDownClass(self):
    # 必须使用 @ classmethod装饰器, 所有test运行完后运行一次
         print('after test class')
    @classmethod
    def setUpClass(self):
    # 必须使用@classmethod 装饰器,所有test运行前运行一次
        print('before test class')
        
        
    
    def test_a(self):
        print("aaa")
    
    def test_b(self):
    
        time.sleep(1)
        
        print("bbb")
        
    
    def test_1(self, a, b, c):
        #self.assertEqual(a+b, c)
        self.assertEqual(2, 2)
        
        
if __name__ == '__main__':
    test_suite = unittest.TestSuite()
    test_suite.addTest(unittest.makeSuite(QtInterceptorsTest))
    runner = xmlrunner.XMLTestRunner(output="report")
    runner.run(test_suite)