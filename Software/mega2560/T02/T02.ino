String inputString = "";     // a String to hold incoming data
bool stringComplete = false; // whether the string is complete

void setup() {
    // initialize serial:
    Serial.begin(115200);
}

void loop() {

    if (stringComplete) {
        Serial.println(inputString);
        // clear the string:
        inputString = "";
        stringComplete = false;
    }
    delay(100);
}

void serialEvent() {
    while (Serial.available()) {
        // get the new byte:
        char inChar = (char)Serial.read();
        // add it to the inputString:
        inputString += inChar;
        // if the incoming character is a newline, set a flag so the main loop
        // can do something about it:
        if (inChar == '\n') {
            stringComplete = true;
        }
    }
}