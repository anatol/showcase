LIBS = gtk+-3.0 webkit2gtk-4.0 dbus-1 yaml-0.1
CFLAGS += `pkg-config --cflags $(LIBS)`
LDFLAGS += `pkg-config --libs $(LIBS)`

showcase: showcase.c config.c
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)

install: showcase
	install -Dm755 showcase $(DESTDIR)/usr/bin/showcase
	install -Dm644 showcase.service $(DESTDIR)/usr/lib/systemd/system/showcase.service
	install -Dm644 polkit.rules $(DESTDIR)/usr/share/polkit-1/rules.d/showcase.rules
	install -Dm644 showcase.sample.yaml $(DESTDIR)/etc/showcase.yaml
	install -Dm644 COPYING $(DESTDIR)/usr/share/licenses/showcase/COPYING

clean:
	$(RM) showcase
