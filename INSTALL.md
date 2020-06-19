# Prerequisites

This project is an GNU compliant project and use the GNU Autotools
toolchain to configure, compile and install. Make sure to have the
following requirements installed:

 * GCC g++ >= 6.3
 * GNU autoconf >=2.69
 * GNU autoconf-archive >=20180313
 * GNU automake >= 1.16.1
 * GNU make >= 4.2.1
 * CUPS libcups2 >= 2.3
 * CUPS libcupsimage2 >= 2.3
 
## On Debian like

You can install all you need on Debian GNU/Linux like distribution.
```
apt update && apt install build-essential autoconf autoconf-archive automake libcups2-dev libcupsimage2-dev
```

# Configuration

```
cd epson-tm-t88v-driver
autoreconf -fiv
./configure --prefix=/usr
```

# Compilation & Installation

```
make && make install
```

# Add your printer in CUPS

Open administration CUPS web page and add your printer with the
printer name listed in EPSON vendor printer list. The default admin
page is http://localhost:631/admin

# Feedback

In any case of fail/success, please send a message with your
configuration.

In specific case of failing, please write an issue.
