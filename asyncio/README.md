# Experimenting with Python's generators, coroutines, async, threads, gil, asyncio, wsgi, asgi etc

## Generators and coroutines
- https://peps.python.org/pep-0289/
- https://peps.python.org/pep-0342/
- David Beazely https://www.dabeaz.com
    - https://www.youtube.com/watch?v=MCs5OvhV9S4
    - https://www.youtube.com/watch?v=D1twn9kLmYg
    - https://www.dabeaz.com/finalgenerator/
    - https://web.archive.org/web/20260521144708/https://www.dabeaz.com/generators/
    - https://web.archive.org/web/20260521035600/https://dabeaz.com/coroutines/

# Asyncio
- https://peps.python.org/pep-0525/
- https://docs.python.org/3/library/asyncio.html
- https://docs.python.org/3/howto/a-conceptual-overview-of-asyncio.html
- https://docs.python.org/3/library/asyncio-task.html#coroutine
- https://peps.python.org/pep-0492/
- https://docs.python.org/3/library/asyncio-stream.html
- https://docs.python.org/3/library/asyncio-queue.html

4. **Asyncio streams**

   * `asyncio.start_server`
   * `StreamReader` / `StreamWriter`
   * compare abstraction vs raw FD readiness

5. **Starlette + Uvicorn**

   * put your application behind Starlette
   * inspect what Uvicorn actually does
   * ASGI interface: `scope`, `receive`, `send`

6. **Tracing experiment**

   * `strace` the blocking vs `select` vs asyncio/Uvicorn versions
   * observe `socket`, `accept`, `epoll`, `read`, `write`, etc.

Keep the HTTP protocol minimal: just enough to serve responses. The point is **I/O/event-loop architecture**, not HTTP.

This is a good standalone Python project and also directly connects to your Linux FD/event-loop interests.
