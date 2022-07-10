
for n in $(seq 64 $[64 + 12])
do
        ./rawdata -S /usr/share/sounds/sf2/FluidR3_GM.sf2 -r 48000.0 -b 1024 -f float -t float -N 2 0 $n 127 3 0 $n 0
done

./rawdata -S /usr/share/sounds/sf2/FluidR3_GM.sf2 -r 48000.0 -b 1024 -f float -t float -N 2 0 64 127 0 0 75 127 3 0 64 0 0 0 75 0 
