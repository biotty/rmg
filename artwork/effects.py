import music


class Comb(music.Filter):

    def __init__(self, mix, delay):
        music.Filter.__init__(self, 0, [mix, delay])


class Echo(music.Filter):

    def __init__(self, mix, delay):
        music.Filter.__init__(self, 1, [mix, delay])
