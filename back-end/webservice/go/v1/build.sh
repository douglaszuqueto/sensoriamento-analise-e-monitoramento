#!/bin/sh

export GOPATH=$(pwd)

echo -e "Realizando build da API...\n"

go build -o sensoriamento-api

echo -e "\nBuild realizado com sucesso!"
