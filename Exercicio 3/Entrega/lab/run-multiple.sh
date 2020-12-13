mkdir build
cd build
cmake ..
make

PORT="8577"
CWD="$(pwd)"
xterm -title "Server" -hold -e "./server/server $PORT" &
sleep 2
xterm -title "Client 1" -hold -e "./client/client 127.0.0.1 $PORT" &
xterm -title "Client 2" -hold -e "./client/client 127.0.0.1 $PORT" &
xterm -title "Client 3" -hold -e "./client/client 127.0.0.1 $PORT" &
xterm -title "Client 4" -hold -e "./client/client 127.0.0.1 $PORT" &
xterm -title "Client 5" -hold -e "./client/client 127.0.0.1 $PORT" &
xterm -title "Client 6" -hold -e "./client/client 127.0.0.1 $PORT" &
xterm -title "Client 7" -hold -e "./client/client 127.0.0.1 $PORT" &
xterm -title "Client 8" -hold -e "./client/client 127.0.0.1 $PORT" &
xterm -title "Client 9" -hold -e "./client/client 127.0.0.1 $PORT" &
xterm -title "Client 10" -hold -e "./client/client 127.0.0.1 $PORT" &
