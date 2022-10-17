# nobot-cpp - 比 voidbot 还要狠的 OneBot 对接框架

> 你有一个性能很高的 Bot 框架，但是你不知道它在哪。

通过以下方式来获得 bot 的事件：

```cpp
#include <iostream>

#include "nobot.h"
int main() {
  Bot bot = Bot({"127.0.0.1", 5701}, {"127.0.0.1", 5700});
  std::string ev;
  while ((ev = bot.receive()) != "") {
    std::cout << ev << std::endl;
  }
}
```

通过以下方式发送消息：

```cpp
#include <string>

#include "nobot.h"

std::string send_msg(bool isgroup, int id, const std::string& msg) {
  return (!isgroup) ? ("/send_private_msg?user_id=" + std::to_string(id) +
                       "&message=" + msg)
                    : ("/send_group_msg?group_id=" + std::to_string(id) +
                       "&message=" + msg);
}
int main() {
  Bot bot = Bot({"127.0.0.1", 5701}, {"127.0.0.1", 5700});
  bot.api(send_msg(true, group, "Hello"));
}
```
