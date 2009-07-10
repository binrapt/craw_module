def colour(index):
	return '%sc%d' % ('\xff', index)
	
def get_rainbow():
	output = ''
	for i in range(0, 10):
		output += '%s A %d A' % (colour(i), i)
	return output
	
white = colour(0)
red = colour(1)
green = colour(2)
blue = colour(3)
unique = colour(4)
grey = colour(5)
black = colour(6)
gold = colour(7)
orange = colour(8)
yellow = colour(9)