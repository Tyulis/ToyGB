


with open("test.log", encoding="utf-16-le") as f:
	c = f.read()

while True:
	try:
		command = input("> ")
		if command.startswith("find "):
			string = command.partition(" ")[2]
			start = 0
			while True:
				index = c.find(string, start)
				if index < 0: break
				print("Found at position", index)
				start = index + 1
		elif command.startswith("show "):
			position = int(command.partition(" ")[-1])
			print(c[position - 10000 : position + 10000])
		elif command.startswith("quit "):
			break
		else:
			vars = {"c": c}
			exec(command, vars, vars)
			if "_" in vars:
				print("Output :", vars["_"])
	except KeyboardInterrupt:
		continue
