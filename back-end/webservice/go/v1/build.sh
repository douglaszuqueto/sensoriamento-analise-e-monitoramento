#!/bin/sh

export GOPATH=$(pwd)

echo -e "Realizando build da API...\n"

go get github.com/gorilla/mux
go get github.com/go-sql-driver/mysql
go build -o bin/sensoriamento-api

echo -e "\nBuild realizado com sucesso!"
