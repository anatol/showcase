Kiosk application for HTML pages
================================

Showcase is a full-screen application that shows a set of HTML pages. It can be used to
create kiosks, dashboards, information booths or similar setup.

The application can be started as a normal application from your Desktop Environment.
Or can be run in standalone mode by starting XWindow using `xinit` and bypassing window
manager completely. The project provides a systemd service file to run the application
in this stadalone mode.

The application implemented in C and  uses minimum set of dependencies.
Showcase utilizes GTK3 and WebKit html rendering engine.
It is suitable for low-power computers like RaspberryPi.

The code is licensed under GPL2.

### Gotchas:
 - If fonts in the stadalone mode look ugly install ttf fonts like `ttf-dejavu`.
