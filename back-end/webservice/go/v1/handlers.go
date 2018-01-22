package main

import (
	"fmt"
	"net/http"
	"encoding/json"
	"database/sql"
	"strconv"

	"github.com/gorilla/mux"
)

type SensorHandler struct {
	db *sql.DB
}

type CreateSensorRequest struct {
	Id int `json:"id"`
	Temperature float64 `json:"t"`
	Humidify float64 `json:"h"`
	Vcc float64 `json:"v"`
	Memory int `json:"m"`
	Time int `json:"u"`
}

func (h *SensorHandler) ListSensor(w http.ResponseWriter, r *http.Request){
	model := SensorModel{h.db}
	json.NewEncoder(w).Encode(model.All())
}

func (h *SensorHandler)  GetSensor(w http.ResponseWriter, r *http.Request){
	params := mux.Vars(r)

	model := SensorModel{h.db}

	id,_ := strconv.Atoi(params["id"])

	json.NewEncoder(w).Encode(model.Get(id))
}

func (h *SensorHandler)  CreateSensor(w http.ResponseWriter, r *http.Request){
	var request CreateSensorRequest
	model := SensorModel{h.db}

	json.NewDecoder(r.Body).Decode(&request)

	sensor := Sensor{
		ChipID: request.Id,
		Name: "Sensor 01",
		Alias: "Sensor 01",
		Temperature: request.Temperature,
		Moisture: request.Humidify}

  fmt.Println(request)

	model.Create(&sensor)

	fmt.Println(sensor)
	json.NewEncoder(w).Encode(&sensor)
}
