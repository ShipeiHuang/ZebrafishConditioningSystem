from datetime import timedelta


def time_to_seconds(text):

    h, m, s = map(int, text.split(":"))

    return h * 3600 + m * 60 + s


def seconds_to_hms(seconds):

    seconds = int(seconds)

    return str(timedelta(seconds=seconds))