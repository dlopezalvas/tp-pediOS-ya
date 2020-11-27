echo "Deploy TP"
cd ~

cd workspace/so-commons-library
sudo make install
cd ~
cd workspace/tp-2020-2c-CoronaLinux/commonsCoronaLinux/Debug
make clean
make all
echo "Por Instalar el LD_LIBRARY_PATH, el actual valor es $LD_LIBRARY_PATH"
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/workspace/tp-2020-2c-CoronaLinux/commonsCoronaLinux/Debug/
echo "Cambiado a $LD_LIBRARY_PATH"
cd ..
cd ..
cd Restaurante/Debug
make clean
make all
cd ..
cd ..
cd App/Debug
make clean
make all
cd ..
cd ..
cd Sindicato/Debug
make clean
make all
cd ..
cd ..
cd Comanda/Debug
make clean
make all
cd ..
cd ..
cd Cliente/Debug
make clean
make all
echo "Compilacion lista"
