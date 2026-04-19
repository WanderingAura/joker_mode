#include <Arduino.h>

#include <LittleFS.h>
#include <WiFi.h>
#include <HTTPClient.h>

#include <esp_http_server.h>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

constexpr int MAX_REQ_BODY_SIZE = 512;
constexpr int MAX_RESP_BODY_SIZE = 512;
constexpr int MAX_RESP_SCORE_NUM = 10;

constexpr int SSID_MAX_LENGTH = 32;
constexpr int PASSPHRASE_MAX_LENGTH = 32;
constexpr int DUCKDNS_URL_MAX_LENGTH = 128;

constexpr int WIFI_CONNECT_TIMEOUT_MS = 10000;
constexpr int WIFI_CONNECT_WAIT_MS = 500;

constexpr int HTTP_SERVER_PORT = 80;

constexpr int MAX_NAME_LENGTH = 11;
constexpr int MAX_HISCORE_NUM = 100;

constexpr int SAVE_INTERVAL_MS = 1000 * 60 * 60 * 12; // 12 hours
constexpr int IP_UPDATE_INTERVAL_MS = 1000 * 60 * 10; // 10 minutes

#define MIN(a,b) ((a) > (b) ? (b) : (a))

struct Hiscore
{
  char name[MAX_NAME_LENGTH + 1];
  int score;
};

// TODO: make this into a real class
struct HiscoreList
{
  void Add(Hiscore hs);
  void WriteToFile();
  void ReadFromFile();
  bool Empty();
  int ToCsvString(char* buf, int maxlen);

  int current_len;
  Hiscore hiscores[MAX_HISCORE_NUM];
  SemaphoreHandle_t lock;
  bool written_to_file;
};

static HiscoreList leaderboard = {};
char ssid[SSID_MAX_LENGTH + 1];
char passphrase[PASSPHRASE_MAX_LENGTH + 1];
char duckdns_url[DUCKDNS_URL_MAX_LENGTH + 1];

// Make sure hiscore struct's size is a multiple of 2.
// Probably doesn't matter but might as well in case it does matter.
static_assert(sizeof(Hiscore) == 16);

static bool INITIALISED = false;
static httpd_handle_t server = NULL;
static esp_err_t hiscore_get_handler(httpd_req_t* req);
static esp_err_t hiscore_post_handler(httpd_req_t* req);
static esp_err_t startServer();
static int connectToWifi(char* ssid, char* passphrase);
static int readConfig(char* ssid, size_t ssid_maxlen, char* passphrase, size_t passphrase_maxlen, char* duckdns_url, size_t url_maxlen);
static void ScanAndPrintNetworks(void);
static void UpdateDuckDnsIp(char* url);

void setup() {
  Serial.begin(115200);
  Serial.println("Mounting LittleFS...");
  if (!LittleFS.begin(false))
  {
    Serial.println("Reformatting LittleFS...");
    LittleFS.format();
    if (!LittleFS.begin(false))
    {
      Serial.println("ERROR: LittleFS Mount Failed");
    }
    Serial.println("Formatting done");
  }
  Serial.println("LittleFS Mounted");

  leaderboard.lock = xSemaphoreCreateMutex();
  if (!leaderboard.lock)
  {
    Serial.println("ERROR: Failed to create mutex");
    return;
  }
  leaderboard.ReadFromFile();

#if SCAN_NETWORKS
  ScanAndPrintNetworks();
#endif

  int ret = readConfig(ssid, SSID_MAX_LENGTH, passphrase, PASSPHRASE_MAX_LENGTH, duckdns_url, DUCKDNS_URL_MAX_LENGTH);
  if (ret != 0)
  {
    Serial.println("ERROR: failed to read Wifi info!");
    return;
  }

  Serial.println(duckdns_url);

  ret = connectToWifi(ssid, passphrase);
  if (ret != 0)
  {
    Serial.println("ERROR: failed to connect to Wifi");
    return;
  }
  Serial.println("Successfully connected to WiFi!");
  Serial.print("Local IP: ");
  Serial.print(WiFi.localIP());
  Serial.println();

  if (startServer() != ESP_OK)
  {
    Serial.println("ERROR: failed to start server");
    return;
  }

  Serial.println("Server successfully started!");

  INITIALISED = true;
}

void loop() {
  if (!INITIALISED)
  {
    Serial.println("ERROR: Device failed to initialise. Please reset or reflash.");
    delay(1000);
    return;
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    int ret = connectToWifi(ssid, passphrase);
    if (ret != 0)
    {
      Serial.println("ERROR: failed to connect to Wifi");
      delay(5000);
    }
  }

  static unsigned long lastSave = 0;
  static unsigned long lastIPUpdate = 0;

  unsigned long now = millis();

  if (now - lastSave >= SAVE_INTERVAL_MS)
  {
    lastSave = now;
    // only save when there's a change
    if (!leaderboard.written_to_file)
    {
      leaderboard.written_to_file = true;
      leaderboard.WriteToFile();
    }
  }

  if (now - lastIPUpdate >= IP_UPDATE_INTERVAL_MS)
  {
    lastIPUpdate = now;
    UpdateDuckDnsIp(duckdns_url);
  }
}

static void UpdateDuckDnsIp(char* url)
{
  HTTPClient http;
  http.begin(url);

  int httpResponseCode = http.GET();

  if (httpResponseCode >= 200 && httpResponseCode < 300)
  {
    Serial.printf("Setting duckdns domain ip OK, status code: %d\n", httpResponseCode);
  }
  else
  {
    Serial.printf("Setting duckdns domain ip ERROR, status code: %d\n", httpResponseCode);
  }
  http.end();
}

static void ScanAndPrintNetworks(void)
{
  int n = WiFi.scanNetworks();
  Serial.print("Scanning networks");
  while (!WiFi.scanComplete())
  {
    Serial.print(".");
    delay(100);
  }
  Serial.println("\nScan done");

  Serial.println("Found networks:");
  for (int i = 0; i < n; i++)
  {
    Serial.println(WiFi.SSID(i));
  }
}

static int readConfig(char* ssid, size_t ssid_maxlen, char* passphrase, size_t passphrase_maxlen, char* duckdns_url, size_t url_maxlen)
{
  int ret = -1;
  size_t bytesRead;
  File file = LittleFS.open("/config.txt", "r");
  if (!file)
  {
    Serial.println("Failed to open WiFi file");
    goto close_file;
  }

  bytesRead = file.readBytesUntil('\n', ssid, ssid_maxlen);
  if (bytesRead == 0)
  {
    Serial.println("Failed to read WiFi SSID from file");
    goto close_file;
  }
  ssid[bytesRead] = 0;

  bytesRead = file.readBytesUntil('\n', passphrase, passphrase_maxlen);
  if (bytesRead == 0)
  {
    Serial.println("Failed to read WiFi password from file");
    goto close_file;
  }
  passphrase[bytesRead] = 0;

  bytesRead = file.readBytesUntil('\0', duckdns_url, url_maxlen);
  if (bytesRead == 0)
  {
    Serial.println("Failed to read duckdns url from file");
    goto close_file;
  }
  duckdns_url[bytesRead] = 0;
  ret = 0;
close_file:
  file.close();
  return ret;
}

static int connectToWifi(char* ssid, char* passphrase)
{
  WiFi.begin(ssid, passphrase);
  Serial.print("Connecting...\n");

  int time_waited_ms = 0;
  while (WiFi.status() != WL_CONNECTED && time_waited_ms <= WIFI_CONNECT_TIMEOUT_MS)
  {
    if (WiFi.status() == WL_CONNECT_FAILED)
    {
      Serial.printf("\nConnection attempt to %s failed\n", ssid);
      return -1;
    }
    delay(WIFI_CONNECT_WAIT_MS);
    Serial.printf("Not yet connected, status: %d\n", WiFi.status());

    time_waited_ms += WIFI_CONNECT_WAIT_MS;
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.printf("\nConnection attempt to %s timed out, status: %d\n", ssid, WiFi.status());
#if DEBUG
    Serial.printf("Passphrase: %s\n", passphrase);
#endif
    return -1;
  }

  return 0;
}

static esp_err_t hiscore_get_handler(httpd_req_t* req)
{
  Serial.println("Received Get request");

  if (req->content_len > 0)
  {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Did not expect body for GET request");
    return ESP_FAIL;
  }

  if (leaderboard.Empty())
  {
    httpd_resp_set_status(req, HTTPD_204);
    return httpd_resp_send(req, "No hiscores available", HTTPD_RESP_USE_STRLEN);
  }
  
  static char respBuf[MAX_RESP_BODY_SIZE];

  int bytesWritten = leaderboard.ToCsvString(respBuf, sizeof(respBuf));
  if (bytesWritten <= 0)
  {
    return ESP_FAIL;
  }

  Serial.printf("Responding with csv:\n%s", respBuf);

  httpd_resp_set_type(req, "text/csv");

  // +1 to include the null terminator
  return httpd_resp_send(req, respBuf, bytesWritten);
}


static esp_err_t hiscore_post_handler(httpd_req_t* req)
{
  if (req->content_len > MAX_REQ_BODY_SIZE)
  {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Body too large");
    return ESP_OK;
  }

  static char buf[MAX_REQ_BODY_SIZE];
  int remaining = req->content_len;
  int total_read = 0;

  while (remaining > 0)
  {
    if (total_read >= sizeof(buf))
    {
      Serial.println("Buffer is full");
      return ESP_FAIL;
    }

    int bytes_received = httpd_req_recv(req, buf + total_read, sizeof(buf) - total_read);
    if (bytes_received <= 0)
    {
      Serial.println("Failed to receive request bytes");
      return ESP_FAIL;
    }

    total_read += bytes_received;
    remaining -= bytes_received;
  }

  if (total_read <= 0)
  {
    Serial.println("Failed to read response body!");
    return ESP_FAIL;
  }

  String body(buf, total_read);

  int commaIdx = body.indexOf(',');

  String name = body.substring(0, commaIdx);
  if (name.length() > MAX_NAME_LENGTH)
  {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "hiscore name must be 11 character or less");
    return ESP_OK;
  }

  Serial.printf("got name: %s\n", name);

  int score = atoi(&body[commaIdx+1]);

  Serial.printf("got score: %d\n", score);

  Hiscore hs = {};
  strncpy(hs.name, name.c_str(), MAX_NAME_LENGTH);
  hs.score = score;
  leaderboard.Add(hs);
  httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

static esp_err_t startServer()
{
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();

  config.recv_wait_timeout = 1;
  config.max_open_sockets = 2;
  config.lru_purge_enable = true;

  if (httpd_start(&server, &config) == ESP_OK)
  {
    httpd_uri_t uri =
    {
      .uri = "/hiscores",
      .method = HTTP_POST,
      .handler = hiscore_post_handler,
      .user_ctx = NULL
    };

    esp_err_t err = httpd_register_uri_handler(server, &uri);
    if (err != ESP_OK)
    {
      return err;
    }
    uri.method = HTTP_GET;
    uri.handler = hiscore_get_handler;
    return httpd_register_uri_handler(server, &uri);
  }
  else
  {
    return ESP_FAIL;
  }
}

void HiscoreList::Add(Hiscore hs)
{
  xSemaphoreTake(lock, portMAX_DELAY);
  Hiscore overwritten_hiscore = hs;
  bool written_score = false;
  int cur_pos = 0;
  for (; cur_pos < current_len; cur_pos++)
  {
    if (!written_score)
    {
      if (hiscores[cur_pos].score > hs.score)
      {
        continue;
      }
      else
      {
        // write new hiscore + move all others down (insertion sort)
        written_score = true;
        overwritten_hiscore = hiscores[cur_pos];
        hiscores[cur_pos] = hs;
      }
    }
    else
    {
      // write in the previous overwritten score
      Hiscore temp = overwritten_hiscore;
      overwritten_hiscore = hiscores[cur_pos];
      hiscores[cur_pos] = temp;
    }
  }

  // add hiscore onto the end if not written yet (but only if the leaderboard isn't full)
  // or if written, add the last score onto the end
  if (current_len < MAX_HISCORE_NUM)
  {
    hiscores[current_len++] = overwritten_hiscore;
  }

  written_to_file = false;
  xSemaphoreGive(lock);
}

void HiscoreList::ReadFromFile()
{
  File file = LittleFS.open("/hiscores.bin", "r");

  if (!file.available())
  {
    Serial.println("It seems the hiscores file is empty, skipping load");
    return;
  }

  uint8_t* buf = (uint8_t*)&current_len;
  int len = file.read(buf, sizeof(current_len));
  buf = (uint8_t*)hiscores;
  int hiscoreBytesExpected = sizeof(Hiscore) * current_len;
  len += file.read(buf, sizeof(Hiscore) * current_len);
  file.close();

  if (len != hiscoreBytesExpected + sizeof(current_len))
  {
    Serial.printf("Only managed to read %d bytes from file, expected %d\n", len, sizeof(hiscores));
    return;
  }

  char printbuf[MAX_RESP_BODY_SIZE];

  int bytes = ToCsvString(printbuf, MAX_RESP_BODY_SIZE);
  if (bytes <= 0)
  {
    Serial.printf("ToCsvString failed: %d bytes\n", bytes);
    return;
  }

  written_to_file = true;
  Serial.println("Loaded hiscores from file:");
  Serial.write(printbuf, bytes);
  Serial.println();
}

int HiscoreList::ToCsvString(char* buf, int maxlen)
{
  int bytesWritten = 0;
  
  for (int i = 0; i < MIN(leaderboard.current_len, MAX_RESP_SCORE_NUM); i++)
  {
    Hiscore cur_score = leaderboard.hiscores[i];
    int len = snprintf(buf + bytesWritten, MAX_RESP_BODY_SIZE - bytesWritten,  "%s,%d\n", cur_score.name, cur_score.score);

    if (len >= MAX_RESP_BODY_SIZE - bytesWritten)
    {
      Serial.println("response too large, sprintf truncated");
      return -1;
    }

    bytesWritten += len;
  }
  return bytesWritten;
}

bool HiscoreList::Empty()
{
  return current_len == 0;
}

void HiscoreList::WriteToFile()
{
  File file = LittleFS.open("/hiscores.tmp", "w");

  xSemaphoreTake(lock, portMAX_DELAY);

  int hiscoreBytesToWrite = current_len * sizeof(Hiscore);

  uint8_t* hsBuf = (uint8_t*)hiscores;
  uint8_t* currentLenBuf = (uint8_t*)&current_len;

  size_t len = file.write(currentLenBuf, sizeof(current_len));
  len += file.write(hsBuf, hiscoreBytesToWrite);

  xSemaphoreGive(lock);
  file.close();

  if (len != hiscoreBytesToWrite + sizeof(current_len))
  {
    Serial.printf("Failed to write all bytes to file, wrote %d expected %d\n", len, sizeof(hiscores));
    return;
  }

  // doing this helps prevent data corruption
  if (!LittleFS.rename("/hiscores.tmp", "/hiscores.bin"))
  {
    Serial.println("File rename failed");
    return;
  }

  Serial.println("Saved hiscore data to file!");

  ReadFromFile();
}