#include <Arduino.h>
#include <ArduinoJson.h>
#include <pinmanager.h>
#include <comms.h>
#include <esp_http_server.h>
#include <string>
#include <esp_ota_ops.h>
#include <esp_https_ota.h>
static const char *TAG = "Comms";
httpd_handle_t server = NULL;

void runComms()
{
	ESP_LOGI(TAG, "Initializing web server... (this is the new app)");
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();

	ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
	ESP_ERROR_CHECK(httpd_start(&server, &config));
	ESP_LOGI(TAG, "Registering URI handlers");
	static const httpd_uri_t ws = {
		.uri = "/ws",
		.method = HTTP_GET,
		.handler = websocket_handler,
		.user_ctx = NULL,
		.is_websocket = true};
	static const httpd_uri_t ota_post = {
		.uri = "/ota",
		.method = HTTP_POST,
		.handler = ota_post_handler,
		.user_ctx = NULL};
	httpd_register_uri_handler(server, &ws);
	httpd_register_uri_handler(server, &ota_post);
	ESP_LOGI(TAG, "Web server init done!");
}

esp_err_t websocket_handler(httpd_req_t *req)
{
	if (req->method == HTTP_GET)
	{
		ESP_LOGI(TAG, "Client connected!");
		JsonDocument message;
		for (size_t i = 0; i < 10; i++)
		{
			message[std::to_string(i)] = on[i];
		}
		std::string msg;
		serializeJson(message, msg);

		httpd_ws_frame_t ws_pkt;
		ws_pkt.payload = (uint8_t *)msg.c_str();
		ws_pkt.len = strlen(msg.c_str());
		ws_pkt.type = HTTPD_WS_TYPE_TEXT;
		httpd_ws_send_frame(req, &ws_pkt);
		return ESP_OK;
	}
	httpd_ws_frame_t ws_pkt;
	uint8_t *buf = NULL;
	memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
	ws_pkt.type = HTTPD_WS_TYPE_TEXT;
	ESP_ERROR_CHECK(httpd_ws_recv_frame(req, &ws_pkt, 0));
	if (ws_pkt.len)
	{
		buf = (uint8_t *)calloc(1, ws_pkt.len + 1);
		if (buf == NULL)
		{
			return ESP_ERR_NO_MEM;
		}
		ws_pkt.payload = buf;
		ESP_ERROR_CHECK(httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len));
		ESP_LOGI(TAG, "Recieved %s", ws_pkt.payload);
		if (ws_pkt.type == HTTPD_WS_TYPE_TEXT)
		{
			JsonDocument message;
			deserializeJson(message, ws_pkt.payload);
			for (size_t i = 0; i < 10; i++)
			{
				if (message.containsKey(String(i)))
					setPinState(i, message[String(i)]);
			}
		}
	}
	free(buf);
	return ESP_OK;
}

esp_err_t ota_post_handler(httpd_req_t *req)
{
	char buf[256];
	httpd_resp_set_status(req, HTTPD_500); // Assume failure

	int ret, remaining = req->content_len;
	ESP_LOGI(TAG, "Getting OTA update... (this is the new app)");

	esp_ota_handle_t update_handle = 0;
	const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);
	const esp_partition_t *running = esp_ota_get_running_partition();

	if (update_partition == NULL)
	{
		ESP_LOGE(TAG, "No ota partition!");
		esp_ota_abort(update_handle);
		httpd_resp_send(req, NULL, 0);
		return ESP_FAIL;
	}

	ESP_LOGI(TAG, "Writing partition: type %d, subtype %d, offset 0x%08x", update_partition->type, update_partition->subtype, update_partition->address);
	ESP_LOGI(TAG, "Running partition: type %d, subtype %d, offset 0x%08x", running->type, running->subtype, running->address);
	ESP_ERROR_CHECK(esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES, &update_handle));
	while (remaining > 0)
	{
		if ((ret = httpd_req_recv(req, buf, min((unsigned int)remaining, sizeof(buf)))) <= 0)
		{
			if (ret == HTTPD_SOCK_ERR_TIMEOUT)
			{
				continue;
			}
			esp_ota_abort(update_handle);
			httpd_resp_send(req, NULL, 0);
			return ESP_FAIL;
		}
		size_t bytes_read = ret;

		remaining -= bytes_read;
		ESP_LOGI(TAG, "Recieving OTA: %i", remaining);
		ESP_ERROR_CHECK(esp_ota_write(update_handle, buf, bytes_read));
	}

	ESP_LOGI(TAG, "Gettting OTA update done");
	ESP_ERROR_CHECK(esp_ota_end(update_handle));
	ESP_ERROR_CHECK(esp_ota_set_boot_partition(update_partition));
	ESP_LOGI(TAG, "OTA Success! Rebooting...");

	httpd_resp_set_status(req, HTTPD_200);
	httpd_resp_send(req, NULL, 0);

	vTaskDelay(2000 / portTICK_RATE_MS);
	esp_restart();

	return ESP_OK;
}

void textAll(JsonDocument message)
{
	if (server == NULL)
	{
		ESP_LOGE(TAG, "Tried to send message before server initialized.");
		return;
	}
	std::string msg;
	serializeJson(message, msg);
	ESP_LOGI(TAG, "Sending %s...\n", msg.c_str());

	httpd_ws_frame_t ws_pkt;
	ws_pkt.payload = (uint8_t *)msg.c_str();
	ws_pkt.len = strlen(msg.c_str());
	ws_pkt.type = HTTPD_WS_TYPE_TEXT;

	size_t fds = CONFIG_LWIP_MAX_LISTENING_TCP;
	int client_fds[CONFIG_LWIP_MAX_LISTENING_TCP] = {0};

	ESP_ERROR_CHECK(httpd_get_client_list(server, &fds, client_fds));
	for (int i = 0; i < fds; i++)
	{
		httpd_ws_client_info_t client_info = httpd_ws_get_fd_info(server, client_fds[i]);
		if (client_info == HTTPD_WS_CLIENT_WEBSOCKET)
		{
			httpd_ws_send_frame_async(server, client_fds[i], &ws_pkt);
		}
	}
}