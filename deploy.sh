echo "Deploy TP"
cd ~

cd workspace/so-commons-library
sudo make install
cd ~
cd workspace/tp-2020-2c-CoronaLinux/commonsCoronaLinux/Debug
make clean
make all
chmod +x configurarPathLibrerias.sh
source configurarPathLibrerias.sh
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
