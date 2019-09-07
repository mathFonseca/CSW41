import matplotlib.image as img 

import numpy as npy 
  


m = img.imread("avatar.png") 



def zoom(imagem, fator, nomesaida):

	w, h = imagem.shape[:2]
	  
	

	xNew = int(w * fator) 
	
	yNew = int(h * fator) 
	   
	

	xScale = xNew/(w-1)
	
	yScale = yNew/(h-1) 
	  
	

	newImage = npy.zeros([xNew, yNew, 4]) 
	  
	

	for i in range(xNew-1): 

	  for j in range(yNew-1): 
	
	    newImage[i + 1, j + 1]= imagem[1 + int(i / xScale), 1 + int(j / yScale)]
	
		
	img.imsave(nomesaida, newImage)



factinit = 1

for i in range(0, 5):

	zoom(m, factinit, "diminui" + str(i) + ".png")
	
	factinit /= 2
	
	print(factinit)




factinit = 1

for i in range(0, 5):

	zoom(m, factinit, "aumenta" + str(i) + ".png")

	factinit *= 2
	
	print(factinit)