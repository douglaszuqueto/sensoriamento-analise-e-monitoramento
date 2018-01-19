package main

import (
	"log"
	"database/sql"
	
	"github.com/gorilla/mux"
	_ "github.com/go-sql-driver/mysql"
)

func routes() *mux.Router {
	router := mux.NewRouter()

	db, err := sql.Open("mysql", "root:root@tcp(192.168.0.10)/sensor")

	if err != nil {
		log.Fatal(err)
	}

	sensorHandler := SensorHandler{}
	sensorHandler.db = db

	router.HandleFunc("/", sensorHandler.ListSensor).Methods("GET")
	router.HandleFunc("/{id}", sensorHandler.GetSensor).Methods("GET")
	router.HandleFunc("/", sensorHandler.CreateSensor).Methods("POST")

	return router
}