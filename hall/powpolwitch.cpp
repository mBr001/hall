#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/parport.h>
#include <linux/ppdev.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

#include "powpolswitch.h"


PwrPolSwitch::PwrPolSwitch() :
    fd(-1)
{
}

void PwrPolSwitch::close()
{
    if (fd >= 0) {
        ioctl(fd, PPRELEASE);
        ::close(fd);
        fd = -1;
    }
}

bool PwrPolSwitch::open(const char *par_port)
{
    const int direction = 0x00;
    const int mode = IEEE1284_MODE_BYTE;
    int err;

    close();

    fd = ::open(par_port, O_RDWR);
    if (fd < 0)
        return false;

    // Claim paralel port
    if (ioctl(fd, PPCLAIM, NULL) < 0)
        goto err1;

	// Set the Mode
    if (ioctl(fd, PPSETMODE, &mode) < 0)
        goto err;

	// Set data pins to output
    if (ioctl(fd, PPDATADIR, &direction) < 0)
        goto err;

    return true;

err:
    err = errno;
    ioctl(fd, PPRELEASE);
    errno = err;

err1:
    err = errno;
    ::close(fd);
    errno = err;

    return false;
}

PwrPolSwitch::state_t PwrPolSwitch::polarity()
{
    state_t state;

    if (ioctl(fd, PPRDATA, &state) < 0)
        return off;

    if (state & direct)
        return direct;

    if (state & cross)
        return cross;

    return off;
}

bool PwrPolSwitch::setPolarity(state_t state)
{
    const state_t state_off = off;

    if (polarity() != state && state != off) {
        if (ioctl(fd, PPWDATA, &state_off) < 0)
            return false;
        usleep(10000);
    }

    if (ioctl(fd, PPWDATA, &state) < 0)
        return false;

    return true;
}

PwrPolSwitch::~PwrPolSwitch()
{
    close();
}

