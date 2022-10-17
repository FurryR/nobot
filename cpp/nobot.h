#ifndef NOBOT_H
#define NOBOT_H

#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include <map>
#include <stdexcept>
#include <string>

typedef class Bot {
  int server;
  std::string buffer;
  std::pair<std::string, uint16_t> api_ip;
  /**
   * 尝试处理事件。
   * @param fd 客户端的连接。
   * @returns 如果获取事件成功，则返回事件的报文，否则返回空字符串。
   */
  std::string process_ev(int fd) {
    size_t idx = buffer.find_first_of('{');
    if (idx != std::string::npos && buffer.size() > 0 &&
        buffer[buffer.size() - 1] == '\n') {
      std::string ret = buffer.substr(idx, buffer.length() - idx - 1);
      buffer.clear();
      if (send(fd, "HTTP/1.1 200 OK\r\nConnection: Close\r\n\r\n",
               39, 0) == -1) {
        ::close(fd);
        throw std::runtime_error("send failed");
      }
      ::close(fd);
      return ret;
    } else {
      return "";
    }
  }

 public:
  /**
   * 停止服务器。
   */
  void close() noexcept {
    if (server == -1) return;
    ::close(server);
    server = -1;
  }
  /**
   * 访问API。
   * @param path 访问路径，包含参数。需编码。
   * @param action 操作。默认为"GET"。
   * @param payload 请求体内容。默认为b""。需编码。
   * @param headers 请求头部。默认为{"Content-Type": "application-json"}。
   */
  std::string api(const std::string& path, const std::string& action = "GET",
                  const std::string& payload = "",
                  const std::map<std::string, std::string>& headers = {
                      {"Content-Type", "application/json"}}) const {
    int fd;
    sockaddr_in sin{};
    std::string arg;
    for (std::map<std::string, std::string>::const_iterator it =
             headers.cbegin();
         it != headers.cend(); it++) {
      arg += it->first + ": " + it->second + "\r\n";
    }
    std::string content =
        action + " " + path + " HTTP/1.1\r\nHost: " + api_ip.first + "\r\nConnection: Close\r\n" +
        arg.substr(0, arg.length() - 2) + "\r\n\r\n" + payload;
    if ((fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
      throw std::runtime_error("socket failed");
    }
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = inet_addr(api_ip.first.c_str());
    sin.sin_port = htons(api_ip.second);
    if (connect(fd, (sockaddr*)&sin, sizeof(sockaddr_in)) < 0) {
      ::close(fd);
      throw std::runtime_error("connect failed");
    }
    if (send(fd, content.c_str(), content.length(), 0) == -1) {
      ::close(fd);
      throw std::runtime_error("send failed");
    }
    int s;
    char buf[1024];
    std::string ret;
    while ((s = recv(fd, buf, 1024, 0)) > 0) ret += std::string(buf, 0, s);
    ::close(fd);
    return ret;
  }
  /**
   * 获得一个事件。注意：如果已经close，则返回空字节串。
   * @returns 事件内容。
   */
  std::string receive() {
    if (server == -1) return "";
    socklen_t revsize = sizeof(sockaddr_in);
    sockaddr_in rev;
    int fd;
    char buf[1024];
    ssize_t s;
    if ((fd = accept(server, (sockaddr*)&rev, &revsize)) < 0) {
      throw std::runtime_error("accept failed");
    }
    while ((s = recv(fd, buf, 1024, 0)) > 0) {
      buffer += std::string(buf, 0, s);
      std::string ret = process_ev(fd);
      if (ret != "") {
        return ret;
      }
    }
    return "";
  }
  /**
   * 创建一个新的Bot对象。
   * @param server_ip 监听反向http的IP和端口。
   * @param api_ip 发送API请求的IP和端口。
   * @param max_conn 最大连接数。 默认为100。
   */
  Bot(const std::pair<std::string, uint16_t>& server_ip,
      const std::pair<std::string, uint16_t>& api_ip, int max_conn = 100)
      : buffer(""), api_ip(api_ip) {
    sockaddr_in sin{};
    if ((server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
      throw std::runtime_error("socket failed");
    static const int opt = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = inet_addr(server_ip.first.c_str());
    sin.sin_port = htons(server_ip.second);
    if (bind(server, (sockaddr*)&sin, sizeof(sin)) < 0) {
      close();
      throw std::runtime_error("bind failed");
    }
    if (listen(server, max_conn) < 0) {
      close();
      throw std::runtime_error("listen failed");
    }
  }
  ~Bot() { close(); }
} Bot;
#endif
