import sys
sys.path.append("/home/tanz/venvs/gen/lib/python3.14/site-packages")
def fib(n):
    if n <= 1:
        return n
    else:
        return fib(n - 1) + fib(n - 2)


def get(url):
    import requests

    requests.get(url).content.decode("utf-8")
    return len(requests.get(url).content.decode("utf-8"))


def thread_v_process(func, *args, **kwargs):
    import threading
    import time

    for i in range(1, 10):
        start = time.time()
        threads = []
        for _ in range(i):
            t = threading.Thread(target=func, args=(args), kwargs=(kwargs))
            threads.append(t)

        for t in threads:
            t.start()

        for t in threads:
            t.join()

        print(f"Multi Threading for {i} calls took {time.time() - start}s")

    from multiprocessing import Pool

    for i in range(1, 10):
        start = time.time()
        with Pool(processes=i) as p:
            p.map(func, [*args]*i)
        print(f"Multi Processing for {i} calls took {time.time() - start}s")


if __name__ == "__main__":
    print("Running fib(40)")
    thread_v_process(fib, 40)
    print("Running get(https://www.google.com)")
    thread_v_process(get, "https://www.google.com")

# Running fib(40)
# Multi Threading for 1 calls took 10.109642744064331s
# Multi Threading for 2 calls took 19.548747301101685s
# Multi Threading for 3 calls took 30.562456369400024s
# Multi Threading for 4 calls took 41.47016930580139s
# Multi Threading for 5 calls took 52.27147603034973s
# Multi Threading for 6 calls took 62.77728605270386s
# Multi Threading for 7 calls took 74.25195217132568s
# Multi Threading for 8 calls took 85.3521409034729s
# Multi Threading for 9 calls took 96.44649338722229s
# Multi Processing for 1 calls took 11.515829086303711s
# Multi Processing for 2 calls took 13.422122240066528s
# Multi Processing for 3 calls took 14.946959018707275s
# Multi Processing for 4 calls took 17.330638885498047s
# Multi Processing for 5 calls took 25.03028964996338s
# Multi Processing for 6 calls took 27.864091873168945s
# Multi Processing for 7 calls took 33.212932109832764s
# Multi Processing for 8 calls took 38.86547374725342s
# Multi Processing for 9 calls took 43.75243782997131s
# Running get(https://www.google.com)
# Multi Threading for 1 calls took 0.5533890724182129s
# Multi Threading for 2 calls took 0.2865138053894043s
# Multi Threading for 3 calls took 0.3029768466949463s
# Multi Threading for 4 calls took 0.3153095245361328s
# Multi Threading for 5 calls took 0.3030281066894531s
# Multi Threading for 6 calls took 0.30960655212402344s
# Multi Threading for 7 calls took 0.3106343746185303s
# Multi Threading for 8 calls took 0.3161652088165283s
# Multi Threading for 9 calls took 0.4334685802459717s
# Multi Processing for 1 calls took 0.3822948932647705s
# Multi Processing for 2 calls took 0.4019596576690674s
# Multi Processing for 3 calls took 0.46541380882263184s
# Multi Processing for 4 calls took 0.47151803970336914s
# Multi Processing for 5 calls took 0.5081956386566162s
# Multi Processing for 6 calls took 0.5330173969268799s
# Multi Processing for 7 calls took 0.589714527130127s
# Multi Processing for 8 calls took 0.6223244667053223s
# Multi Processing for 9 calls took 0.7302751541137695s


# Trying with python3.14t -X gil=0 threading_processing.py with the free-threaded python
# So threading has same progressions as multi-processing nice
# Multi Threading for 1 calls took 13.86557126045227s
# Multi Threading for 2 calls took 14.914439916610718s
# Multi Threading for 3 calls took 17.727806568145752s
# Multi Threading for 4 calls took 19.84550380706787s
# Multi Threading for 5 calls took 27.832088470458984s
# Multi Threading for 6 calls took 30.323920488357544s
# Multi Threading for 7 calls took 35.54615521430969s
# Multi Threading for 8 calls took 46.04133176803589s
# Multi Threading for 9 calls took 45.8270046710968s
# Multi Processing for 1 calls took 12.909422636032104s
# Multi Processing for 2 calls took 14.083682537078857s
# Multi Processing for 3 calls took 16.743496894836426s
# Multi Processing for 4 calls took 19.439156532287598s
# Multi Processing for 5 calls took 26.55686354637146s
# Multi Processing for 6 calls took 30.29132056236267s
# Multi Processing for 7 calls took 34.65312099456787s
# Multi Processing for 8 calls took 40.60117554664612s
# Multi Processing for 9 calls took 44.70902132987976s
# Running get(https://www.google.com)
# Multi Threading for 1 calls took 0.5884692668914795s
# Multi Threading for 2 calls took 0.3055903911590576s
# Multi Threading for 3 calls took 0.28757667541503906s
# Multi Threading for 4 calls took 0.2942805290222168s
# Multi Threading for 5 calls took 0.6110198497772217s
# Multi Threading for 6 calls took 0.31148838996887207s
# Multi Threading for 7 calls took 0.31512951850891113s
# Multi Threading for 8 calls took 0.5737473964691162s
# Multi Threading for 9 calls took 0.35735583305358887s
# Multi Processing for 1 calls took 0.3944103717803955s
# Multi Processing for 2 calls took 0.43840599060058594s
# Multi Processing for 3 calls took 0.6952292919158936s
# Multi Processing for 4 calls took 0.5882256031036377s
# Multi Processing for 5 calls took 0.6759793758392334s
# Multi Processing for 6 calls took 0.7530226707458496s
# Multi Processing for 7 calls took 0.6722004413604736s
# Multi Processing for 8 calls took 0.7377874851226807s
# Multi Processing for 9 calls took 0.7796401977539062s
