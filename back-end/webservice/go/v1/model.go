package main

import (
	// "fmt"
	"database/sql"
)

type Sensor struct {
	Id int64
	ChipID int
	Name string
	Alias string
	Temperature float64
	Moisture float64
}

type Sensors []Sensor

type SensorModel struct {
	db *sql.DB
}

func (m *SensorModel) All() Sensors {
	rows, err := m.db.Query("SELECT id, chip_id, name, alias, temperature, moisture FROM sensors ORDER BY id DESC");

	if err != nil {
		panic(err.Error())
	}

	sensors := Sensors{}

	for rows.Next() {
		var sensor Sensor

		err = rows.Scan(&sensor.Id, &sensor.ChipID ,&sensor.Name ,&sensor.Alias ,&sensor.Temperature, &sensor.Moisture)
		if err != nil {
			panic(err.Error())
		}

		sensors = append(sensors, sensor)
	}

	return sensors
}

func (m *SensorModel) Get(id int) *Sensor{
	stmt, err := m.db.Prepare("SELECT id, chip_id, name, alias, temperature, moisture FROM sensors WHERE id = ?");

	if err != nil {
		panic("QUERY: " + err.Error())
	}

	sensor :=  Sensor{}

	err = stmt.QueryRow(id).Scan(&sensor.Id, &sensor.ChipID ,&sensor.Name ,&sensor.Alias ,&sensor.Temperature, &sensor.Moisture)

	if err == sql.ErrNoRows{
		return nil
	}
	
	if err != nil {
		panic("RESULT: " + err.Error())
	}

	return &sensor
}

func (m *SensorModel) Create(sensor *Sensor) *Sensor{
	stmt, err := m.db.Prepare("INSERT INTO sensors (chip_id, name, alias, temperature, moisture) VALUES (?,?,?,?,?)");

	if err != nil {
		panic("QUERY: " + err.Error())
	}

	res, err := stmt.Exec(sensor.ChipID ,sensor.Name ,sensor.Alias ,sensor.Temperature, sensor.Moisture)
	
	if err != nil {
		panic("RESULT: " + err.Error())
	}

	rows, err := res.RowsAffected()

	if(rows != 1){
		return nil
	}

	id, _ := res.LastInsertId()

	sensor.Id = id

	return sensor
}