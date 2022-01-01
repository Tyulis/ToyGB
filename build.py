import os, sys


CC = "g++"
LDFLAGS = "-lm -pthread -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio"
if "--release" in sys.argv:
	CFLAGS = "-Wall -Wextra -Wno-unused-parameter -std=c++20 -O3 -fcoroutines -I./include"
else:
	CFLAGS = "-Wall -Wextra -Wno-unused-parameter -std=c++20 -g -fcoroutines -I./include"
EXE = "toygb"

BOOTROMS = {
	"BOOT_DMG0": os.path.join("boot", "toyboot_dmg0"),
	"BOOT_DMG": os.path.join("boot", "toyboot_dmg"),
	"BOOT_MGB": os.path.join("boot", "toyboot_mgb"),

	# TODO
	"BOOT_CGB0": os.path.join("boot", "toyboot_dmg"),
	"BOOT_CGB": os.path.join("boot", "toyboot_dmg"),
	"BOOT_AGB": os.path.join("boot", "toyboot_dmg"),
	"BOOT_SGB": os.path.join("boot", "toyboot_dmg"),
	"BOOT_SGB2": os.path.join("boot", "toyboot_dmg"),
}

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
		command = f"{CC} {CFLAGS} -c -o {buildpath} {srcpath}"
		print(command)
		if (os.system(command) != 0): exit(1)
		objects.append(buildpath)

command = f"{CC} -o {EXE} {' '.join(objects)} {LDFLAGS}"
print(command)
if (os.system(command) != 0): exit(1)

if "--toyboot" in sys.argv or "--full" in sys.argv:
	for bootrom in BOOTROMS.values():
		sourcepath = bootrom + ".s"
		outpath = bootrom + ".bin"
		command = f"./{EXE} {outpath} --assemble={sourcepath}"
		print(command)
		if (os.system(command) != 0): exit(1)

if "--bundle" in sys.argv or "--full" in sys.argv:
	for argument in sys.argv:
		if argument.startswith("BOOT_") and "=" in arguments:
			key, value = argument.split("=")
			if key not in BOOTROMS.keys():
				print("Invalid bootROM key : " + key)
				print("Valid keys are " + ", ".join(BOOTROMS.keys()))
			else:
				if value.endswith((".s", ".asm")):
					binvalue = value.replace(".s", ".bin").replace(".asm", ".bin")
					comand = f"./{EXE} {binvalue} --assemble={value}"
					print(command)
					if (os.system(command) != 0): exit(1)
					value = binvalue
				elif not value.endswith(".bin"):
					print("Invalid bootROM file, name must end with .bin")

				BOOTROMS[key] = value.rpartition(".")[0]

	srcpath = os.path.join("src", "core", "bootroms.cpp")
	buildpath = os.path.join("build", "core", "bootroms-builtin.cpp");
	objpath = os.path.join("build", "core", "bootroms.o");
	with open(srcpath, "r") as f:
		cppboot = f.read()

	for key, filename in BOOTROMS.items():
		try:
			with open(filename + ".bin", "rb") as rom:
				content = rom.read()
		except FileNotFoundError:
			print(f"BootROM file {filename + '.bin'} is not found or could not be opened")
			continue

		cppcontent = ", ".join(map(str, content))

		cppboot = cppboot.replace(f"false;//$${key}_BUILTIN$$", "true;")
		cppboot = cppboot.replace(f"0;//$${key}_SIZE$$", f"{len(content)};")
		cppboot = cppboot.replace(f"0//$${key}$$", cppcontent);

	with open(buildpath, "w") as f:
		f.write(cppboot);

	command = f"{CC} {CFLAGS} -c -o {objpath} {buildpath}"
	print(command)
	if (os.system(command) != 0): exit(1)

	command = f"{CC} -o {EXE} {' '.join(objects)} {LDFLAGS}"
	print(command)
	if (os.system(command) != 0): exit(1)
