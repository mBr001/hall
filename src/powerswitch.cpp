#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/parport.h>
#include <linux/ppdev.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

#include "powerswitch.h"



PowerSwitch::PowerSwitch()
{
}

bool PowerSwitch::open(const char *par_port)
{
    const int direction = 0x00;
    const int mode = IEEE1284_MODE_BYTE;
    int err;

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
    close(fd);
    errno = err;

    return false;
}

bool PowerSwitch::setPolarity(state_t state)
{
    if (ioctl(fd, PPWDATA, &state) < 0)
        return false;

    return true;
}

PowerSwitch::~PowerSwitch()
{
	int result;
	result = ioctl(fd, PPRELEASE);
	if (result < 0)
		perror("~PolSwitch");
	close(fd);
}

