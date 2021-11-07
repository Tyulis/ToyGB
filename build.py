import os


LDFLAGS = "-lm -pthread"
CFLAGS = "-Wall -Wextra -Wno-unused-parameter -g -I./include"

objects = []
for path, dirs, files in os.walk("src"):
	if path == "src":
		outdir = "build"
	else:
		outdir = path.replace("src" + os.path.sep, "build" + os.path.sep)

	if not os.path.exists(outdir):
		os.mkdir(outdir)

	for filename in files:
		srcpath = os.path.join(path, filename)
		buildpath = os.path.join(outdir, filename.replace(".cpp", ".o").replace(".c", ".o"))
		command = f"g++-10 {CFLAGS} -c -o {buildpath} {srcpath}"
		print(command)
		os.system(command)
		objects.append(buildpath)

command = f"g++-10 -o toygb {' '.join(objects)} {LDFLAGS}"
print(command)
os.system(command)
