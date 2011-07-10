#ifndef POLSWITCH_H
#define POLSWITCH_H

class PolSwitch
{
public:
    PolSwitch();
    ~PolSwitch();
    bool open(const char *par_port);
    void setPolarity(int pol);
private:
	int fd;
	static const unsigned char pol_off = 0;
	static const unsigned char pol_left = 1;
	static const unsigned char pol_right = 2;
};

#endif // POLSWITCH_H
