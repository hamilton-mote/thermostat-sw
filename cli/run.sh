go build -o cli
sudo setcap 'cap_net_raw,cap_net_admin+eip' cli
sudo service bluetooth stop
./cli
sudo service bluetooth start
