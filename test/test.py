import luigi
import subprocess


class TaskA(luigi.Task):
	def run(self):
		subprocess.call(['touch', "TaskA"])
		print 'TaskA executed'
	def output(self):
		return luigi.LocalTarget('TaskA')

class TaskB(luigi.Task):
	def requires(self):
		return TaskA()
	def run(self):
		subprocess.call(['touch', "TaskB"])
		print 'TaskB executed'
	def output(self):
		return luigi.LocalTarget("TaskB")


class TaskC(luigi.Task):
	def requires(self):
		return TaskA()
	def run(self):
		subprocess.call(['touch', "TaskC"])
		print 'TaskC executed'
	def output(self):
		return luigi.LocalTarget("TaskC")

class TaskD(luigi.Task):
	def requires(self):
		subprocess.call(['touch', "TaskD"])
		return TaskA()
	def run(self):
		print 'TaskD executed'
	def output(self):
		return luigi.LocalTarget("TaskD")

class TaskE(luigi.Task):
	def requires(self):
		subprocess.call(['touch', "TaskE"])
		return {'TaskC':TaskC(), 'TaskD':TaskD()}
	def run(self):
		print 'TaskE executed.'
	def output(self):
		return luigi.LocalTarget("TaskE")



luigi.build([TaskE(), TaskB()], workers = 4)