def file_write(file_name, max_duration = 10):
    import random
    import time
    with open(file_name , "a") as f:
        while True:
            n = random.randint(1, max_duration)
            f.write(f"asdf{n}")
            f.flush()
            time.sleep(n)


if __name__ == "__main__":
    file_write("/tmp/test.log", 10)
