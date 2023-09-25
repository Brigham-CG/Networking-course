
all:
	mkdir workspace workspace/client1 workspace/client2 workspace/client3
	g++ -o server server.cpp -lcrypto
	g++ -o client client.cpp -lcrypto
	cp client workspace/client1
	cp client workspace/client2
	cp client workspace/client3
	cp hacking_group.jpg workspace/client1
	cp texto.txt workspace/client1
	cp mario.png workspace/client1
	cp wallpaper.jpg workspace/client1

clean:
	rm -rf workspace
	rm -f server client
