import time


def coroutine(func):
    def start(*args, **kwargs):
        g = func(*args, **kwargs)
        next(g)
        return g

    return start


def generate_ints(N):
    for i in range(N):
        yield i


gen = generate_ints(5)
a, b, c = gen.__next__(), gen.__next__(), gen.__next__()
print(a, b, c)
for i in gen:
    print(i)

print("-------------------")


def countdown(n):
    print("starting from ", n)
    while n >= 0:
        newval = yield n
        if newval is not None:
            n = newval
        else:
            n -= 1


c = countdown(5)
for n in c:
    print(n)
    if n == 5:
        c.send(3)

print("-------------------")


@coroutine
def counter(maximum):
    print("started")
    i = 0
    while i < maximum:
        print("i is", i)
        val = yield (i, i + 1)
        # If value provided, change counter
        if val is not None:
            i = val
            print("value provided")
        else:
            i += 1
            print("value not provided")


it = counter(10)
print("-")
print(it.send(0))
# print(it.send(None)) #the first time either next(it) or send None
print("-")
print(next(it))
print("-")
print(it.send(8))
print("-")
# it.close()
print(next(it))
# print(next(it))

print("-------------------")


def producer(sentence, next_coroutine):
    """
    Producer which just split strings and
    feed it to pattern_filter coroutine
    """
    tokens = sentence.split(" ")
    for token in tokens:
        next_coroutine.send(token)
    next_coroutine.close()


def pattern_filter(pattern="ing", next_coroutine=None):
    """
    Search for pattern in received token
    and if pattern got matched, send it to
    print_token() coroutine for printing
    """
    print("Searching for {}".format(pattern))
    try:
        while True:
            token = yield
            if pattern in token:
                next_coroutine.send(token)
    except GeneratorExit:
        print("Done with filtering!!")
        next_coroutine.close()


def print_token():
    """
    Act as a sink, simply print the
    received tokens
    """
    print("I'm sink, i'll print tokens")
    try:
        while True:
            token = yield
            print(token)
    except GeneratorExit:
        print("Done with printing!")


pt = print_token()
pt.__next__()
pf = pattern_filter(next_coroutine=pt)
pf.__next__()

sentence = "Bob is running behind a fast moving car"
producer(sentence, pf)

print("-------------------")

print(sum([x * x for x in range(10)]))
print(sum(x * x for x in range(10)))  # the (...) becomes a generator expression

# print("-------------------")
#
# def fetch(host):
#     sock = socket.socket()
#     sock.setblocking(False)
#
#     sock.connect((host[0], host[1]))
#
#     sock.send(f"GET / HTTP/1.1\r\nHost: {host}\r\nConnection: close\r\n\r\n".encode())
#     len1 = 0
#     while True:
#         yield sock
#         data = sock.recv(4096)
#         if not data:
#             break
#
#         len1 += len(data)
#
#     return len1
#
#     sock.close()
#
#
# def fetch_bad(url):
#     return len(requests.get(url).content.decode("utf-8"))
#
#
# urls = [("localhost", 8000)] * 1
#
#
# tasks = [fetch(url) for url in urls]
#
#
# def wait():
#     waiting = {}
#
#     while tasks or waiting:
#         for task in tasks:
#             try:
#                 sock = next(task)
#                 waiting[sock] = task
#             except StopIteration as e:
#                 print("exception", e)
#                 pass
#
#         tasks.clear()
#
#         ready, _, _ = select.select(waiting, [], [])
#
#         for sock in ready:
#             tasks.append(waiting.pop(sock))
#
#
# start = time.time()
# len1 = wait()
# print("coroutine", time.time() - start, len1)
#
#
# len1 = 0
# start = time.time()
# for url in urls:
#     len1 += fetch_bad(url)
# print("sequential", time.time() - start, len1)

print("-------------------")


def read_file(target):
    with open("/tmp/test.log", "r") as f:
        f.seek(0, 2)
        while True:
            line = f.readline()
            if not line:
                time.sleep(0.1)
                continue
            try:
                target.send(line)
            except GeneratorExit:
                print("generator exit for read_file")
                target.close()
                break
            except StopIteration:
                print("splitter is no longer running")
                break


def split(target):
    # some random function
    while True:
        try:
            line = yield
            if line is None:
                break
            num = line[-1]
            if int(num) > 3:
                target.send(line)
            if int(num) > 5:
                print("got >5")
                target.close()
                break
        except GeneratorExit:
            # split exited
            print("generator exit for split")
            target.close()  # close target also then
            break


def printf():
    while True:
        try:
            line = yield
            if line is None:
                break
            print(line)
        except GeneratorExit:
            print("generator exit for printf")
            break


printer = printf()
printer.send(None)
splitter = split(printer)
splitter.send(None)  # priming
read_file(splitter)
