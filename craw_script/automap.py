from utility import *
from text import *
import sys, traceback, craw

class automap_handler:

	def __init__(self):
		self.original_npc_colour = 0x20
		
		self.npc_colour = self.original_npc_colour
		 
		self.monster_colour = 0x0a
		self.minion_colour = 0xab
		self.champion_colour = 0x9b
		self.boss_colour = 0x87
		
	def is_minion(self):
		return get_flag(self.monster_data.monster_flags, 4)
		
	def is_champion(self):
		return get_flag(self.monster_data.monster_flags, 2)
		
	def is_boss(self):
		return get_flag(self.monster_data.monster_flags, 3)
		
	def is_npc(self):
		return get_flag(self.monster_data.flags, 9)
	
	def process_data(self, monster_data):
		try:
			#craw.draw_text(get_rainbow(), 32, 32, False)
			
			self.monster_data = monster_data
			
			coordinate = (monster_data.x, monster_data.y)
			
			if monster_data.type != 1:
				if monster_data.colour != 0x00:
					draw_box(coordinate, monster_data.colour)
				return
				
			if monster_data.colour == self.original_npc_colour:
				colour = self.npc_colour
			else:
				if monster_data.mode in [0, 12]:
					return
					
				if monster_data.treasure_class[0] == 0:
					return
					
				if self.is_boss():
					colour = self.boss_colour
				elif self.is_champion():
					colour = self.champion_colour
				elif self.is_minion():
					colour = self.minion_colour
				else:
					colour = self.monster_colour
					
				immunity_colours = [red, blue, yellow, green, gold, orange]
				immunities = [
					monster_data.fire_resistance,
					monster_data.cold_resistance,
					monster_data.lightning_resistance,
					monster_data.poison_resistance,
					monster_data.damage_resistance,
					monster_data.magic_resistance
				]
				
				
				immunity_string = ''
				offset = 0
				for immunity in immunities:
					if immunity >= 100:
						immunity_string += '%so' % immunity_colours[offset]
					
					offset += 1
					
				if len(immunity_string) > 0:
					draw_automap_text(immunity_string, coordinate)
					
			draw_box(coordinate, colour)
			
		except:
			traceback.print_exc(file = sys.stdout)