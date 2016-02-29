import pi2golite, time
import sys
import tty
import logging
import pykka
import termios
import threading
import getopt
import socket
import struct
import json

DEFAULT_HOST = "10.5.5.1"
logger = logging.getLogger()

def readchar():
    fd = sys.stdin.fileno()
    old_settings = termios.tcgetattr(fd)
    try:
        tty.setraw(sys.stdin.fileno())
        ch = sys.stdin.read(1)
    finally:
        termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
    if ch == '0x03':
        raise KeyboardInterrupt
    return ch

def readkey(getchar_fn=None):
    getchar = getchar_fn or readchar
    c1 = getchar()
    if ord(c1) != 0x1b:
        return c1
    c2 = getchar()
    if ord(c2) != 0x5b:
        return c1
    c3 = getchar()
    return chr(0x10 + ord(c3) - 65)  # 16=Up, 17=Down, 18=Right, 19=Left arrows


class Controller(threading.Thread):
    def __init__(self, motor):
        super(Controller, self).__init__()
        self._motor = motor

    @staticmethod
    def _xytospeeddirection(x, y):
        def sign(num):
            if num < 0:
                return -1
            else:
                return 1
        # Use the squares of x and y to make it non-linear
        speed = y * y * sign(y) * 1000.0
        direction = x * x * sign(x) * 200.0
        return int(speed), int(direction)

    @staticmethod
    def _xytolr(x, y):
        def sign(num):
            if num < 0:
                return -1
            else:
                return 1
        # Use the squares of x and y to make it non-linear
        speed = y * y * sign(y) * 80.0
        direction = x * x * sign(x) * 20.0
        return int(speed + direction), int(speed - direction)

    @staticmethod
    def deadzone(val, cutoff):
        if abs(val) < cutoff:
            return 0
        elif val > 0:
            return (val - cutoff) / (1 - cutoff)
        else:
            return (val + cutoff) / (1 - cutoff)

class RemoteController(Controller):
    """Interface to a remote controller, using JSON over an IP connection"""

    def __init__(self, ipaddress, motor):
        super(RemoteController, self).__init__(motor)
        self.running = True
        self._ipaddress = ipaddress
        self.conn = None
        self.lastpos = [0, 0]
        self.lastdmh = False  # Dead Man's Handle

    def update(self):
        """
        Blocking function which receives a joystick update from the network.
        Joystick updates are framed JSON - the first byte gives the length
        of the incoming packet.
        The JSON encodes all the joystick inputs.  Note that these are
        different for different types of joystick. This routine must
        understand the differences and pick out the correct values.
        """
        try:
            header = self.conn.recv(4)
            if len(header) < 4:
                self.conn.close()
                self.conn = None
                return {}
            length = struct.unpack("<L", header)[0]
            payload = self.conn.recv(length)
        except socket.error as msg:
            logger.error("Unexpected socket error: %s", sys.exc_info()[0])
            logger.error(msg)
            return {}
        try:
            control_msg = json.loads(payload)
        except ValueError:
            logger.error("Error in json - skipping this update")
            return {}
        logger.info(control_msg)

        pos = self.lastpos
        dmh = self.lastdmh
        retval = {}

        if control_msg["controller"] == "keypad":
            # Extract keypad data
            pos = [float(control_msg["K_RIGHT"] - control_msg["K_LEFT"]),
                   float(control_msg["K_UP"] - control_msg["K_DOWN"])]
            dmh = control_msg["K_SPACE"]

        elif control_msg["controller"] == "Wireless Controller":
            # Extract Playstation Controller data
            pos = [self.deadzone(float(control_msg['sticks'][2]), 0.1),
                   self.deadzone(float(control_msg['sticks'][3]), 0.1) * -1]
            dmh = (int(control_msg['buttons'][6]) + int(
                control_msg['buttons'][7])) > 0

        elif control_msg["controller"] == "Controller (XBOX 360 For Windows)":
            # Extract XBOX 360 controller data
            pos = [self.deadzone(float(control_msg['sticks'][4]), 0.2),
                   self.deadzone(float(control_msg['sticks'][3]), 0.2) * -1]
            dmh = abs(float(control_msg['sticks'][2])) > 0.1

        if pos != self.lastpos:
            self.lastpos = pos
            retval["pos"] = pos

        if dmh != self.lastdmh:
            self.lastdmh = dmh
            retval["dmh"] = dmh

        return retval

    def run(self):
        logger.info("Remote Controller Thread Running")
        port = 10000
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(1.0)
            sock.bind((self._ipaddress, port))
        except:
            pass
        addr = None

        logger.info("Waiting for Controller Connection")
        while self.running:
            sock.listen(1)
            try:
                self.conn, addr = sock.accept()
                self.conn.setblocking(1)
            except socket.timeout:
                pass
            if self.conn is not None:
                logger.info("Controller Connected to {0}".format(addr))
            while self.running and self.conn is not None:
                controller_msg = self.update()
                if "pos" in controller_msg:
                    leftSpeed, rightSpeed = self._xytolr(controller_msg["pos"][0],controller_msg["pos"][1])
                    self._motor.go(leftSpeed, rightSpeed)

        logger.info("Remote Controller Thread Stopped")


class LocalController(Controller):
    """ Interface to a locally attached PS controller """

    def __init__(self, mode):
        super(LocalController, self).__init__(mode)
        self.running = True
        self.jsdev = None
        self.lastpos = [0, 0]
        self.lastdmh = False  # Dead Man's Handle

    def update(self):
        retval = {}
        # we want a copy of the values in self.lastpos, so we use list()
        pos = list(self.lastpos)
        dmh = self.lastdmh

        try:
            evbuf = self.jsdev.read(8)
        except IOError:
            self.jsdev = None
            return retval

        if evbuf:
            _time, _value, _type, _number = struct.unpack('IhBB', evbuf)

            if _type & 0x02:
                if _number == 0x02:  # x axis
                    pos[0] = self.deadzone(float(_value) / 32767.0, 0.1)
                elif _number == 0x05:  # y axis
                    pos[1] = self.deadzone(float(_value) / -32767.0, 0.1)
                elif _number == 0x03 or _number == 0x04:  # dmh
                    dmh = _value > 0
                elif _number == 0x07:
                    if _value > 0:
                        # down
                        retval["down"] = True
                    elif _value < 0:
                        # up
                        retval["up"] = True

        if pos != self.lastpos:
            self.lastpos = pos
            retval["pos"] = pos

        if dmh != self.lastdmh:
            self.lastdmh = dmh
            retval["dmh"] = dmh

        return retval

    def run(self):
        logger.info("Local Controller Thread Running")

        logger.info("Waiting for Controller Connection")
        while self.running:
            try:
                self.jsdev = open('/dev/input/js0', 'rb')
            except IOError:
                self.jsdev = None

            while self.running and self.jsdev is not None:
                controller_msg = self.update()
                if "pos" in controller_msg:
                    leftSpeed, rightSpeed = self._xytolr(controller_msg["pos"][0],controller_msg["pos"][1])
                    self._motor.go(leftSpeed, rightSpeed)

            time.sleep(1)  # try to connect again in a second's time

        logger.info("Local Controller Thread Stopped")


class Py2go(pykka.ThreadingActor):

    def __init__(self):
        super(Py2go, self).__init__()

    def go(self, leftSpeed, rightSpeed):
        pi2golite.go(leftSpeed, rightSpeed)

def main(argv):
    logging.basicConfig(level=logging.WARNING)

    try:
        opts, args = getopt.getopt(argv, "hd")
    except getopt.GetoptError:
        print 'metabot.py -h -d -i <ip address>'
        print '-h   help'
        print '-d   debug'
        print '-i   set local ip address'
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print 'keys : '
            print 'w, up-arrow      forward'
            print 'z, down arrow    reverse'
            print 's, right arrow   spin right'
            print 'd, left arrow    spin left'
            print '>, .             increase speed'
            print '<, ,             decrease speed'
            print 'space            stop'
            print 'i                IR light status'
            print 'f                toggle front LED'
            print 'r                toggle rear LED'
            print 'u                ultrasound distance'
            print 't                switch status'
            print '1,2,..9          move forward n steps'
            print 'End, ^C          exit'
            sys.exit(2)
        elif opt == '-i':
            host = arg
        elif opt == '-d':
            logger.setLevel(level=logging.DEBUG)

    host = DEFAULT_HOST
    frontLED = 0
    rearLED = 0

    pi2golite.init()
    py2go = Py2go.start().proxy()

    remote_controller = RemoteController(host, py2go)
    remote_controller.daemon = True
    remote_controller.start()

    local_controller = LocalController(py2go)
    local_controller.daemon = True
    local_controller.start()

    speed = 30

    # main loop
    try:
        while True:
            keyp = readkey()
            if keyp == 'w' or ord(keyp) == 16:
                py2go.go(speed, speed)
                print 'Forward', speed
            elif keyp == 'z' or ord(keyp) == 17:
                py2go.go(-speed, -speed)
                print 'Reverse', speed
            elif keyp == 's' or ord(keyp) == 18:
                py2go.go(speed, -speed)
                print 'Spin Right', speed
            elif keyp == 'a' or ord(keyp) == 19:
                py2go.go(-speed, speed)
                print 'Spin Left', speed
            elif keyp == '.' or keyp == '>':
                speed = min(100, speed+10)
                print 'Speed+', speed
            elif keyp == ',' or keyp == '<':
                speed = max (0, speed-10)
                print 'Speed-', speed
            elif keyp == ' ':
                py2go.go(0,0)
                print 'Stop'
            elif keyp == 'i':
                print 'Left:', pi2golite.irLeft()
                print 'Right:', pi2golite.irRight()
                print 'Line left', pi2golite.irLeftLine()
                print 'Line right', pi2golite.irRightLine()
                print
            elif keyp == 'f':
                frontLED = (frontLED + 1) % 2
                pi2golite.LsetLED (0, frontLED)
                print 'Toggle front LEDs'
            elif keyp == 'r':
                rearLED = (rearLED + 1) % 2
                pi2golite.LsetLED (1, rearLED)
                print 'Toggle rear LEDs'
            elif keyp == 'u':
                print pi2golite.getDistance()
            elif keyp == 't':
                print pi2golite.getSwitch()
            elif keyp >= '1' and keyp <= '9':
                print 'Step Forward:', keyp
                pi2golite.stepForward(20,int(keyp))
            elif ord(keyp) == 3:
                break

    except KeyboardInterrupt:
        pass

    remote_controller.running = False
    local_controller.running = False
    pykka.ActorRegistry.stop_all()
    pi2golite.cleanup()
    sys.exit()

if __name__ == "__main__":
    main(sys.argv[1:])

