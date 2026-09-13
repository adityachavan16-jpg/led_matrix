#include <WiFi.h>
#include <WebServer.h>
#include <FastLED.h>

// ===================
// LED STRIP CONFIG
// ===================
#define LED_PIN     13
#define NUM_LEDS    144
#define NUM_SEGMENTS 12
#define LEDS_PER_SEG (NUM_LEDS / NUM_SEGMENTS) // Will be 12

#define BRIGHTNESS  150
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

// "Very very dim" as requested
CRGB COLOR_BRIGHT = CRGB::White;
CRGB COLOR_DIM    = CRGB(10, 10, 10); 

// ===================
// WI-FI CREDENTIALS
// ===================
const char* ssid = "Rahul";
const char* password = "Rahullll";

WebServer server(80);

// Helper function to dim a specific segment
void dimSegment(int segmentId) {
  if (segmentId >= 0 && segmentId < NUM_SEGMENTS) {
    int startLed = segmentId * LEDS_PER_SEG;
    int endLed = startLed + LEDS_PER_SEG;
    // Loop through the 12 LEDs in this segment and dim them all
    for (int i = startLed; i < endLed; i++) {
      if (i < NUM_LEDS) { // Safety check
        leds[i] = COLOR_DIM;
      }
    }
  }
}

void handleLedControl() {
  // 1. Reset ALL LEDs to BRIGHT first
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = COLOR_BRIGHT;
  }

  // 2. Parse received segment list (e.g., "dim=4" or "dim=4,5")
  if (server.hasArg("dim")) {
    String dim_string = server.arg("dim");
    if (dim_string != "-1") {
        int last_comma = -1;
        for (int i = 0; i < dim_string.length(); i++) {
          if (dim_string.charAt(i) == ',') {
            int segment_id = dim_string.substring(last_comma + 1, i).toInt();
            dimSegment(segment_id);
            last_comma = i;
          }
        }
        // Process the last segment number after the final comma
        int segment_id = dim_string.substring(last_comma + 1).toInt();
        dimSegment(segment_id);
    }
  }

  server.send(200, "text/plain", "OK");
  FastLED.show();
}

void setup() {
  Serial.begin(115200);
  
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  fill_solid(leds, NUM_LEDS, COLOR_BRIGHT);
  FastLED.show();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nLED Controller connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/leds", HTTP_GET, handleLedControl);
  server.begin();
}

void loop() {
  server.handleClient();
}
