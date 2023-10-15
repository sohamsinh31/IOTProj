
SOHAMSINH BORASIA 1 : 39 PM(0 minutes ago)
    to me

#define BLYNK_TEMPLATE_ID "TMPL38p9t6viN"
#define BLYNK_TEMPLATE_NAME "Quickstart Device"
#define BLYNK_AUTH_TOKEN "NI0DDioO687PuDHcIHiwfBZFDETMHjeN"

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <TimeLib.h> // Include a library for working with time (e.g., TimeLib)
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ThingSpeak.h>
#include <BlynkSimpleEsp32.h>

    const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 0;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, gmtOffset_sec, daylightOffset_sec);

char auth[] = "NI0DDioO687PuDHcIHiwfBZFDETMHjeN";
char ssid[] = "SOHAM2";   // Your WiFi SSID
char pass[] = "12345678"; // Your WiFi password
const unsigned long channelID = 2304789;
const char *writeAPIKey = "5NTQLCT3MVCPPDWX";

BlynkTimer timer;
WiFiClient client;

constexpr uint8_t RST_PIN = 22; // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = 21;  // Configurable, see typical pin layout above
constexpr uint8_t LED_PIN = 2;  // LED connected to pin D2

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;

String tag;
// Define a list of tag IDs and corresponding enrollments
String tags[] = {"23218916713", "1282421138"};
int enrollments[] = {9, 11}; // Enrollment numbers corresponding to tags

void setup()
{
    Serial.begin(9600);
    Blynk.begin(auth, ssid, pass);
    SPI.begin(18, 19, 23, 21); // SCK, MISO, MOSI, SS
    pinMode(LED_PIN, OUTPUT);  // Set LED pin as an output
    rfid.PCD_Init();           // Init MFRC522
    WiFi.begin(ssid, pass);

    // Wait for the WiFi connection to be established
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    ThingSpeak.begin(client);
}

int getYear(NTPClient &ntpClient)
{
    time_t rawtime = ntpClient.getEpochTime();
    struct tm *ti;
    ti = localtime(&rawtime);
    int year = ti->tm_year + 1900;

    return year;
}

int getMonth(NTPClient &ntpClient)
{
    time_t rawtime = ntpClient.getEpochTime();
    struct tm *ti;
    ti = localtime(&rawtime);
    int month = (ti->tm_mon + 1) < 10 ? 0 + (ti->tm_mon + 1) : (ti->tm_mon + 1);

    return month;
}

int getDay(NTPClient &ntpClient)
{
    time_t rawtime = ntpClient.getEpochTime();
    struct tm *ti;
    ti = localtime(&rawtime);
    int day = (ti->tm_mday) < 10 ? 0 + (ti->tm_mday) : (ti->tm_mday);

    return day;
}

void loop()
{
    Blynk.run();
    if (!rfid.PICC_IsNewCardPresent())
        return;
    if (rfid.PICC_ReadCardSerial())
    {
        for (byte i = 0; i < 4; i++)
        {
            tag += rfid.uid.uidByte[i];
        }
        Serial.println(tag);

        // Check if the read tag is in the list of tags
        int matchedEnrollment = getEnrollmentForTag(tag);
        if (matchedEnrollment != -1)
        {
            digitalWrite(LED_PIN, HIGH); // Turn on the LED
            delay(1000);                 // Keep the LED on for 1 second
            digitalWrite(LED_PIN, LOW);  // Turn off the LED

            timeClient.update();

            // // Get the current date and time
            String currentDate = String(getYear(timeClient)) + "-" + String(getMonth(timeClient)) + "-" + String(getDay(timeClient));
            String currentTime = timeClient.getFormattedTime(); // Use a real-time clock library to get the time

            // Calculate end1 and end2
            String end1 = currentDate;
            String end2 = addMinutes(currentTime, 50); // Add 50 minutes to the current time

            // Create a JSON object with all the data
            String tagData = createTagData(tag, matchedEnrollment, currentDate, currentTime, end1, end2);

            Blynk.virtualWrite(V4, tagData);
            Blynk.virtualWrite(V5, matchedEnrollment);

            int fieldNumber = 1; // Replace with the desired field number
            int entryId = -1;    // Use -1 to create a new entry each time

            // Send the data to ThingSpeak with entryId
            long currentEntry = ThingSpeak.readLongField(channelID, fieldNumber, writeAPIKey);

            // Increment the entry counter for the new entry
            currentEntry++;

            // Set the data value for your field
            ThingSpeak.setField(fieldNumber, currentEntry);

            // Send the tagData to ThingSpeak by setting another field (e.g., Field 2)
            int tagDataField = 2; // Use Field 2 for tag data
            ThingSpeak.setField(tagDataField, tagData);

            // Send the data to ThingSpeak
            int status = ThingSpeak.writeFields(channelID, writeAPIKey);

            if (status == 200)
            {
                Serial.println("Data sent to ThingSpeak successfully");
            }
            else
            {
                Serial.println("Failed to send data to ThingSpeak");
            }

            tag = "";

            rfid.PICC_HaltA();
            rfid.PCD_StopCrypto1();
        }
    }
}

int getEnrollmentForTag(String readTag)
{
    // Get the enrollment number for the read tag
    for (int i = 0; i < sizeof(tags) / sizeof(tags[0]); i++)
    {
        if (readTag == tags[i])
        {
            return enrollments[i]; // Return the matching enrollment number
        }
    }
    return -1; // Return -1 if tag is not found
}

String createTagData(String tag, int enrollment, String start1, String start2, String end1, String end2)
{
    // Create a JSON object with all the data
    DynamicJsonDocument jsonDoc(256);
    JsonObject jsonData = jsonDoc.to<JsonObject>();
    jsonData["tag"] = tag;
    jsonData["enrollment"] = enrollment;
    jsonData["start1"] = start1;
    jsonData["start2"] = start2;
    jsonData["end1"] = end1;
    jsonData["end2"] = end2;
    jsonData["degree"] = "BE/BTECH";
    jsonData["branch"] = "CS";
    jsonData["sem"] = 1;
    jsonData["sub"] = "SESH1080";

    // Serialize the JSON object to a string
    String jsonString;
    serializeJsonPretty(jsonData, jsonString);

    return jsonString;
}

String addMinutes(String currentTime, int minutesToAdd)
{
    int currentHour = currentTime.substring(0, 2).toInt();
    int currentMinute = currentTime.substring(3, 5).toInt();

    currentMinute += minutesToAdd;
    currentHour += currentMinute / 60;
    currentMinute %= 60;
    currentHour %= 24;

    // Format the time back to a string (e.g., "HH:mm")
    String formattedHour = currentHour < 10 ? "0" + String(currentHour) : String(currentHour);
    String formattedMinute = currentMinute < 10 ? "0" + String(currentMinute) : String(currentMinute);

    return formattedHour + ":" + formattedMinute;
}