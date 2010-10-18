FaceTracker/README.txt
2010-10-18, Pedro Cortez <pcortez@gmail.com>

MAIN ARG
el main tiene como input 5 parametros:

-rightVideo   
	arg:path al primer video a trackear (se parte por el de la camara derecha)

-topVideo
	arg: path al segundo video a trackear (cuando el objeto se sale del primer video continua en el segundo video - es el video con la vista superior o top)

-configPathRV
	arg: path al archivo de config del primer video (rightVideo)

-configPathTV
	arg: path al archivo de config del segundo video (topVideo)

-debug
	no tiene argumento - se si se agrega, imprime informacion de debug y ademas hace cvWaiting() antes de partir cada video


CONFIG FILES:
el orden de los archivos config

12:01,13:20// intervalo de tiempo total en que se debe trackear al objeto (incluye el tiempo del primer y segundo video para los dos config debe ser igual). acepta milisegundos (12:01.3)
478,100 //posicion x e y del centro del cuadrado inicial
78,90  //ancho y alto del cuadrado inciial
0// angulo de rotacion inicial
FACE // si es cara - se puede poner PEDESTRIAN
/Users/pcortez/Documents/Magister/Codigo/FaceTracker/video //donde se guarda el video que se crea
1 //1 = si son imagenes RGB - 0= BW 
30 // la memoria del modelo - dejalo en 30 
0// alpha = factor de ponderacion de la posicion de los cuadrantes muestreados - si estas usando nuestros videos usa 0 pero puedes poner del orden de 0.05
2.5 // espacio de caunto busca en el tracking la formula es x+x-width/2.5 - en el caso que sea 0 busca en todo el patch
0.9,1,1.3 // las escalas que prueba el tracking
0// los angulos que prueba el tracking