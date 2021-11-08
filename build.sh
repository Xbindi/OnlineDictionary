#/bin/bash

cd ./dict_to_sql
make 
cp dict.db ../online_dict
cd ../online_dict
make