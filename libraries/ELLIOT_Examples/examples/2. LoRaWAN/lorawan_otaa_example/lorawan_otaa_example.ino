#include <RFThings.h>
#include <rfthings_sx126x.h>

#define TX_INTERVAL 10

#define LED 7

static uint8_t app_eui[] =  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static uint8_t app_key[] =  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static uint8_t dev_eui[] =  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


uint8_t* dev_addr;
uint8_t* nwkS_key;
uint8_t* appS_key;

// nss_pin, rst_pin, busy_pin, dio1_pin, antenna_switch_pin
rfthings_sx126x sx126x(E22_NSS, E22_NRST, E22_BUSY, E22_DIO1, E22_RXEN);
rft_status_t status;
bool join_accept = false;

char payload[255];
uint32_t payload_len;

String message = "hello world";

void setup() {
  analogReadResolution(12);

  Serial.begin(115200);

  while (!Serial && millis() < 3000);

  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  // Init sx126x
  Serial.println("#### SX126X INITIALIZE ####");
  status = sx126x.init(RFT_REGION_EU863_870);
  Serial.print("sx126x init: ");
  Serial.println(rft_status_to_str(status));

  // Random Seed
  int rSeed = analogRead(15) * 1000; // PA5
  randomSeed(rSeed);
  Serial.print("Random Seed value = ");
  Serial.println(rSeed);

  // set up LoRaWAN key (ABP by default)
  sx126x.set_lorawan_activation_type(RFT_LORAWAN_ACTIVATION_TYPE_OTAA);
  sx126x.set_devive_eui(dev_eui);
  sx126x.set_application_eui(app_eui);
  sx126x.set_application_key(app_key);
  sx126x.set_rx1_delay(5000);

  sx126x.set_frequency(868100000);
  sx126x.set_tx_power(21);
  sx126x.set_spreading_factor(RFT_LORA_SPREADING_FACTOR_10);
  sx126x.set_bandwidth(RFT_LORA_BANDWIDTH_125KHZ);
  sx126x.set_coding_rate(RFT_LORA_CODING_RATE_4_5);
  sx126x.set_syncword(RFT_LORA_SYNCWORD_PUBLIC);
}

void loop() {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  delay(100);
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  delay(100);

  if (!join_accept) {
    Serial.println("Sending join request: ");
    status = sx126x.send_join_request(NULL, NULL);

    if (status == RFT_STATUS_OK) {
      dev_addr = sx126x.get_device_address();
      nwkS_key = sx126x.get_network_session_key();
      appS_key = sx126x.get_application_session_key();

      Serial.println("Join accept!");
      join_accept = true;

      Serial.print("Device address: ");
      for (int i = 0; i < 4; i++) {
        Serial.print(dev_addr[i], HEX);
        Serial.print(" ");
      }
      Serial.println();

      Serial.print("Network session key: ");
      for (int i = 0; i < 16; i++) {
        Serial.print(nwkS_key[i], HEX);
        Serial.print(" ");
      }
      Serial.println();

      Serial.print("Application session address: ");
      for (int i = 0; i < 16; i++) {
        Serial.print(appS_key[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
    } else {
      Serial.println(rft_status_to_str(status));
    }
  } else {
    Serial.print("Sending LoRaWAN message: ");

    build_payload();
    status = sx126x.send_uplink((byte*)payload, payload_len, NULL, NULL);

    if (status == RFT_STATUS_OK) {
      Serial.println("receive downlink packet");
      Serial.print("    RSSI: ");
      Serial.println(sx126x.get_rssi());
      Serial.print("    SNR: ");
      Serial.println(sx126x.get_snr());
      Serial.print("    Signal rssi: ");
      Serial.println(sx126x.get_signal_rssi());

      Serial.print("Downlink payload: ");
      for (int i = 0; i < sx126x.get_rx_length(); i++) {
        Serial.print(payload[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
    } else {
      Serial.println(rft_status_to_str(status));
    }
  }

  delay(TX_INTERVAL * 1000);
}

void build_payload(void) {
  message.toCharArray(payload, 255);
  payload_len = message.length();
}
