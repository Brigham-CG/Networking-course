all:
	mkdir workspace workspace/client1 workspace/client2 workspace/client3
	g++ -o server server.cpp -lcrypto
	g++ -o client client.cpp -lcrypto
	cp client workspace/client1
	cp client workspace/client2
	cp client workspace/client3
	cp files/hacking_group.jpg workspace/client1
	cp files/texto.txt workspace/client1
	cp files/mario.png workspace/client1
	cp files/wallpaper.jpg workspace/client1
	cp files/code_1.82.2-1694671812_amd64.deb workspace/client1

clean:
	rm -rf workspace
	rm -f server client
