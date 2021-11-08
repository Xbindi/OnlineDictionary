OBJS:=main.c recite.c
TARGET:=app
all:$(TARGET)
$(TARGET):$(OBJS)
	$(CC) $^ -o $@ -lsqlite3 -g
.PHONY:clean
clean:
	rm $(TARGET)