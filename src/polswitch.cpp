#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/parport.h>
#include <linux/ppdev.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#include "polswitch.h"



PolSwitch::PolSwitch()
{
}

bool PolSwitch::open(const char *par_port)
{
    int result;

    fd = ::open(par_port, O_RDWR);
    if (fd < 0) {
		perror("PolSwitch: ");; // TODO: error
        return false;
    }

    result = ioctl(fd, PPCLAIM, NULL);
    if (result < 0)
    {
        perror("PPCLAIM");
        close(fd);
        return false;
    }
	// Set the Mode
    int mode = IEEE1284_MODE_BYTE;
    // int mode = IEEE1284_MODE_EPP;
	if (ioctl(fd, PPSETMODE, &mode))
	{
		perror("Could not set mode");
        ioctl(fd, PPRELEASE);
        close(fd);
        return false;
	}

	// Set data pins to output
	int dir = 0x00;
	if (ioctl(fd, PPDATADIR, &dir))
	{
		perror("Could not set parallel port direction");
		ioctl(fd, PPRELEASE);
		close(fd);
        return false;
	}

    return true;
}

void PolSwitch::setPolarity(int pol)
{
    int result;
    int x;

    if (pol == 0) {
        x = 0;
        result = ioctl(fd, PPWDATA, &x);
    } else if (pol > 0) {
        x = pol_left;
        result = ioctl(fd, PPWDATA, &x);
    } else if (pol < 0) {
        x = pol_right;
        result = ioctl(fd, PPWDATA, &x);
    }

    if (result < 0)
        perror("setPolarity");
}

PolSwitch::~PolSwitch()
{
	int result;
	result = ioctl(fd, PPRELEASE);
	if (result < 0)
		perror("~PolSwitch");
	close(fd);
}

