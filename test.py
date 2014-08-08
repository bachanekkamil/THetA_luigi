samples = {}

class A():
    global samples
    def run(self):
        samples['bob'] = 2

class B():
    def run(self):
        print samples['bob']

A().run()
B().run()
