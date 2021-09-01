TARGET_EXEC := slether

OBJS := slether.o hexdump.o

$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

.PHONY: clean

clean:
	-rm $(TARGET_EXEC) $(OBJS)
