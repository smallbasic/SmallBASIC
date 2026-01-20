// This file is part of SmallBASIC
//
// Copyright(C) 2001-2025 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <config.h>

#include <microhttpd.h>
#include <curl/curl.h>
#include <cstring>
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <unordered_set>
#include <algorithm>

constexpr int BUFFER_SIZE = 128;
static const char *g_path = nullptr;
static const char *g_host = nullptr;

using namespace std;

void log(const char *format, ...);

// headers to skip
const unordered_set<string> SKIP_HEADERS = {
  "host",                // curl sets this from the URL
  "content-length",      // curl calculates this
  "transfer-encoding",   // curl handles this
  "connection",          // hop-by-hop header
  "keep-alive",          // hop-by-hop header
  "proxy-connection",    // hop-by-hop header
  "upgrade",             // hop-by-hop header
  "te",                  // hop-by-hop header except "trailers"
  "trailer"              // hop-by-hop header
};

// handles the response from libcurl.
static size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata) {
  size_t total_size = size * nmemb;
  auto *str = static_cast<string *>(userdata);
  str->append(static_cast<char *>(ptr), total_size);
  return total_size;
}

// read headers from the microhttpd request to forward with the curl proxy
static MHD_Result get_headers(void *cls, enum MHD_ValueKind kind, const char *key, const char *value) {
  if (key && value) {
    // lowercase for comparison
    string key_lower(key);
    std::transform(key_lower.begin(), key_lower.end(), key_lower.begin(), ::tolower);
    if (!SKIP_HEADERS.count(key_lower)) {
      vector<string>* headers = static_cast<vector<string> *>(cls);
      headers->push_back(string(key) + ": " + value);
    }
  }
  return MHD_YES;
}

// reads the microhttpd request URL args
static MHD_Result get_url_arg(void *cls, enum MHD_ValueKind kind, const char *key, const char *value) {
  auto *args = static_cast<string*>(cls);
  if (key && value) {
    if (!(*args).empty()) {
      *args += "&";
    }
    *args += key;
    *args += "=";
    *args += value;
  }
  return MHD_YES;
}

// copy the curl proxy cookies back to the microhttpd response
static void set_cookies(MHD_Response *response, curl_slist *cookies) {
  // assumes cookies use the netscape 7 field tab separated format
  struct curl_slist *cookie = cookies;
  while (cookie) {
    string cookie_line(cookie->data);
    string formatted;
    stringstream ss(cookie_line);
    string part;
    string cookie_str;
    int field = 0;
    while (getline(ss, part, '\t')) {
      if (field == 5) {
        cookie_str = part;
      } else if (field == 6) {
        cookie_str.append("=").append(part);
        MHD_add_response_header(response, "Set-Cookie", cookie_str.c_str());
        break;
      }
      field++;
    }
    cookie = cookie->next;
  }
}

void proxy_init(const char *path, const char *host) {
  g_path = path;
  g_host = host;
  if (g_path && g_host) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
  }
}

void proxy_cleanup() {
  if (g_path && g_host) {
    curl_global_cleanup();
  }
}

bool proxy_accept(MHD_Connection *connection, const char *path) {
  return path && g_path && g_host && strncmp(g_path, path, strlen(g_path)) == 0;
}

MHD_Response *proxy_request(MHD_Connection *connection, const char *path, const char *method, string &body) {
  auto curl = curl_easy_init();

  if (!curl) {
    log("proxy init failed");
    return nullptr;
  }

  vector<string> headers;
  MHD_get_connection_values(connection, MHD_HEADER_KIND, &get_headers, &headers);

  curl_slist* curl_headers = nullptr;
  for (const auto &h : headers) {
    curl_headers = curl_slist_append(curl_headers, h.c_str());
  }

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_headers);

  // If it's a POST request, add the body
  if (!body.empty()) {
    log("post [%d] bytes", body.length());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body.length());
  }

  string url;
  url.append(g_host).append("/").append(path);

  string url_args;
  MHD_get_connection_values(connection, MHD_GET_ARGUMENT_KIND, get_url_arg, &url_args);
  if (!url_args.empty()) {
    url.append("?").append(url_args);
  }

  string response;
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method);
  curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
  curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "");

  log("Proxy to [%s]", url.c_str());
  auto res = curl_easy_perform(curl);
  curl_slist *receiveCookies = nullptr;
  curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &receiveCookies);
  curl_easy_cleanup(curl);

  MHD_Response *result;
  if (res != CURLE_OK) {
    log("proxy failed: [%s]", curl_easy_strerror(res));
    result = nullptr;
  } else {
    result = MHD_create_response_from_buffer(response.length(), (void *)response.c_str(), MHD_RESPMEM_MUST_COPY);
    set_cookies(result, receiveCookies);
  }

  if (curl_headers) {
    curl_slist_free_all(curl_headers);
  }
 
  if (receiveCookies) {
    curl_slist_free_all(receiveCookies);
  }
  return result;
}

