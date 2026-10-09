# import asyncio
# import types


# @types.coroutine
# async def gen():
#     for i in range(0):
#         yield i
#         await asyncio.sleep(0.5)


# async def test():
#     loop = asyncio.get_event_loop()
#     print(loop)
#     async for x in gen():
#         print(x)


# if __name__ == "__main__":
#     asyncio.run(test())
import socket

def fetch(url: str) -> None:
    sock: socket.SocketType = socket.socket()
    sock.connect(('xkcd.com', 80))
    request = 'GET {} HTTP/1.0\r\nHost: xkcd.com\r\n\r\n'.format(url)
    sock.send(request.encode('ascii'))
    response = b''
    chunk = sock.recv(4096)
    while chunk:
        response += chunk
        chunk = sock.recv(4096)

    # links = parse_links(response)
    # q.add(links)