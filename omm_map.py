def main():
    with open("build/us_pc/res/omm.map", "r") as f:
        lines = f.readlines()
    with open("build/us_pc/res/omm.map", "w") as f:
        for line in lines:
            line = line.replace(" ", "")
            i0 = line.find("(sec1)(fl0x00)(ty20)(scl2)(nx0)0x")
            i1 = line.find("(sec1)(fl0x00)(ty20)(scl3)(nx0)0x")
            if i0 != -1 or i1 != -1:
                i2 = max(i0, i1) + len("(sec1)(fl0x00)(ty20)(scl2)(nx0)0x")
                addr = line[i2:i2 + 16]
                name = line[i2 + 16:].strip()
                f.write(addr + " " + name + "\n")

if __name__ == "__main__":
    main()