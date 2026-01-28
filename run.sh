if [ -e "saper" ]; then
    rm saper
fi
clear
gcc src/saper.c -osaper
./saper
if [ -e "saper" ]; then
    rm saper
fi
