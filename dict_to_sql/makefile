OBJS:=dict_to_sql.c

all:dict

dict:$(OBJS)
	@ # @ 表示取消回显 --删除 dict 和 dict.db
	@ rm -rf dict dict.db
	@ # 编译
	$(CC) $^ -o $@ -lsqlite3
	@ # 执行
	./dict
	@ # 删除dict二进制文件
	rm dict

.PHONE:clean
clean:
	@ rm -rf dict dict.db