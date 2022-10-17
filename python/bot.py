# coding=utf-8
import socket


class Bot:
    __server: socket.socket
    __buffer: bytes
    __api: tuple[str, str]

    def __process_ev(self, conn: socket.socket) -> bytes:
        """尝试处理事件。

        Args:
            conn (socket.socket): 客户端的连接。

        Returns:
            bytes: 如果获取事件成功，则返回事件的报文，否则返回空字符串。
        """
        idx = self.__buffer.find(b"{")  # 判断字符是否为 payload 开头
        if idx != -1 and self.__buffer.endswith(b"\n"):  # 判断报文是否完整
            ev = self.__buffer[idx:-1]  # 临时存放缓冲区
            self.__buffer = b""  # 清空缓冲区
            conn.sendall(
                b"HTTP/1.1 200 OK\r\nConnection: Close\r\nContent-Type: text/html\r\n\r\n"
            )  # 发送应答报文
            conn.close()  # 关闭连接
            return ev  # 返回事件内容
        else:
            return b''  # 解析失败，返回空字符串

    def api(
        self,
        path: str,
        action: str = "GET",
        payload: bytes = b"",
        headers: dict[str, str] = {"Content-Type": "application/json"},
    ) -> bytes:
        """访问API。

        Args:
            path (str): 访问路径，包含参数。需编码。
            action (str, optional): 操作。默认为"GET"。
            payload (bytes, optional): 请求体内容。默认为b""。需编码。
            headers (dict[str,str], optional): 请求头部。默认为{"Content-Type": "application-json"}。

        Returns:
            bytes: 服务器返回的内容。
        """
        conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  # 打开连接
        conn.connect((self.__api[0], self.__api[1]))  # 连接API网址
        conn.sendall(
            f"{action} {path} HTTP/1.1\r\nConnection: Close\r\nHost: {self.__api[0]}\r\n{'\r\n'.join(map(lambda key: (key + ': ' + headers[key]), headers.keys()))}\r\n\r\n".encode(
                encoding="utf-8"
            )
            + payload
        )  # 发送报文
        buffer, r = b"", b""  # 初始化buffer和临时值
        while r := conn.recv(1024):  # 获取回复报文
            buffer += r  # buffer追加临时值
        conn.close()  # 关闭连接
        return buffer  # 返回buffer

    def __init__(
        self, server_ip: tuple[str, int], api_ip: tuple[str, int], max_conn: int = 100
    ) -> None:
        """创建一个新的Bot对象。

        Args:
            server_ip (tuple[str, int]): 监听反向http的IP和端口。
            api_ip (tuple[str, int]): 发送API请求的IP和端口。
            max_conn (int, optional): 最大连接数。 默认为100。
        """
        self.__buffer, self.__api = b"", api_ip  # 初始化buffer并保存 API IP 供 get 使用。
        self.__server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  # 新建socket
        self.__server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.__server.bind((server_ip[0], server_ip[1]))  # 绑定到指定的IP和端口。
        self.__server.listen(max_conn)  # 设定最大连接数。

    def receive(self) -> bytes:
        """获得一个事件。
        注意：如果已经close，则返回空字节串。

        Returns:
            bytes: 事件内容。
        """
        (conn, _) = self.__server.accept()  # 接受反向连接
        r = b""
        while r := conn.recv(1024):  # 无限循环到成功获得事件
            self.__buffer += r  # 接收1024个byte
            tmp: bytes = self.__process_ev(conn)  # 尝试解析事件
            if tmp:  # 不是空字符串的情况
                return tmp  # 返回事件
        return b""

    def close(self) -> None:
        """停止服务器。"""
        self.__server.close()

    def __del__(self) -> None:
        self.close()
