mkdir -p build
cd build 
rm moteur
cmake ..
ln -s compile_commands.json ../compile_commands.json
make -j
./launch-moteur.sh