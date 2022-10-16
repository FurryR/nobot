# nobot-python - 比voidbot还要狠的OneBot对接框架

> 你有一个性能很高的Bot框架，但是你不知道它在哪。

通过以下方式来获得bot的事件：
```python
import json
from nobot import Bot


bot = Bot(("127.0.0.1", 5701), ("127.0.0.1", 5700))
for ev in bot:
    print(json.loads(ev))
```

通过以下方式发送消息：
```python
from typing import Union, Literal
from urllib.parse import urlencode
from nobot import Bot


def send_msg(
    mode: Union[Literal["group"], Literal["private"]], id: int, msg: str
) -> str:
    """发送消息。

    Args:
        mode (Union[Literal["group"], Literal["private"]]): group表示发送群聊消息，private表示发送私聊消息。
        id (int): 目标的ID。
        msg (str): 要发送的消息。

    Returns:
        str: 生成的请求，可以用于bot.api。
    """
    return (
        f"/send_private_msg?" + urlencode({"user_id": id, "message": msg})
        if mode == "private"
        else f"/send_group_msg?" + urlencode({"group_id": id, "message": msg})
    )


bot = Bot(("127.0.0.1", 5701), ("127.0.0.1", 5700))
bot.api(send_msg("group", group, "你好！"))
```
