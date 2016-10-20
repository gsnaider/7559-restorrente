kill -2 $(ps aux | grep 'Resto' | awk '{print $2}' | head -1)
