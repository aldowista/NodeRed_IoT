void handleButton(){
  if (digitalRead(BUTTON_PIN) == HIGH) {
    digitalWrite(LED1_PIN, HIGH);
    publishMessage(pirSensor_topic,String(1),true);
  }
  else {
    digitalWrite(LED1_PIN, LOW);
    publishMessage(pirSensor_topic,String(0),true);
  }
}

