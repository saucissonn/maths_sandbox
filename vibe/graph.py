import subprocess
import struct
import matplotlib.pyplot as plt
import os

src = "goldbach.c"
exe = "goldbach.exe"

if not os.path.exists(exe) or os.path.getmtime(src) > os.path.getmtime(exe):
    subprocess.run(["gcc", src, "-O2", "-o", exe], check=True)


n = int(input("Entrer un entier pair n >= 4: "))

p = subprocess.Popen(
    ["./goldbach.exe"],
    stdin=subprocess.PIPE,
    stdout=subprocess.PIPE
)

out, _ = p.communicate(f"{n}\n".encode("ascii"))
if p.returncode != 0:
    raise SystemExit(f"programme C a échoué: {p.returncode}")

# format: int32 count, puis count*(p,q,i) en int32
count = struct.unpack_from("<i", out, 0)[0]
offset = 4

I, P, Q = [], [], []
for _ in range(count):
    pval, qval, ival = struct.unpack_from("<iii", out, offset)
    offset += 12
    P.append(pval); Q.append(qval); I.append(ival)

plt.figure()
plt.plot(I, P, marker=".", linestyle="none", label="p")
plt.plot(I, Q, marker=".", linestyle="none", label="q")
plt.xlabel("i (pair)")
plt.ylabel("premiers")
plt.title(f"Goldbach (une décomposition par i), n={n}")
plt.grid(True)
plt.legend()
plt.show()
