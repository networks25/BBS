for i in 1 2 3 4; do
  (echo "hi from client $i" | nc 127.0.0.1 12345 &)
done

