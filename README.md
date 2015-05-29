# Maze-Solver

Maze Solver - HodorBot

Proiectul a fost realizat folosind AtmelStudio 6.2

Observati:
	- Driverul de motor L298N este posibil sa nu alimenteze egal cele doua motoare,
	in cazul meu exista o diferenta de 0.15V pe o parte ceea ce facea robotul sa 
	o ia intr-o parte ⇒ problema rezolvata din cod, adaug celuilalt motor 
	diferenta necesara pentru a merge in linie dreapta

	- Datorita senzorilor analogici de 10cm - 80cm si algoritmului ales am fost
	nevoit sa imi construiesc un maze propriu cu distanta intre pereti de minim 
	15cm + latimea robotului
	
	- Robotul se bazeaza pe un algorithm de tipul right-hand wall follower putin
	modificat, datorita faptului ca folosesc senzori analogici pentru 10-80cm am 
	definit un “sweetspot” pentru robot care reprezinta pozitionarea intre 10cm 
	si 15cm fata de perete. Daca este mai aproape de 10cm se va departa putin, 
	daca este mai departe se va apropia putin. Se verifica constant sa se afle 
	in acest “sweetspot” si ajusteaza distanta cand este nevoie.
	
	- Robotul poate fi alimentat prin baterii 5V+ sau AC/DC adapter ( eu am folosit o 
	sursa 12V/5A trecuta printr-un regulator 7805 pentru a limita la 5V). 
