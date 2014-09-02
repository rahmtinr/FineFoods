import sys
import os
import matplotlib.pyplot as plt
import numpy as np
import pylab
import matplotlib

def autolabel(rects):
    # attach some text labels
    for rect in rects:
        height = rect.get_height()
        ax.text(rect.get_x()+rect.get_width()/2., 1.05*height, '%d'%int(height),
                ha='center', va='bottom')



matplotlib.rcParams.update({'font.size': 8})

for root, dirs, files in os.walk('../Output_FineFoods'):
    for file in files:
        if file.endswith(".txt"):
#	if root.endswith("PerItem"):	
#				continue
			with open(root + "/" + file) as f:
   				lines = f.readlines()
				x = []
				y = []
				for line in lines:
					x.append(line.split()[0])
					y.append(int(line.split()[1]))
				if len(x) == 1:
					continue
				ind = np.arange(len(y))             # the x locations for the groups
				width = 0.35       		                 # the width of the bars
				fig, ax = plt.subplots()
				rects1 = ax.bar(ind, y, width, color='b')

				# add some text for labels, title and axes ticks
				ax.set_ylabel('number of reviews')
				ax.set_title(file[:-4])
				ax.set_xticks(ind+width)
				ax.set_xticklabels(x, rotation=30)

				## add a legend
				autolabel(rects1)
				pylab.savefig(root + '/' + file[:-4] + '.png')	

#for line in lines:
#	sys.stdout.write(line)
