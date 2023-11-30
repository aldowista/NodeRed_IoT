SensorData readSensor() {
  delay(2000);
  
  SensorData data;
  data.humidity = dht.readHumidity();
  data.temperature = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(data.humidity) || isnan(data.temperature)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    data.temperature = NAN;
    data.humidity = NAN;
    return data;
  }

  // Compute and display additional sensor data
  float fahrenheit = dht.readTemperature(true);
  float heatIndexC = dht.computeHeatIndex(data.temperature, data.humidity, false);
  float heatIndexF = dht.computeHeatIndex(fahrenheit, data.humidity);

  Serial.print(F("Humidity: "));
  Serial.print(data.humidity);
  Serial.print(F("%  Temperature: "));
  Serial.print(data.temperature);
  Serial.print(F("°C "));
  Serial.print(fahrenheit);
  Serial.print(F("°F  Heat index: "));
  Serial.print(heatIndexC);
  Serial.print(F("°C "));
  Serial.print(heatIndexF);
  Serial.println(F("°F"));

  return data;
}
